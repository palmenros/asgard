#include "cache.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

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
        callable(addr, cpu_index);
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

constexpr uint32_t KB = 1024;
constexpr uint32_t MB = 1024 * KB;

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
    uint32_t assoc = 4;
    uint32_t block_size = 64;

    std::vector<Cache> caches = {
            {16*KB, assoc, block_size},
            {32*KB, assoc, block_size},
            {64*KB, assoc, block_size},
            {128*KB, assoc, block_size},
    };

    size_t num_accesses = 0;
    for_each_trace_line(graph_trace, [&](uintptr_t addr, uintptr_t cpu_index){
        num_accesses++;

        for(auto& cache: caches) {
            cache.access(addr);
        }
    });

    auto misses = getMisses(caches);
    std::cout << "Misses: " << misses << std::endl;

    std::cout << "Analyzed " << num_accesses << " accesses" << std::endl;
}

void generate_stats() {
    std::cout << "Generating stats..." << std::endl;

    multiple_private_cache_sizes();;
}