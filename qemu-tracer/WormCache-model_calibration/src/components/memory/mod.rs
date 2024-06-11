mod checkpoint;
mod mtr;
mod per_core_record;
mod ts_cache;
mod ts_model;
mod ts_set;

use std::ffi;
use std::fs;
use std::fs::File;
use std::io::Write;
use std::ptr::null;

pub use checkpoint::CacheBlockState;
pub use checkpoint::PrivateCacheParameters;
pub use per_core_record::TimestampSingleCoreMemoryHierarchy;
pub use ts_cache::TimestampCache;
pub use ts_cache::TimestampCacheMetaData;
pub use ts_model::TimestampMemoryHierarchy;
pub use ts_set::CacheReturnResult;
pub use ts_set::TimestampCacheLineStatus;
pub use ts_set::TimestampCacheSet;

use crate::qemu_api;
use crate::CORE_COUNT;

use once_cell::sync::Lazy;

static PLUGIN: Lazy<
    TimestampMemoryHierarchy<
        { crate::PRI_CACHE_ASSO },
        { crate::PRI_CACHE_SET },
        { crate::SHARED_CACHE_ASSO },
        { crate::SHARED_CACHE_SET },
    >,
> = Lazy::new(|| TimestampMemoryHierarchy::new(crate::CORE_COUNT));

fn get_memory_ts() -> u128 {
    return std::time::SystemTime::now()
        .duration_since(std::time::SystemTime::UNIX_EPOCH)
        .unwrap()
        .as_nanos() as u128;
}

static mut trace_log_file : Option<File> = None;

unsafe extern "C" fn vcpu_mem_access(
    cpu_idx: u32,
    info: qemu_api::qemu_plugin_meminfo_t,
    vaddr: u64,
    _: *mut ffi::c_void, // should be NULL.
) {
    let hw_handler = qemu_api::qemu_plugin_get_hwaddr(info, vaddr);
    let is_device = qemu_api::qemu_plugin_hwaddr_is_io(hw_handler);

    if !is_device {
        let is_store = qemu_api::qemu_plugin_mem_is_store(info);
        let paddr = qemu_api::qemu_plugin_hwaddr_phys_addr(hw_handler) as usize;

        trace_log_file.as_ref().unwrap().write_all(format!("{:#x} {} {}\n", paddr, cpu_idx, is_store as i8).as_bytes()).unwrap();

        // For Asgard: To replace with your trace analyzer
        // println!("Physcial address: {} from core {}.", paddr, cpu_idx);
    } else {
        
    }
}

unsafe extern "C" fn vcpu_insn_exec(
    cpu_idx: u32,
    paddr: *mut ffi::c_void, // it is basically its physical address.
) {
    let paddr = paddr as usize;
    // println!("Physcial address: {} from core {}.", paddr, cpu_idx);
}

// #[cfg(target_pointer_width = "64")]
// #[derive(Debug)]
// pub struct PluginFetchBlockContext {
//     /// The virtual address of the first instruction.
//     pub va: usize,
//     /// The physical address of the first instruction.
//     pub pa: usize,
//     /// The number of instructions in this fetch block.
//     pub size: usize,
// }

// static FETCH_BLOCK_CONTEXT_MAP: Lazy<Mutex<HashMap<usize, Box<PluginFetchBlockContext>>>> =
//     Lazy::new(|| Mutex::new(HashMap::new()));

pub struct MemoryPlugin {}

impl super::Plugin for MemoryPlugin {

    #[inline]
    fn init() {
        println!("Memory plugin initialized.");
        unsafe {
            trace_log_file = Some(File::create("/mnt/sdc/outside_home/asgard/alpine_image/traces/trace").unwrap());
        }
        println!("File created!");
    }

    #[inline]
    fn dump_snapshot() {
        println!("Dumping snapshot!");
    }

    #[inline]
    unsafe fn on_translation(tb: *mut crate::qemu_api::qemu_plugin_tb) {
        let n_instruction = qemu_api::qemu_plugin_tb_n_insns(tb);

        if n_instruction == 0 {
            return;
        }

        let mut block_id = vec![];
        for i in 0..n_instruction {
            let inst = qemu_api::qemu_plugin_tb_get_insn(tb, i);
            block_id.push(
                qemu_api::qemu_plugin_insn_haddr(inst) as usize
                    >> crate::parameter::CACHE_LINE_SIZE.trailing_zeros(),
            );
        }

        let fb_info = crate::util::find_fetch_block_from_block_id_sequence(block_id);

        // bind the instruction call back.
        for (idx, _) in fb_info.into_iter() {
            let i = qemu_api::qemu_plugin_tb_get_insn(tb, idx);
            qemu_api::qemu_plugin_register_vcpu_insn_exec_cb(
                i,
                Some(vcpu_insn_exec),
                qemu_api::qemu_plugin_cb_flags_QEMU_PLUGIN_CB_NO_REGS,
                qemu_api::qemu_plugin_insn_haddr(i) as *mut ffi::c_void,
            );
        }

        // bind the memory callback.
        for i in 0..n_instruction {
            let inst = qemu_api::qemu_plugin_tb_get_insn(tb, i);
            qemu_api::qemu_plugin_register_vcpu_mem_cb(
                inst,
                Some(vcpu_mem_access),
                qemu_api::qemu_plugin_cb_flags_QEMU_PLUGIN_CB_NO_REGS,
                qemu_api::qemu_plugin_mem_rw_QEMU_PLUGIN_MEM_RW,
                std::ptr::null_mut(),
            );
        }
    }
}
