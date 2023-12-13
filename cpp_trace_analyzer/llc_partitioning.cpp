#include <cassert>
#include <cmath>
#include <bitset>
#include <utility>
#include <iostream>
#include "llc_partitioning.hpp"

WayPartitioning::WayPartitioning(uint64_t cache_size, uint32_t block_size, const std::vector<uint32_t>& n_ways) {
    uint32_t s_ways = 0;
    for (const auto& n_way: n_ways) {
        s_ways += n_way;
    }

    if (cache_size % (block_size * s_ways) != 0) {
        throw std::invalid_argument("Block size * associativity should be a multiple of cache size!");
    }
    uint32_t sets = cache_size / ((uint64_t) block_size * s_ways);

    way_partitioned_caches_.resize(n_ways.size());
    for (size_t i = 0; i < n_ways.size(); i++) {
        way_partitioned_caches_[i] = Cache(cache_size, sets, n_ways[i], block_size);
    }
}

void WayPartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }
    auto& cache = way_partitioned_caches_[client_id];
    auto loc = Cache::compute_location_info(addr, cache.block_size(), cache.sets(), cache.tag_bits());
    cache.access(loc, addr);
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

InterNodePartitioning::InterNodePartitioning(uint64_t slice_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t>& n_slices) {
    memory_nodes_.resize(n_slices.size());
    for (size_t i = 0; i < n_slices.size(); i++) {
        std::vector<Cache> slices(n_slices[i], Cache(slice_size, assoc, block_size));
        memory_nodes_[i] = slices;
    }
}


static uint32_t bits_to_represent(uint32_t n) {
    return n > 0 ? 1 + bits_to_represent(n/2) : 0;
}

void InterNodePartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    // Find the LLC slices that belong to that ID.
    auto& memory_node = memory_nodes_[client_id];

    // // Using block bits as page offset bits.
    auto page_offset_bits = (uint32_t) std::log2(memory_node[0].block_size());
    uint32_t node_selection_bits = bits_to_represent(memory_nodes_.size());

    auto node_selection = static_cast<uint32_t>((addr >> page_offset_bits) & Cache::mask(node_selection_bits));
    auto& slice = memory_node[node_selection % memory_node.size()];

    slice.access(Cache::compute_location_info(addr, slice.block_size(), slice.sets(), slice.tag_bits()), addr);

//    if (write) {
//        slice.write(addr);
//    } else {
//        slice.read(addr);
//    }
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

IntraNodePartitioning::IntraNodePartitioning(uint64_t cache_size, uint32_t assoc,
                                             uint32_t block_size, std::vector<fixed_bits_t> aux_table)
                                             : cache_(cache_size, assoc, block_size),
                                               aux_table_(std::move(aux_table)), stats_(aux_table_.size(), {0, 0}) {}

void IntraNodePartitioning::access(uint32_t client_id, uintptr_t addr) {
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

    auto tag_bits = 64 - block_offset_bits;
    LocationInfo loc {
        .set_index = static_cast<uint32_t>((baddr.to_ulong() >> block_offset_bits) & Cache::mask(set_bits)),
        .tag = static_cast<uint64_t>(addr >> block_offset_bits) & Cache::mask(tag_bits)
    };

    auto& stats = stats_[client_id];
    uint32_t misses = cache_.misses();
    cache_.access(loc, addr);
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

Cache &IntraNodePartitioning::cache() {
    return cache_;
}

// TODO(kostas): Discuss with Luis how to initialize this.
ClusterPartitioning::ClusterPartitioning() {

}

void ClusterPartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= clusters_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }

    // Using block bits as page offset bits.
    auto page_offset_bits = (uint32_t) std::log2(clusters_[0].inp_cache[0].cache().block_size());
    // Node selection bits to represent number of clusters.
    uint32_t node_selection_bits = bits_to_represent(clusters_.size());

    // Node selection % cores.
    auto node_selection = static_cast<uint32_t>((addr >> page_offset_bits) & Cache::mask(node_selection_bits)) % n_cores;
    uint32_t cluster = 0;
    for (size_t i = 0; i < clusters_.size(); i++) {
        if (clusters_[i].c_sum >= node_selection) {
            cluster = i;
            break;
        }
    }

    auto& inp_cache = clusters_[cluster].inp_cache;

    inp_cache.access(client_id, addr);
}

uint32_t ClusterPartitioning::misses(uint32_t client_id) const {
    if (client_id >= clusters_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }

    return clusters_[client_id].inp.misses(client_id);
}

uint32_t ClusterPartitioning::hits(uint32_t client_id) const {
    if (client_id >= clusters_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }

    return clusters_[client_id].inp.hits(client_id);
}
