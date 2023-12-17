#include <cassert>
#include <cmath>
#include <bitset>
#include <utility>
#include <iostream>
#include "llc_partitioning.hpp"

WayPartitioning::WayPartitioning(uint64_t cache_size, uint32_t block_size,
                                 const std::vector<uint32_t>& n_ways) {

    uint32_t s_ways = 0;
    for (const auto& way: n_ways) {
        s_ways += way;
    }

    if (cache_size % (block_size * s_ways) != 0) {
        throw std::invalid_argument("Block size * associativity should be a multiple of cache size!");
    }
    uint32_t sets = cache_size / ((uint64_t) block_size * s_ways);

    way_partitioned_caches_.resize(n_ways.size());
    for (size_t i = 0; i < n_ways.size(); i++) {
        way_partitioned_caches_[i] = Cache((cache_size * n_ways[i]) / s_ways, sets, n_ways[i], block_size);
    }
}

bool WayPartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    auto& cache = way_partitioned_caches_[client_id];
    auto loc = Cache::compute_location_info(addr, cache.block_size(), cache.sets(), cache.tag_bits());
    return cache.access(loc, addr);
}

uint32_t WayPartitioning::misses(uint32_t client_id) const {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return way_partitioned_caches_[client_id].misses();
}

uint32_t WayPartitioning::hits(uint32_t client_id) const {
    if (client_id >= way_partitioned_caches_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return way_partitioned_caches_[client_id].hits();
}
Cache &WayPartitioning::get_cache(uint32_t client_id) {
    return way_partitioned_caches_.at(client_id);
}

const Cache &WayPartitioning::get_cache(uint32_t client_id) const {
    return way_partitioned_caches_.at(client_id);
}

InterNodePartitioning::InterNodePartitioning(uint64_t slice_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t>& n_slices) {
    num_clusters = 0;
    memory_nodes_.resize(n_slices.size());
    for (size_t i = 0; i < n_slices.size(); i++) {
        std::vector<Cache> slices(n_slices[i], Cache(slice_size, assoc, block_size));
        memory_nodes_[i] = slices;
        num_clusters += n_slices[i];
    }
}

// Generates a bitmask with n bits set to 1 at the right. Ej, 00001, 00011, 00111
static uint32_t bit_mask_n_bits_right(uint32_t n) {
    return (1 << n) - 1;
}

bool InterNodePartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!!");
    }
    // Find the LLC slices that belong to that ID.
    auto& memory_node = memory_nodes_[client_id];

    // // Using block bits as page offset bits.
    auto block_offset_bits = (uint32_t) std::log2(memory_node[0].block_size());
    auto set_offset_bits = (uint32_t) std::log2(memory_node[0].sets());

    auto node_selection = static_cast<uint32_t>(addr >> (set_offset_bits + block_offset_bits)) & bit_mask_n_bits_right(std::log2(num_clusters));
    auto& slice = memory_node[node_selection % memory_node.size()];

    return slice.access(Cache::compute_location_info(addr, slice.block_size(), slice.sets(), slice.tag_bits()), addr);
}


uint32_t InterNodePartitioning::misses(uint32_t client_id) const {
    // Sum of all misses of all memory nodes.
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!!");
    }
    auto& memory_node = memory_nodes_[client_id];

    uint32_t misses = 0;
    for (const auto& slice: memory_node) {
        misses += slice.misses();
    }

    return misses;
}

uint32_t InterNodePartitioning::hits(uint32_t client_id) const {
    // Sum of all hits of all memory nodes.
    if (client_id >= memory_nodes_.size()) {
        throw std::invalid_argument("Invalid client_id given!!");
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
        throw std::invalid_argument("Invalid client_id given!!");
    }

    return memory_nodes_[client_id];
}

IntraNodePartitioning::IntraNodePartitioning(uint64_t cache_size, uint32_t assoc,
                                             uint32_t block_size, std::vector<fixed_bits_t> aux_table)
                                             : cache_(cache_size, assoc, block_size),
                                               aux_table_(std::move(aux_table)), stats_(aux_table_.size(), {0, 0}) {}

bool IntraNodePartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= aux_table_.size()) {
        throw std::invalid_argument("Invalid client_id given!!");
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

    auto tag_bits = ADDRESS_SIZE - block_offset_bits;
    LocationInfo loc {
        .set_index = static_cast<uint32_t>((baddr.to_ulong() >> block_offset_bits) & Cache::mask(set_bits)),
        .tag = static_cast<uint64_t>(addr >> block_offset_bits) & Cache::mask(tag_bits)
    };

    auto& stats = stats_[client_id];
    uint32_t misses = cache_.misses();
    bool hit = cache_.access(loc, addr);
    if (!hit) {
        stats.first++;
    } else {
        stats.second++;
    }
    return hit;
}

uint32_t IntraNodePartitioning::misses(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return stats_[client_id].first;
}

uint32_t IntraNodePartitioning::hits(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return stats_[client_id].second;
}

Cache &IntraNodePartitioning::cache() {
    return cache_;
}

