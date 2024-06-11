# Asgard

Asgard is a hardware-partitioning technique for terabyte-scale bare-metal cloud servers using the Midgard address space. It improves granularity for core assignment to clients in bare-metal cloud and reduces unnecessary Last-Level Cache (LLC) communication and physical sharing, while maintaining high associativity.

## Report

An in-depth report of Asgard can be read in `Report.pdf`, and a poster can be found in `Poster.pdf`.

## Folders

- `cpp_trace_analyzer` contains a C++ cache simulator that reads the generated QEMU memory access traces and simulates different multi-level partitioned cache architectures, producing statistics that are used in the report.
- `qemu-tracer` is a Rust QEMU plugin that generates memory access trace files to be analyzed by the C++ cache simulator.
- `plotting` contains Python scripts for producing the different graphs exposed in the report and poster.
