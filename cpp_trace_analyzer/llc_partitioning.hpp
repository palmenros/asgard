#pragma once

#include <cstdint>
#include <vector>

#include "cache.hpp"

class WayPartitioning {
public:
    WayPartitioning(uint64_t cache_size, uint32_t block_size, const std::vector<uint32_t> &n_ways);

    // Returns if its a hit or not.
    bool access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
private:
    std::vector<Cache> way_partitioned_caches_;
};

class InterNodePartitioning {
public:
    // clients is the number of clients in the system.
    // n_slices is how many slices each client has.
    // the rest are information for LLC slice.
    InterNodePartitioning(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t> &n_slices);

    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id);
    uint32_t hits(uint32_t client_id);
    const std::vector<Cache> &memory_nodes(uint32_t client_id);
private:
    // Memory node list per client.
    // memory_nodes[i][j] = slice j of client i
    std::vector<std::vector<Cache>> memory_nodes_;
};

struct fixed_bits_t {
    std::bitset<32> bits;
    uint32_t n_bits;
};

class IntraNodePartitioning {
public:
    IntraNodePartitioning(uint64_t cache_size, uint32_t assoc, uint32_t block_size, std::vector<fixed_bits_t> aux_table);

    void access(uint32_t client_id, uintptr_t addr);
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

    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    std::vector<WayPartitioning> &clusters();
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
                               const std::vector<inter_intra_aux_table_t>& aux_tables_per_client,
                               uint32_t max_bitwidth);

    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    Cache& get_cache_slice(uint32_t client_id, uint32_t cluster_id);
private:
    std::vector<inter_intra_aux_table_t> aux_tables_per_client_;
    // inp[client][cluster] -> Cache of that client has in cluster.
    std::vector<std::vector<Cache>> inp_;
    // *maximum* number of bits to get after the set bits from right to left in order to do modulo and compare between cum_sum
    uint32_t max_bitwidth_;
    uint32_t block_size_;
    // Set bits is the maximum number of set bits for each cache.
    uint32_t set_bits_;
    // Hits/Misses per client.
    std::vector<std::pair<uint32_t, uint32_t>> stats_;
};