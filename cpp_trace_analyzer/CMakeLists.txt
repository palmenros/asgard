cmake_minimum_required(VERSION 3.15)
project(cpp_trace_analyzer)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpp_trace_analyzer memory_analyzer.cpp cache.cpp llc_partitioning.cpp)
