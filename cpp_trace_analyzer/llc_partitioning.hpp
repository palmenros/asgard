#pragma once

#include <cstdint>
#include <vector>

#include "cache.hpp"

// Way partitioning.
class WayPartitioning {
public:
    WayPartitioning(const std::vector<uint32_t>& n_ways, uint64_t slice_size, uint32_t block_size);

    void read(uint32_t client_id, uintptr_t addr) noexcept;
    void write(uint32_t client_id, uintptr_t addr) noexcept;
    uint32_t misses(uint32_t client_id) const noexcept;
    uint32_t hits(uint32_t client_id) const noexcept;
private:
    std::vector<Cache> way_partitioned_caches_;
};

// Inter-node partitioning.
class InterNodePartitioning {
public:
    // clients is the number of clients in the system.
    // n_slices is how many slices each client has.
    // the rest are information for LLC slice.
    InterNodePartitioning(uint32_t clients, const std::vector<uint32_t>& n_slices,
                          uint64_t slice_size, uint32_t assoc, uint32_t block_size);


    void read(uint32_t client_id, uintptr_t addr);
    void write(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id);
    uint32_t hits(uint32_t client_id);
    const std::vector<Cache> &memory_nodes(uint32_t client_id);
private:
    void access(uint32_t client_id, uintptr_t addr, bool write);

    // Memory node list per client.
    std::vector<std::vector<Cache>> memory_nodes_;
};

struct fixed_bits_t {
    std::bitset<32> bits;
    uint32_t n_bits;
};

// Intra-node partitioning.
class IntraNodePartitioning {
public:
    IntraNodePartitioning(uint32_t clients, uint64_t cache_size, uint32_t assoc,
                          uint32_t block_size, std::vector<fixed_bits_t> aux_table);

    void read(uint32_t client_id, uintptr_t addr) noexcept;
    void write(uint32_t client_id, uintptr_t addr) noexcept;
    uint32_t misses(uint32_t client_id) const noexcept;
    uint32_t hits(uint32_t client_id) const noexcept;
private:
    void access(uint32_t client_id, uintptr_t addr, bool write) noexcept;

    Cache cache_;
    std::vector<fixed_bits_t> aux_table_;
    std::vector<std::pair<uint32_t, uint32_t>> stats;
};

// Cluster partitioning
class ClusterPartitioning {
    // TODO(kostas): FILLME.
};
