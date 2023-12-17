#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <vector>

#include "cache.hpp"
#include "llc_partitioning.hpp"

#define ASSERT(cond) \
    do \
    { \
        if (!(cond)) \
        { \
            std::cerr << "Assertion failed: " << #cond << ", at " <<  __FILE__ << ", line " <<  __LINE__ << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

void load_trace(const std::string& name, std::ifstream& out_file) {
    out_file.open("../traces/" + name);
    if (!out_file.is_open()) {
        std::cerr << "Could not open trace file '" << name << "'" << std::endl;
        exit(EXIT_FAILURE);
    }
}

template <class Callable>
void for_each_trace_line(std::ifstream& trace_file, Callable callable) {
    uintptr_t addr, cpu_index;
    bool is_store;
    size_t line_no = 1;
    while (true) {
        // This reads until it encounters white space, not just newline.
        if (!(trace_file >> std::hex >> addr >> std::hex >> cpu_index >> is_store)) {
            if (trace_file.eof()) {
                return;
            }
            std::cerr << "Format error on line " << line_no << std::endl;
            exit(EXIT_FAILURE);
        }
        ++line_no;
        callable(addr, cpu_index, is_store);
//        std::cout << std::hex << addr << " " << std::hex << cpu_index << " " << is_store << std::endl;
    }
}

template <class T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
    out << '[';

    bool first = true;

    for(const auto& e : v) {
        if(first) {
            first = false;
        } else {
            out << ", ";
        }
        out << e;
    }

    out << "]";

    return out;
}

void header(const std::string& str) {
    static uint32_t counter = 0;
    counter++;
    std::cout << "\n[" << counter << "] " << str << "\n" << std::endl;
}

constexpr uint32_t KiB = 1024;
constexpr uint32_t MiB = 1024 * KiB;

template <class InputType, class OutputType, class Callable>
std::vector<OutputType> mapVector(const std::vector<InputType>& v, Callable callable) {
    std::vector<OutputType> out;
    out.reserve(v.size());

    for(const auto& i : v) {
        out.emplace_back(callable(i));
    }

    return out;
}

template <class T>
std::vector<uint32_t> getMisses(const std::vector<T>& caches, uint32_t client_id = 0) {
    return mapVector<T, uint32_t>(caches, [client_id](const T& t){
        return t.misses(client_id);
    });
}

void multiple_private_cache_sizes(const std::string& trace_name) {
    header("Multiple private cache sizes");

    std::ifstream graph_trace;
    load_trace(trace_name, graph_trace);

    // L1: 64KB 4-way, 64-byte blocks

    uint32_t num_cores = 2;
    uint32_t block_size = 64;

    Cache L1 {64* KiB, 4, block_size };

    uint32_t l2_assoc = 8;

    std::vector<MultiLevelCache<Cache>> caches = {
            {num_cores, L1, Cache(2 * MiB, l2_assoc, block_size)},
            {num_cores, L1, Cache(4 * MiB, l2_assoc, block_size)},
            {num_cores, L1, Cache(8 * MiB, l2_assoc, block_size)},
            {num_cores, L1, Cache(16 * MiB, l2_assoc, block_size)},
            {num_cores, L1, Cache(32 * MiB, l2_assoc, block_size)},
            {num_cores, L1, Cache(64 * MiB, l2_assoc, block_size)},
    };

    size_t num_accesses = 0;
    for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index, uint32_t is_store){
        num_accesses++;

        for(auto& cache: caches) {
            cache.access(cpu_index, 0, addr);
        }
    });

    auto misses = getMisses(caches);
    std::cout << "Misses: " << misses << std::endl;

    std::cout << "Analyzed " << num_accesses << " accesses" << std::endl;
}

