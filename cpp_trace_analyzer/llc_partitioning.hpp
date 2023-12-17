#pragma once

#include <cstdint>
#include <vector>
#include <bitset>

#include "cache.hpp"

class WayPartitioning {
public:
    WayPartitioning(uint64_t cache_size, uint32_t block_size, const std::vector<uint32_t> &n_ways);

    // Returns if its a hit or not.
    bool access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    Cache& get_cache(uint32_t client_id);
    const Cache& get_cache(uint32_t client_id) const;
private:
    std::vector<Cache> way_partitioned_caches_;
};

class InterNodePartitioning {
public:
    // clients is the number of clients in the system.
    // n_slices is how many slices each client has.
    // the rest are information for LLC slice.
    InterNodePartitioning(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t> &n_slices);

    bool access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    const std::vector<Cache> &memory_nodes(uint32_t client_id);
private:
    // Memory node list per client.
    // memory_nodes[i][j] = slice j of client i
    std::vector<std::vector<Cache>> memory_nodes_;
    uint32_t num_clusters;
};

struct fixed_bits_t {
    std::bitset<32> bits;
    uint32_t n_bits;
};

class IntraNodePartitioning {
public:
    IntraNodePartitioning(uint64_t cache_size, uint32_t assoc, uint32_t block_size, std::vector<fixed_bits_t> aux_table);

    bool access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    Cache &cache();
private:
    Cache cache_;
    std::vector<fixed_bits_t> aux_table_;
    // Hits/Misses per client.
    std::vector<std::pair<uint32_t, uint32_t>> stats_;
};

class ClusterWayPartitioning {
public:
    ClusterWayPartitioning(uint32_t n_clusters, uint64_t cache_size, uint32_t block_size,
                           const std::vector<uint32_t> &n_ways);

    bool access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    std::vector<WayPartitioning> &clusters();
    uint32_t n_clusters() const;
private:
    uint32_t block_size_;
    using cluster_t_intra_node_t = WayPartitioning;
    std::vector<cluster_t_intra_node_t> clusters_;
    // Hits/Misses per client.
    std::vector<std::pair<uint32_t, uint32_t>> stats_;
};

struct inter_intra_aux_table_entry_t {
    uint32_t cluster_id;
    uint32_t cumulative_core_sum;
};

struct inter_intra_aux_table_t {
    uint32_t total_num_cores;
    std::vector<inter_intra_aux_table_entry_t> entries;
};

class InterIntraNodePartitioning {
public:
    InterIntraNodePartitioning(uint32_t assoc, uint32_t block_size,
                               // n_cache_sizes[clusterId][client] -> Size of the private cache of that client
                               const std::vector<std::vector<uint32_t>> &n_cache_sizes,
                               // aux_tables_per_client[client] -> Auxiliary table for client
                               const std::vector<inter_intra_aux_table_t>& aux_tables_per_client);

    bool access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    Cache& get_cache_slice(uint32_t client_id, uint32_t cluster_id);
    uint32_t n_clusters() const;
private:
    std::vector<inter_intra_aux_table_t> aux_tables_per_client_;
    // inp[cluster][client] -> Cache of that client has in cluster.
    std::vector<std::vector<Cache>> inp_;
    uint32_t block_size_;
    // Set bits is the maximum number of set bits for each cache.
    uint32_t set_bits_;
    // Hits/Misses per client.
    std::vector<std::pair<uint32_t, uint32_t>> stats_;
};

template <class L2Cache>
class MultiLevelCache {
public:
    // private_cache is a per-client cache. It will be copied for each core
    MultiLevelCache(uint32_t num_cores, const Cache& private_cache, L2Cache shared_cache);

    // Returns true if hits in either L1 or L2
    bool access(uint32_t core_id, uint32_t client_id, uintptr_t addr);

    Cache& get_private_cache(uint32_t core_id);

    L2Cache& get_shared_cache();

    // returns the number of misses in the L2 cache
    [[nodiscard]] uint32_t misses(uint32_t client_id) const;

private:
    std::vector<Cache> private_caches_;
    L2Cache shared_cache_;
};

template<class L2Cache>
uint32_t MultiLevelCache<L2Cache>::misses(uint32_t client_id) const {
    return shared_cache_.misses(client_id);
}

template<class L2Cache>
bool MultiLevelCache<L2Cache>::access(uint32_t core_id, uint32_t client_id, uintptr_t addr) {
    auto& private_cache = get_private_cache(core_id);
    bool hit = private_cache.access(addr);

    if (hit) {
        return true;
    }

    // We didn't hit in L1, try in shared L2
    return shared_cache_.access(client_id, addr);
}

template<class L2Cache>
Cache &MultiLevelCache<L2Cache>::get_private_cache(uint32_t core_id) {
    return private_caches_.at(core_id);
}

template<class L2Cache>
L2Cache &MultiLevelCache<L2Cache>::get_shared_cache() {
    return shared_cache_;
}

template<class L2Cache>
MultiLevelCache<L2Cache>::MultiLevelCache(uint32_t num_cores, const Cache& private_cache, L2Cache shared_cache)
    : shared_cache_(std::move(shared_cache)), private_caches_(num_cores, private_cache)
{
}