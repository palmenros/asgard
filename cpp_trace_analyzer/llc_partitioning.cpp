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

void WayPartitioning::read(uint32_t client_id, uintptr_t addr) noexcept {
    way_partitioned_caches_[client_id].read(addr);
}

void WayPartitioning::write(uint32_t client_id, uintptr_t addr) noexcept {
    return way_partitioned_caches_[client_id].write(addr);
}

uint32_t WayPartitioning::misses(uint32_t client_id) const noexcept {
    return way_partitioned_caches_[client_id].misses();
}

uint32_t WayPartitioning::hits(uint32_t client_id) const noexcept {
    return way_partitioned_caches_[client_id].hits();
}

InterNodePartitioning::InterNodePartitioning(uint32_t clients, const std::vector<uint32_t>& n_slices,
                                             uint64_t slice_size, uint32_t assoc, uint32_t block_size) {
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
    assert(client_id < memory_nodes_.size());
    auto& memory_node = memory_nodes_[client_id];

    uint32_t misses = 0;
    for (const auto& slice: memory_node) {
        misses += slice.misses();
    }

    return misses;
}

uint32_t InterNodePartitioning::hits(uint32_t client_id) {
    // Sum of all hits of all memory nodes.
    assert(client_id < memory_nodes_.size());
    auto& memory_node = memory_nodes_[client_id];

    uint32_t hits = 0;
    for (const auto& slice: memory_node) {
        hits += slice.hits();
    }

    return hits;
}

const std::vector<Cache> &InterNodePartitioning::memory_nodes(uint32_t client_id) {
    // Sum of all hits of all memory nodes.
    assert(client_id < memory_nodes_.size());

    return memory_nodes_[client_id];
}

static uint32_t bits_to_represent(uint32_t n)
{
    return n > 0 ? 1 + bits_to_represent(n/2) : 0;
}

void InterNodePartitioning::access(uint32_t client_id, uintptr_t addr, bool write) {
    // Find the LLC slices that belong to that ID.
    assert(client_id < memory_nodes_.size());
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
                                               aux_table_(std::move(aux_table)),
                                               stats(clients, {0, 0}) {}

void IntraNodePartitioning::read(uint32_t client_id, uintptr_t addr) noexcept {
    access(client_id, addr, false);
}

void IntraNodePartitioning::write(uint32_t client_id, uintptr_t addr) noexcept {
    access(client_id, addr, true);
}

void IntraNodePartitioning::access(uint32_t client_id, uintptr_t addr, bool write) noexcept {
    assert(client_id < aux_table_.size());
    auto& bits_info = aux_table_[client_id];

    // Create a bitset of the addr.
    std::bitset<64> baddr(addr);

    // Replace the most significant set_index with the `fixed_bits`
    auto set_bits = (size_t) std::log2(cache_.sets());
    for (size_t i = set_bits; i < set_bits + bits_info.n_bits; i++) {
        baddr[i] = bits_info.bits[i];
    }

    auto block_bits = (uint32_t) std::log2(cache_.block_size());

    LocationInfo loc {
        .set_index = static_cast<uint32_t>((baddr.to_ulong() >> block_bits) & Cache::mask(set_bits)),
        .tag = static_cast<uint32_t>((baddr.to_ulong() >> block_bits) & Cache::mask(cache_.tag_bits() + set_bits))
    };

    cache_.access(loc, write);
}

uint32_t IntraNodePartitioning::misses(uint32_t client_id) const noexcept {
    return stats[client_id].first;
}

uint32_t IntraNodePartitioning::hits(uint32_t client_id) const noexcept {
    return stats[client_id].second;
}
