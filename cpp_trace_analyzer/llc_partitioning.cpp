#include <cassert>
#include <cmath>
#include <bitset>
#include <utility>
#include "llc_partitioning.hpp"

WayPartitioning::WayPartitioning(const std::vector<uint32_t> &n_ways, uint64_t slice_size, uint32_t block_size) {
    for (const auto& n_way: n_ways) {
        way_partitioned_caches_.emplace_back(slice_size, n_way, block_size);
    }
}

void WayPartitioning::read(uint32_t client_id, uintptr_t addr) {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    way_partitioned_caches_[client_id].read(addr);
}

void WayPartitioning::write(uint32_t client_id, uintptr_t addr) {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    return way_partitioned_caches_[client_id].write(addr);
}

uint32_t WayPartitioning::misses(uint32_t client_id) const {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    return way_partitioned_caches_[client_id].misses();
}

uint32_t WayPartitioning::hits(uint32_t client_id) const {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    return way_partitioned_caches_[client_id].hits();
}

InterNodePartitioning::InterNodePartitioning(uint32_t clients, const std::vector<uint32_t>& n_slices,
                                             uint64_t slice_size, uint32_t assoc, uint32_t block_size) {
    if (n_slices.size() != clients) {
        throw std::invalid_argument("Invalid clients or slice array given!");
    }
    memory_nodes_.resize(clients);
    for (size_t i = 0; i < n_slices.size(); i++) {
        std::vector<Cache> slices(n_slices[i], Cache(slice_size, assoc, block_size));
        memory_nodes_[i] = slices;
    }
}


void InterNodePartitioning::read(uint32_t client_id, uintptr_t addr) {
    access(client_id, addr, false);
}

void InterNodePartitioning::write(uint32_t client_id, uintptr_t addr) {
    access(client_id, addr, true);
}

uint32_t InterNodePartitioning::misses(uint32_t client_id) {
    // Sum of all misses of all memory nodes.
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    auto& memory_node = memory_nodes_[client_id];

    uint32_t misses = 0;
    for (const auto& slice: memory_node) {
        misses += slice.misses();
    }

    return misses;
}

uint32_t InterNodePartitioning::hits(uint32_t client_id) {
    // Sum of all hits of all memory nodes.
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    auto& memory_node = memory_nodes_[client_id];

    uint32_t hits = 0;
    for (const auto& slice: memory_node) {
        hits += slice.hits();
    }

    return hits;
}

const std::vector<Cache> &InterNodePartitioning::memory_nodes(uint32_t client_id) {
    // Sum of all hits of all memory nodes.
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }

    return memory_nodes_[client_id];
}

static uint32_t bits_to_represent(uint32_t n) {
    return n > 0 ? 1 + bits_to_represent(n/2) : 0;
}

void InterNodePartitioning::access(uint32_t client_id, uintptr_t addr, bool write) {
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    // Find the LLC slices that belong to that ID.
    auto& memory_node = memory_nodes_[client_id];

    // TODO(kostas). FIXME.
    auto page_offset_bits = (uint32_t) std::log2(memory_node[0].block_size());
    uint32_t node_selection_bits = bits_to_represent(memory_nodes_.size());

    auto node_selection = static_cast<uint32_t>((addr >> page_offset_bits) & Cache::mask(node_selection_bits));
    auto& slice = memory_node[node_selection % memory_node.size()];

    if (write) {
        slice.write(addr);
    } else {
        slice.read(addr);
    }
}

IntraNodePartitioning::IntraNodePartitioning(uint32_t clients, uint64_t cache_size, uint32_t assoc,
                                             uint32_t block_size, std::vector<fixed_bits_t> aux_table)
                                             : cache_(cache_size, assoc, block_size),
                                               aux_table_(std::move(aux_table)), stats_(clients, {0, 0}) {
    if (aux_table_.size() != clients) {
        throw std::invalid_argument("Invalid clients or auxiliary table given!");
    }
}

void IntraNodePartitioning::read(uint32_t client_id, uintptr_t addr) {
    access(client_id, addr, false);
}

void IntraNodePartitioning::write(uint32_t client_id, uintptr_t addr) {
    access(client_id, addr, true);
}

void IntraNodePartitioning::access(uint32_t client_id, uintptr_t addr, bool write) {
    if (client_id >= aux_table_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    auto& bits_info = aux_table_[client_id];

    // Create a bitset of the addr.
    std::bitset<64> baddr(addr);

    auto block_offset_bits = (uint32_t) std::log2(cache_.block_size());
    auto set_bits = (size_t) std::log2(cache_.sets());

    // Replace the most significant set_index with the `fixed_bits`
    auto bit_start = block_offset_bits + set_bits - bits_info.n_bits;
    auto bit_end = block_offset_bits + set_bits;
    auto bits_info_idx = 0;
    for (auto i = bit_start; i < bit_end; i++) {
        baddr[i] = bits_info.bits[bits_info_idx++];
    }

    auto tag_bits = 64 - block_offset_bits - (set_bits - bits_info.n_bits);
    LocationInfo loc {
        .set_index = static_cast<uint32_t>((baddr.to_ulong() >> block_offset_bits) & Cache::mask(set_bits)),
        .tag = static_cast<uint32_t>(addr >> (block_offset_bits - (set_bits - bits_info.n_bits))) & Cache::mask(tag_bits)
    };

    auto& stats = stats_[client_id];
    uint32_t misses = cache_.misses();
    cache_.access(loc, write);
    if (cache_.misses() > misses) {
        stats.first++;
    } else {
        stats.second++;
    }
}

uint32_t IntraNodePartitioning::misses(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    return stats_[client_id].first;
}

uint32_t IntraNodePartitioning::hits(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    return stats_[client_id].second;
}