void multiple_private_cache_assocs(const std::string& trace_name) {
    header("Multiple private cache associativities");

    std::ifstream graph_trace;
    load_trace(trace_name, graph_trace);

    // L1: 64KB 4-way, 64-byte blocks

    uint32_t num_cores = 2;
    uint32_t block_size = 64;

    Cache L1 {64* KiB, 4, block_size };

    uint32_t l2_size = 4 * MiB;

    std::vector<MultiLevelCache<Cache>> caches = {
            {num_cores, L1, Cache(l2_size, 1, block_size)},
            {num_cores, L1, Cache(l2_size, 2, block_size)},
            {num_cores, L1, Cache(l2_size, 4, block_size)},
    };

//    std::vector<Cache> caches = {
//                    Cache(l2_size, 1, block_size),
//                    Cache(l2_size, 2, block_size),
//                    Cache(l2_size, 4, block_size),
//                    Cache(l2_size, 8, block_size),
//                    Cache(l2_size, 16, block_size),
//                    Cache(l2_size, 32, block_size)
//            };

    size_t num_accesses = 0;
    for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index, uint32_t is_store){
        num_accesses++;

        for(auto& cache: caches) {
            cache.access(cpu_index,0, addr);
        }
    });

    auto misses = getMisses(caches);
    std::cout << "Misses: " << misses << std::endl;

    std::cout << "Analyzed " << num_accesses << " accesses" << std::endl;
}

std::vector<uint32_t> getWayPartitionedNumAccesses(std::vector<MultiLevelCache<WayPartitioning>>& caches) {
    return mapVector<MultiLevelCache<WayPartitioning>, uint32_t>(caches, [](const MultiLevelCache<WayPartitioning>& cache) -> uint32_t {
        return cache.num_total_accesses(0);
    });
}

std::vector<uint32_t> getIntraNumAccess(std::vector<MultiLevelCache<IntraNodePartitioning>>& caches) {
    return mapVector<MultiLevelCache<IntraNodePartitioning>, uint32_t>(caches, [](const MultiLevelCache<IntraNodePartitioning>& cache) -> uint32_t {
        return cache.num_total_accesses(0);
    });
}


void intra_vs_way_partitioning(const std::string& trace_name) {
    header("Intra vs. way partitioning");

    std::ifstream graph_trace;
    load_trace(trace_name, graph_trace);


    uint32_t num_cores = 2;
    uint32_t block_size = 64;

    // L1: 64KB, 4-way, 64-byte blocks
    Cache L1 {64* KiB, 4, block_size};

    // L2: total 8-way.

    std::vector<uint32_t> sizes = {8*MiB, 16*MiB, 32*MiB, 64*MiB, 128*MiB, 256*MiB, 512*MiB};

    std::vector<uint32_t> n_ways = {1, 7};
    //  Sum up all associativity
    uint32_t total_assoc = std::reduce(n_ways.begin(), n_ways.end());

    // Way partitioning
    std::vector<MultiLevelCache<WayPartitioning>> way_partitioned_caches;
    way_partitioned_caches.reserve(sizes.size());
    for(auto size : sizes) {
        way_partitioned_caches.emplace_back(num_cores, L1, WayPartitioning{size, block_size, n_ways});
    }

    // Intra-node partitioning

    // TODO: Review that the table is well constructed

    // Four clients:
    //  0 -> 1 core   -> 3 fixed bits -> 000
    //  1 -> 1 core   -> 3 fixed bits -> 001
    //  2 -> 2 cores  -> 2 fixed bits -> 01
    //  3 -> 4 cores  -> 1 fixed bit  -> 1

    std::vector<fixed_bits_t> aux_table = {
            fixed_bits_t{std::bitset<32>(0b000), 3},
            fixed_bits_t{std::bitset<32>(0b001), 3},
            fixed_bits_t{std::bitset<32>(0b01), 2},
            fixed_bits_t{std::bitset<32>(0b1), 1},
    };

    std::vector<MultiLevelCache<IntraNodePartitioning>> intra_node_caches;
    intra_node_caches.reserve(sizes.size());
    for(auto size: sizes) {
        auto shared_cache = IntraNodePartitioning{size, total_assoc, block_size, aux_table};
        intra_node_caches.emplace_back(num_cores, L1, std::move(shared_cache));
    }

    size_t num_accesses = 0;
    for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index, uint32_t is_store){
        num_accesses++;

        for(auto& cache: way_partitioned_caches) {
            cache.access(cpu_index,0, addr);
        }

        for(auto& cache: intra_node_caches) {
            cache.access(cpu_index,0, addr);
        }
    });

    std::cout << "{\n";
    std::cout << "'cache_slice_sizes': " << sizes << ',' << std::endl;

    auto way_partitioned_misses = getMisses(way_partitioned_caches);
    std::cout << "'way_partition_misses': " << way_partitioned_misses << ',' << std::endl;

    auto intra_node_misses = getMisses(intra_node_caches);
    std::cout << "'intra_node_misses': " << intra_node_misses << ',' << std::endl;

    std::cout << "'way_partition_accesses': " << getWayPartitionedNumAccesses(way_partitioned_caches) << ',' << std::endl;
    std::cout << "'intra_node_accesses': " << getIntraNumAccess(intra_node_caches) << ',' << std::endl;

    std::cout << "'total_analyzed': " << num_accesses << ',' << std::endl;
    std::cout << "}" << std::endl;
}

