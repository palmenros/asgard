#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "statistics_generator.hpp"

static const std::string& get_opt(const std::vector<std::string>& args, const std::string& option) {
    auto itr =  std::find(args.begin(), args.end(), option);
    if (itr != args.end() && ++itr != args.end()){
        return *itr;
    }
    static const std::string empty_string;
    return empty_string;
}

static void analyze_trace_file(const std::string& trace_file_name) {
    std::ifstream trace_file(trace_file_name);

    if (!trace_file.is_open()) {
        std::cerr << "Could not open " << trace_file_name << std::endl;
        return;
    }

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
            return;
        }
        ++line_no;
        std::cout << std::hex << addr << " " << std::hex << cpu_index << " " << is_store << std::endl;
    }
}

int main_standalone(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "<usage> memory_analyzer -f <trace_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> args;

    for (int i = 1; i < argc; i++) {
        args.emplace_back(argv[i]);
    }

    auto trace_file_name = get_opt(args, "-f");
    if (trace_file_name.empty()) {
        std::cerr << "<usage> memory_analyzer -f <trace_file>" << std::endl;
        return EXIT_FAILURE;
    }

    analyze_trace_file(trace_file_name);

    return 0;
}

int main_statistics(int arg, char** argv) {
    generate_stats();
    return 0;
}

int main(int argc, char** argv) {
    main_statistics(argc, argv);
    return 0;
}