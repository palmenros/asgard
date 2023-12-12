#pragma once

#include <cstdint>
#include <cstdlib>
#include <vector>

enum class CacheLineState {
    VALID,
    DIRTY,
    INVALID
};

struct LocationInfo {
    uint32_t set_index;
    uint64_t tag;
};

class CacheSet {
public:
    explicit CacheSet(uint32_t assoc);

    struct CacheLine {
        CacheLineState state;
        uint32_t tag;
        uint64_t addr; // For debug.

        explicit CacheLine() : state(CacheLineState::INVALID), tag(0), addr(0) {}
    };

    uint32_t associativity() const noexcept;
    uint32_t evict();
    void update_lru(uint32_t way, bool is_valid);
    CacheLine& cache_line(uint32_t way);
private:
    uint32_t assoc_;
    std::vector<uint32_t> lru_stats_;
    std::vector<CacheLine> cache_lines_;
};

class Cache {
public:
    Cache(uint64_t cache_size, uint32_t assoc, uint32_t block_size);

    // Creates a bitmask consisting of ones, of size `bits`.
    // For example, if bits == 2, returns 0b11.
    static uint32_t mask(uint32_t bits) {
        int result = 0;
        for (size_t i = 0; i < bits; i++) {
            result = (result << 1) | 0x1;
        }
        return result;
    }

    bool exists(uintptr_t addr);
    void read(uintptr_t addr);
    void write(uintptr_t addr);
    void access(const LocationInfo& loc, bool write, uintptr_t addr);
    uint64_t cache_size() const noexcept;
    uint32_t sets() const noexcept;
    uint32_t block_size() const noexcept;
    uint32_t tag_bits() const noexcept;
    uint32_t misses() const noexcept;
    uint32_t hits() const noexcept;
    void update_hits() noexcept;
    void update_misses() noexcept;
private:
    std::vector<CacheSet> cache_;
    // Actual size of the cache.
    uint64_t cache_size_;
    // Block bytes in bytes.
    uint32_t block_size_;
    // Tag bits.
    uint32_t tag_bits_;
    uint32_t misses_, hits_;

    uint32_t compute_sets(uint32_t assoc) const;
    LocationInfo compute_location_info(uintptr_t addr) const noexcept;
};
