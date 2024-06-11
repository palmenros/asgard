# How to run 

- `run_vm.sh`: runs the QEMU virtual machine without the Rust plugin active
- `load_cp.sh`: runs the QEMU virtual machine without the Rust plugin from a given checkpoint
	* To create a checkpoint inside QEMU, run the benchmark where we want to set the checkpoint and type `ctrl-a c` (press `ctrl-a`, release all the keys, type `c`) and then enter the command `savevm <name>` to save the checkpoint. 
	* In order to run QEMU from a given checkpoint run `load_cp.sh -loadvm <name>`
- `with_plugin.sh`: runs the QEMU virtual machine from the checkpoint base1 with the Rust plugin loaded.
	* To run QEMU from a given checkpoint with the plugin run `load_cp.sh -loadvm <name>`

# How to code the plugin

The base rust project is in `WormCache-model_calibration`.

To build the the rust project run `cargo build`.

## Folders

- `src/components/memory/mod.rs`: Base module for the Rust plugin
	- `MemoryPlugin::init` initializes the plugin
	- `vcpu_mem_access`: Where we get called on every address translation. `vaddr` holds the virtual address and `paddr` the physical address. `is_store` tells us whether we have an store. `cpu_idx` tells us the index of the CPU.
	- `dump_snapshot`: Dumps an snapshot.
- `src/cache`: Holds the implementation for PrivateCache. Can be extended to support more advanced cache models. 

# Workflow

1. We set up a test and take a checkpoint at the beginning of the execution. How do we do it without `ctrl-a c`?
2. We run `with_plugin.sh` with the new checkpoint
3. From the Rust Plugin we output to a file (in some format to be determined later) each memory access.
4. From a C++ program, we run some statistics on the output.

# Rust plugin output format

The output file will be a list of lines. Each line will be of the following format:
```
{cpu_idx} {vaddr} {paddr} {is_store}
```

# Benchmarks

- Graph analytics: https://github.com/sbeamer/gapbs
