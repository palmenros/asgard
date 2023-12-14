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

template <class CacheClass>
struct cluster_t {
    // Each memory node of the cluster is managed using intra-node partitioning.
    CacheClass cache;
    // Cumulative sum
    uint32_t c_sum;
};

template<class CacheClass>
class ClusterPartitioningProposal {
public:
    ClusterPartitioningProposal(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t>& coresPerClient);

//    void read(uint32_t client_id, uintptr_t addr);
//    void write(uint32_t client_id, uintptr_t addr);
    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    std::vector<CacheClass> &clusters();
private:
    // List of clusters per client.
    std::vector<CacheClass> clusters_;
    // Total number of cores the client owns.
    uint32_t n_cores;
};

// TODO(kostas): Discuss with Luis how to initialize this.
template<class CacheClass>
ClusterPartitioningProposal<CacheClass>::ClusterPartitioningProposal(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t>& coresPerClient) {

}

template<class CacheClass>
void ClusterPartitioningProposal<CacheClass>::access(uint32_t client_id, uintptr_t addr) {
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

template<class CacheClass>
uint32_t ClusterPartitioningProposal<CacheClass>::misses(uint32_t client_id) const {
    if (client_id >= clusters_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }

    return clusters_[client_id].inp.misses(client_id);
}

template<class CacheClass>
uint32_t ClusterPartitioningProposal<CacheClass>::hits(uint32_t client_id) const {
    if (client_id >= clusters_.size()) {
        throw std::invalid_argument("Invalid client_id given");
    }

    return clusters_[client_id].inp.hits(client_id);
}

template<class CacheClass>
std::vector<cluster_t<CacheClass>> &ClusterPartitioningProposal<CacheClass>::clusters() {
    return clusters_;
}

template<class CacheClass>
class ClusterPartitioningNormal {
public:
    ClusterPartitioningNormal(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t>& coresPerClient);

    //    void read(uint32_t client_id, uintptr_t addr);
    //    void write(uint32_t client_id, uintptr_t addr);
    void access(uint32_t client_id, uintptr_t addr);
    uint32_t misses(uint32_t client_id) const;
    uint32_t hits(uint32_t client_id) const;
    std::vector<CacheClass> &clusters();
private:
    // List of clusters per client.
    std::vector<CacheClass> clusters_;
    // Total number of cores the client owns.
    uint32_t n_cores;
};

template<class CacheClass>
std::vector<CacheClass> &ClusterPartitioningNormal<CacheClass>::clusters() {
}

template<class CacheClass>
uint32_t ClusterPartitioningNormal<CacheClass>::hits(uint32_t client_id) const {
    return 0;
}

template<class CacheClass>
uint32_t ClusterPartitioningNormal<CacheClass>::misses(uint32_t client_id) const {
    return 0;
}

template<class CacheClass>
void ClusterPartitioningNormal<CacheClass>::access(uint32_t client_id, uintptr_t addr) {
}

template<class CacheClass>
ClusterPartitioningNormal<CacheClass>::ClusterPartitioningNormal(uint64_t cache_size, uint32_t assoc, uint32_t block_size, const std::vector<uint32_t> &coresPerClient) {
}