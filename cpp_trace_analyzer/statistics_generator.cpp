#include <fstream>
#include <iostream>
#include <iterator>
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
std::vector<uint32_t> getMisses(const std::vector<T>& caches) {
    return mapVector<T, uint32_t>(caches, [](const T& t){
        return t.misses();
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

    uint32_t l2_size = 8 * MiB;

    std::vector<MultiLevelCache<Cache>> caches = {
            {num_cores, L1, Cache(l2_size, 1, block_size)},
            {num_cores, L1, Cache(l2_size, 4, block_size)},
            {num_cores, L1, Cache(l2_size, 8, block_size)},
            {num_cores, L1, Cache(l2_size, 16, block_size)},
            {num_cores, L1, Cache(l2_size, 32, block_size)}
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

//    multiple_private_cache_sizes();
//    multiple_private_cache_assocs();
    separate_trace_file_per_core();
}