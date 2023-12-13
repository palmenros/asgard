#pragma once

#include <cstdint>
#include <vector>

#include "cache.hpp"

class WayPartitioning {
public:
    WayPartitioning(uint64_t cache_size, uint32_t block_size, const std::vector<uint32_t>& n_ways);

//    void read(uint32_t client_id, uintptr_t addr);
//    void write(uint32_t client_id, uintptr_t addr);
    void access(uint32_t client_id, uintptr_t addr);
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
    InterNodePartitioning(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t>& n_slices);


//    void read(uint32_t client_id, uintptr_t addr);
//    void write(uint32_t client_id, uintptr_t addr);
    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id);
    uint32_t hits(uint32_t client_id);
    const std::vector<Cache> &memory_nodes(uint32_t client_id);
private:
//    void access(uint32_t client_id, uintptr_t addr, bool write);

    // Memory node list per client.
    std::vector<std::vector<Cache>> memory_nodes_;
};

struct fixed_bits_t {
    std::bitset<32> bits;
    uint32_t n_bits;
};

class IntraNodePartitioning {
public:
    IntraNodePartitioning(uint64_t cache_size, uint32_t assoc, uint32_t block_size, std::vector<fixed_bits_t> aux_table);

//    void read(uint32_t client_id, uintptr_t addr);
//    void write(uint32_t client_id, uintptr_t addr);
    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    Cache &cache();
private:
//    void access(uint32_t client_id, uintptr_t addr, bool write);

    Cache cache_;
    std::vector<fixed_bits_t> aux_table_;
    std::vector<std::pair<uint32_t, uint32_t>> stats_;
};

struct cluster_t {
    // Each memory node of the cluster is managed using intra-node partitioning.
    IntraNodePartitioning inp_cache;
    // Cumulative sum
    uint32_t c_sum;
};

class ClusterPartitioning {
public:
    ClusterPartitioning();

//    void read(uint32_t client_id, uintptr_t addr);
//    void write(uint32_t client_id, uintptr_t addr);
    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
private:
    // List of clusters per client.
    std::vector<cluster_t> clusters_;
    // Total number of cores the client owns.
    uint32_t n_cores;
};
