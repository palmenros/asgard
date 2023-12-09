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
    uint32_t tag;
};

class CacheSet {
public:
    explicit CacheSet(uint32_t assoc);

    struct CacheLine {
        CacheLineState state;
        uint32_t tag;

        explicit CacheLine() : state(CacheLineState::INVALID), tag(0) {}
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
    Cache(uint64_t cache_size, uint32_t assoc, uint32_t block_size, uint32_t tag_size);

    // Creates a bitmask consisting of ones, of size `bits`.
    // For example, if bits == 2, returns 0b11.
    static uint32_t mask(uint32_t bits) {
        int result = 0;
        for (size_t i = 0; i < bits; i++) {
            result = (result << 1) | 0x1;
        }
        return result;
    }

    void read(uintptr_t addr);
    void write(uintptr_t addr);
    void access(const LocationInfo& loc, bool write);
    uint64_t cache_size() const noexcept;
    uint32_t sets() const noexcept;
    uint32_t block_size() const noexcept;
    uint32_t tag_size() const noexcept;
    uint32_t misses() const noexcept;
    uint32_t hits() const noexcept;
    void update_hits() noexcept;
    void update_misses() noexcept;
private:
    std::vector<CacheSet> cache_;
    uint64_t cache_size_;
    uint32_t sets_, block_size_, tag_size_, misses_, hits_;

    uint32_t compute_sets(uint32_t assoc) const noexcept;
    LocationInfo compute_location_info(uintptr_t addr) const noexcept;
};
