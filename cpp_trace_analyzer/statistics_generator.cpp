#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <vector>

#include "cache.hpp"
#include "llc_partitioning.hpp"

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
    std::cout << "[" << counter << "] " << str << std::endl;
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

void multiple_private_cache_sizes() {
    header("Multiple private cache sizes");

    std::ifstream graph_trace;
    load_trace("qemu_graph_trace", graph_trace);

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

void multiple_private_cache_assocs() {
    header("Multiple private cache associativities");

    std::ifstream graph_trace;
    load_trace("qemu_graph_trace", graph_trace);

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

void intra_vs_way_partitioning() {

    header("Multiple private cache associativities");

    std::ifstream graph_trace;
    load_trace("qemu_graph_trace", graph_trace);


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
        intra_node_caches.emplace_back(num_cores, L1, IntraNodePartitioning{size, total_assoc, block_size, aux_table});
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

    std::cout << "Cache sizes: " << sizes << std::endl;

    auto way_partitioned_misses = getMisses(way_partitioned_caches);
    std::cout << "Way Partition Misses: " << way_partitioned_misses << std::endl;

    auto intra_node_misses = getMisses(intra_node_caches);
    std::cout << "Intra Node Misses: " << intra_node_misses << std::endl;

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


//    multiple_private_cache_sizes();
//    multiple_private_cache_assocs();
    intra_vs_way_partitioning();

}