// For each cache (a test run), returns a vector of clusters and how many accesses each cluster has received. Will not show every cluster, only the clusters the client has accessed
std::vector<std::vector<uint32_t>> getInterNodeNumAccesses(std::vector<MultiLevelCache<InterNodePartitioning>>& caches) {
    std::vector<std::vector<uint32_t>> res;
    res.reserve(caches.size());

    for(auto& multi_level_cache : caches) {
        auto& cache = multi_level_cache.get_shared_cache();
        auto& memory_nodes = cache.memory_nodes(0);

        std::vector<uint32_t> accesses = mapVector<Cache, uint32_t>(memory_nodes, [](const Cache& cache){
            return cache.hits() + cache.misses();
        });

        res.push_back(accesses);
    }

    return res;
}

std::vector<std::vector<uint32_t>> getClusterWayPartitionedNumAccesses(std::vector<MultiLevelCache<ClusterWayPartitioning>>& caches) {
    std::vector<std::vector<uint32_t>> res;
    res.reserve(caches.size());

    for(auto& multi_level_cache : caches) {
        auto& cache = multi_level_cache.get_shared_cache();
        auto& memory_nodes = cache.clusters();

        std::vector<uint32_t> accesses = mapVector<WayPartitioning, uint32_t>(memory_nodes, [](const WayPartitioning& cache){
            return cache.hits(0) + cache.misses(0);
        });

        res.push_back(accesses);
    }

    return res;
}

std::vector<std::vector<uint32_t>> getInterIntraNumAccesses(std::vector<MultiLevelCache<InterIntraNodePartitioning>>& caches, uint32_t num_clusters) {
    std::vector<std::vector<uint32_t>> res;
    res.reserve(caches.size());

    for(auto& multi_level_cache : caches) {
        auto& cache = multi_level_cache.get_shared_cache();

        std::vector<uint32_t> accesses;
        accesses.reserve(num_clusters);

        for(uint32_t cluster_id = 0; cluster_id < num_clusters; cluster_id++) {
            auto& memory_nodes = cache.get_cache_slice(0, cluster_id);
            accesses.push_back(memory_nodes.hits() + memory_nodes.misses());
        }

        res.push_back(accesses);
    }

    return res;
}