ClusterWayPartitioning::ClusterWayPartitioning(uint32_t n_clusters, uint64_t slice_size, uint32_t block_size,
                                               const std::vector<uint32_t> &n_ways) {
    // n_clusters should be power of 2
    if (!Cache::is_power_of_2(n_clusters)) {
        throw std::invalid_argument("n_clusters should be power of 2!");
    }

    clusters_.resize(n_clusters, WayPartitioning(slice_size, block_size, n_ways));
    stats_.resize(n_ways.size(), {0, 0});
    block_size_ = block_size;
}

bool ClusterWayPartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }

    // Get slice id.
    auto slice_id_bits = (uint32_t) log2(clusters_.size());
    auto block_offset_bits = (uint32_t) std::log2(block_size_);
    auto cluster = (uint32_t) (addr >> block_offset_bits) & bit_mask_n_bits_right(slice_id_bits);
    assert(cluster < clusters_.size());

    // Remove slice id bits to avoid conflicts.
    auto block_offset_mask = bit_mask_n_bits_right(block_offset_bits);

    auto new_addr = ((addr >> slice_id_bits) & ~block_offset_mask) | (addr & block_offset_mask);

    bool hit = clusters_[cluster].access(client_id, new_addr);
    if (!hit) {
        stats_[client_id].first++;
    } else {
        stats_[client_id].second++;
    }
    return hit;
}

uint32_t ClusterWayPartitioning::misses(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return stats_[client_id].first;
}

uint32_t ClusterWayPartitioning::hits(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return stats_[client_id].second;
}

std::vector<WayPartitioning> &ClusterWayPartitioning::clusters() {
    return clusters_;
}

InterIntraNodePartitioning::InterIntraNodePartitioning(uint32_t assoc, uint32_t block_size,
                                                       const std::vector<std::vector<uint32_t>> &n_cache_sizes,
                                                       const std::vector<inter_intra_aux_table_t>& aux_tables_per_client) {
    // Check that all clusters contain cache sizes for each client.
    uint32_t n_clients = aux_tables_per_client.size();
    for (const auto& cache_sizes_per_cluster: n_cache_sizes) {
        if (cache_sizes_per_cluster.size() != n_clients) {
            throw std::invalid_argument("Invalid number of clients in cache sizes vector!");
        }
    }

    uint32_t n_clusters = n_cache_sizes.size();
    inp_.resize(n_clusters);
    uint32_t max_num_sets = 0;

    for (uint32_t cluster = 0; cluster < n_clusters; cluster++) {
        uint32_t clients = n_cache_sizes[cluster].size();
        inp_[cluster].resize(clients);
        for (uint32_t client = 0; client < clients; client++) {
            if (n_cache_sizes[cluster][client] > 0) {
                Cache cache(n_cache_sizes[cluster][client], assoc, block_size);
                inp_[cluster][client] = cache;
                if (cache.sets() > max_num_sets) {
                    max_num_sets = cache.sets();
                }
            }
        }
    }

    set_bits_ = (uint32_t) std::log2(max_num_sets);

    aux_tables_per_client_ = aux_tables_per_client;
    block_size_ = block_size;
    stats_.resize(inp_[0].size());
}

bool InterIntraNodePartitioning::access(uint32_t client_id, uintptr_t addr) {
    if (client_id >= aux_tables_per_client_.size() || client_id >= inp_[0].size()) {
        throw std::invalid_argument("Invalid client_id given!!");
    }

    // Get the node selection bits (after set_index).
    auto block_offset_bits = (uint32_t) std::log2(block_size_);
    auto node_selection_bits = ADDRESS_SIZE - (block_offset_bits + set_bits_);

    auto node_selection = (addr >> (block_offset_bits + set_bits_)) & Cache::mask(node_selection_bits);

    uint32_t cluster_id = 0;
    auto& aux_table = aux_tables_per_client_[client_id];
    node_selection %= aux_table.total_num_cores;
    for (const auto& entry: aux_table.entries) {
        if (entry.cumulative_core_sum > node_selection ) {
            cluster_id = entry.cluster_id;
            break;
        }
    }
    assert(cluster_id < inp_.size());
    assert(client_id < inp_[cluster_id].size());

    auto& cache = inp_[cluster_id][client_id];
    if (cache.cache_size() > 0) {
        bool hit = cache.access(addr);
        if (!hit) {
            stats_[client_id].first++;
        } else {
            stats_[client_id].second++;
        }
        return hit;
    }
    return false;
}

uint32_t InterIntraNodePartitioning::misses(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return stats_[client_id].first;
}

uint32_t InterIntraNodePartitioning::hits(uint32_t client_id) const {
    if (client_id >= stats_.size()) {
        throw std::invalid_argument("Invalid client_id given!");
    }
    return stats_[client_id].second;
}

Cache& InterIntraNodePartitioning::get_cache_slice(uint32_t client_id, uint32_t cluster_id) {
    if (cluster_id >= inp_.size() || client_id >= inp_[cluster_id].size()) {
        throw std::invalid_argument("Invalid cluster or client id!");
    }

    return inp_[cluster_id][client_id];
}