void inter_vs_cluster_way_partitioning_vs_inter_intra(const std::string& trace_name) {
    uint32_t num_cores = 2;
    uint32_t block_size = 64;

    // L1: 64KB, 4-way, 64-byte blocks
    Cache L1 {64* KiB, 4, block_size};

    // System with 8 clusters. Each cluster has only one core.
    // Client 0 (our client) only has cores in num_slices_our_client_has of them (see below test function)
    uint32_t num_clusters = 8;

    // Size of a single slice
    std::vector<uint32_t> sizes = {2*MiB, 4*MiB, 8*MiB, 16*MiB, 32*MiB, 64*MiB, 128*MiB};

    auto test = [&](uint32_t num_slices_our_client_has) {
        ASSERT(num_slices_our_client_has > 0);
        ASSERT(num_slices_our_client_has <= num_clusters);

        std::ifstream graph_trace;
        load_trace(trace_name, graph_trace);


        // Inter partitioning

        ASSERT(num_clusters >= 2);
        std::vector<uint32_t> n_slices(num_clusters, 0);
        n_slices[0] = num_slices_our_client_has;
        n_slices[1] = num_clusters - num_slices_our_client_has;

        std::vector<MultiLevelCache<InterNodePartitioning>> inter_node_partitioned_caches;
        for(auto size : sizes) {
            auto shared_cache = InterNodePartitioning{size, num_clusters, block_size, n_slices};
//            std::cerr << mapVector<Cache, uint32_t>(shared_cache.memory_nodes(0), [](const Cache& cache) -> uint32_t { return cache.cache_size(); }) << std::endl;

            inter_node_partitioned_caches.emplace_back(num_cores, L1, shared_cache);
        }

        // Cluster way partitioning

        std::vector<uint32_t> n_ways = {num_slices_our_client_has, num_clusters - num_slices_our_client_has};

        std::vector<MultiLevelCache<ClusterWayPartitioning>> way_partitioned_caches;
        way_partitioned_caches.reserve(sizes.size());
        for(auto size : sizes) {
            // Cache size is per slice?
            auto shared_cache = ClusterWayPartitioning{num_clusters, size, block_size, n_ways};
//            std::cerr << mapVector<WayPartitioning, uint32_t>(shared_cache.clusters(), [](const WayPartitioning& way_partitioning) -> uint32_t { return way_partitioning.get_cache(0).cache_size(); }) << std::endl;

            way_partitioned_caches.emplace_back(num_cores, L1, shared_cache);
        }

        // Inter-intra node partitioning
        std::vector<MultiLevelCache<InterIntraNodePartitioning>> inter_intra_node_caches;
        inter_intra_node_caches.reserve(sizes.size());
        for(auto size: sizes) {
            ASSERT(size % num_clusters == 0);
            uint32_t cache_slice_size = size;

            // n_cache_sizes[clusterId][client] -> Size of the private cache of that client
            std::vector<std::vector<uint32_t>> n_cache_sizes(num_clusters, std::vector<uint32_t>(2, 0));

            std::vector<inter_intra_aux_table_entry_t> own_slices;
            own_slices.resize(num_slices_our_client_has);

            uint32_t slice_id = 0;
            for(slice_id = 0; slice_id < num_slices_our_client_has; ++slice_id) {
                own_slices[slice_id] = inter_intra_aux_table_entry_t {
                        slice_id, slice_id+1
                };
                n_cache_sizes[slice_id][0] = cache_slice_size;
            }

            std::vector<inter_intra_aux_table_entry_t> other_slices;
            other_slices.resize(num_clusters - num_slices_our_client_has);

            for(uint32_t i = 0; i < num_clusters - num_slices_our_client_has; ++i) {
//                other_slices[i] = inter_intra_aux_table_entry_t {
//                    slice_id + i, i + 1
//                };
//                n_cache_sizes[slice_id + i][1] = cache_slice_size;
            }

            std::vector<inter_intra_aux_table_t> aux_tables_per_client = {
                    inter_intra_aux_table_t{num_slices_our_client_has, own_slices},
                    inter_intra_aux_table_t{num_clusters - num_slices_our_client_has, other_slices}
            };

            auto shared_cache = InterIntraNodePartitioning{num_clusters, block_size, n_cache_sizes, aux_tables_per_client};
//
//            for(uint32_t i = 0; i < num_clusters; i++) {
//                auto& cache = shared_cache.get_cache_slice(0, i);
//                std::cerr << cache.cache_size() << ", ";
//            }
//
//            std::cerr << std::endl;

            inter_intra_node_caches.emplace_back(num_cores, L1, std::move(shared_cache));
        }

        size_t num_accesses = 0;
        for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index, uint32_t is_store){
//            std::cout << num_accesses << "/" << expected_num_accesses << std::endl;
            num_accesses++;

            for(auto& cache: inter_node_partitioned_caches) {
                cache.access(cpu_index,0, addr);
            }

            for(auto& cache: way_partitioned_caches) {
                cache.access(cpu_index,0, addr);
            }

            // TODO: Enable after inter_intra_node_caches works properly
            for(auto& cache: inter_intra_node_caches) {
                cache.access(cpu_index,0, addr);
            }
        });

        std::cout <<  num_slices_our_client_has << ": {\n";
        std::cout << "'cache_slice_sizes': " << sizes << ',' << std::endl;

        auto inter_node_misses = getMisses(inter_node_partitioned_caches);
        std::cout << "'inter_node_misses': " << inter_node_misses << ',' << std::endl;

        auto way_partitioned_misses = getMisses(way_partitioned_caches);
        std::cout << "'way_partition_misses': " << way_partitioned_misses << ',' << std::endl;

        auto intra_node_misses = getMisses(inter_intra_node_caches);
        std::cout << "'inter_intra_misses': " << intra_node_misses << ',' << std::endl;

        auto inter_node_accesses = getInterNodeNumAccesses(inter_node_partitioned_caches);
        std::cout << "'inter_node_accesses': " << inter_node_accesses << ',' << std::endl;
        
        auto way_partitioned_accesses = getClusterWayPartitionedNumAccesses(way_partitioned_caches);
        std::cout << "'way_partition_accesses': " << way_partitioned_accesses << ',' << std::endl;
        
        auto intra_node_accesses = getInterIntraNumAccesses(inter_intra_node_caches, num_clusters);
        std::cout << "'inter_intra_accesses': " << intra_node_accesses << ',' << std::endl;

        std::cout << "'total_analyzed': " << num_accesses << "\n},"  << std::endl;
    };

    std::stringstream ss;
    ss << "Inter vs. way partitioning. Client owns varying number of slices out of "  << num_clusters;
    header(ss.str());

    std::cout << "{" << std::endl;

    test(1);
    test(2);
    test(4);
    test(8);

    std::cout << "}" << std::endl;
}

void access_uniformity_way_vs_inter_intra(const std::string& trace_name) {
    header("Access uniformity way vs inter-intra...");

    std::ifstream graph_trace;
    load_trace(trace_name, graph_trace);

    // Test-case:
    //  8 clusters, each cluster with 2 cores
    //  The client has 3 cores (2 in one cluster, 1 in another cluster)
    //  Compare the distribution of access between Cluster Way Partitioning and Inter-Intra node partitioning


    uint32_t block_size = 64;

    // L1: 64KB, 4-way, 64-byte blocks
    Cache L1 {64* KiB, 4, block_size};

    // Size of a single slice
    std::vector<uint32_t> sizes = {2*MiB, 4*MiB, 8*MiB, 16*MiB, 32*MiB, 64*MiB, 128*MiB};

    // BE CAREFUL! Updating this parameters may require manually updating some of the tables below.
    uint32_t num_clusters = 8;
    uint32_t cores_per_cluster = 2;
    uint32_t cores_owned_by_us = 3;
    uint32_t num_cores = num_clusters * cores_per_cluster;

    // Cluster way partitioning

    std::vector<uint32_t> n_ways = {cores_owned_by_us, num_clusters * cores_per_cluster - cores_owned_by_us};

    std::vector<MultiLevelCache<ClusterWayPartitioning>> way_partitioned_caches;
    way_partitioned_caches.reserve(sizes.size());
    for(auto size : sizes) {
        // TODO: Is cache-size per slice?
        way_partitioned_caches.emplace_back(num_cores, L1, ClusterWayPartitioning{num_clusters, size, block_size, n_ways});
    }


    // Inter-intra node partitioning

    std::vector<MultiLevelCache<InterIntraNodePartitioning>> inter_intra_node_caches;
    inter_intra_node_caches.reserve(sizes.size());
    for(auto size: sizes) {
        uint32_t cache_slice_size = size;
        ASSERT(cache_slice_size % 2 == 0);

        // n_cache_sizes[clusterId][client] -> Size of the private cache of that client
        std::vector<std::vector<uint32_t>> n_cache_sizes(num_clusters, std::vector<uint32_t>(2, 0));
        n_cache_sizes[0][0] = cache_slice_size;
        n_cache_sizes[1][0] = cache_slice_size / 2;


        std::vector<inter_intra_aux_table_entry_t> own_slices = {
                {0, 2},
                {1, 3},
        };


        std::vector<inter_intra_aux_table_entry_t> other_slices = {
                {1, 1},
                {2, 3},
                {3, 5},
                {4, 7},
                {5, 9},
                {6, 11},
                {7, 13},
        };

        std::vector<inter_intra_aux_table_t> aux_tables_per_client = {
                inter_intra_aux_table_t{3, own_slices},
                inter_intra_aux_table_t{13, other_slices}
        };

        auto shared_cache = InterIntraNodePartitioning{num_clusters * cores_per_cluster, block_size, n_cache_sizes, aux_tables_per_client};
        inter_intra_node_caches.emplace_back(num_cores, L1, std::move(shared_cache));
    }


    size_t num_accesses = 0;
    for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index, uint32_t is_store){
        //            std::cout << num_accesses << "/" << expected_num_accesses << std::endl;
        num_accesses++;

        for(auto& cache: way_partitioned_caches) {
            cache.access(cpu_index,0, addr);
        }

        // TODO: Enable after inter_intra_node_caches works properly
        for(auto& cache: inter_intra_node_caches) {
            cache.access(cpu_index,0, addr);
        }
    });

    std::cout << "Cache slice sizes: " << sizes << std::endl;

    auto way_partitioned_misses = getMisses(way_partitioned_caches);
    std::cout << "Way Partition Misses: " << way_partitioned_misses << std::endl;

    auto intra_node_misses = getMisses(inter_intra_node_caches);
    std::cout << "Inter-Intra Node Misses: " << intra_node_misses << std::endl;

    auto way_partitioned_accesses = getClusterWayPartitionedNumAccesses(way_partitioned_caches);
    std::cout << "Way Partition Accesses: " << way_partitioned_accesses << std::endl;

    auto intra_node_accesses = getInterIntraNumAccesses(inter_intra_node_caches, num_clusters);
    std::cout << "Inter-Intra Node Accesses: " << intra_node_accesses << std::endl;


    std::cout << "Analyzed " << num_accesses << " accesses" << std::endl;

}

void separate_trace_file_per_core() {
    header("Separating trace file into one per core...");


    std::ifstream graph_trace;
    load_trace("qemu_graph_trace", graph_trace);

    uint32_t num_cores = 2;
    std::vector<std::ofstream> output_files{num_cores};
    for(int i = 0; i < num_cores; i++) {
        std::stringstream ss;
        ss << "../traces/qemu_graph_trace_core_" << i;
        output_files[i].open(ss.str());
        if (!output_files[i].is_open()) {
            std::cerr << "Couldn't open file '" << ss.str() << "'" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::vector<uint32_t> num_lines{num_cores, {}};

    for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index, uint32_t is_store){
        num_lines[cpu_index]++;

        auto& out = output_files[cpu_index];
        out << "0x" << std::hex << addr << std::hex << " " << cpu_index << " " << is_store << "\n";
    });

    std::cout << "Number of accesses per core: " << num_lines << std::endl;
}

void generate_stats() {
    std::cout << "Generating stats..." << std::endl;
    //    separate_trace_file_per_core();

    std::string trace_name = "qemu_graph_trace_page_rank";

//    multiple_private_cache_sizes(trace_name);
//    multiple_private_cache_assocs(trace_name);
    intra_vs_way_partitioning(trace_name);
//    inter_vs_cluster_way_partitioning_vs_inter_intra(trace_name);

//    access_uniformity_way_vs_inter_intra(trace_name);
}