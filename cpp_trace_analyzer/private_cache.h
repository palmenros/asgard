#pragma once

#include <cstdint>
#include <cstdlib>
#include <vector>

class PrivateCache {
public:
    PrivateCache(uint64_t cache_size, uint32_t assoc, uint32_t block_size, uint32_t tag_size)
        : cache_size_(cache_size), assoc_(assoc), block_size_(block_size), tag_size_(tag_size) {

        sets_ = compute_sets();
        cache_.resize(sets_, std::vector<CacheLine>(assoc_, CacheLine()));
        lru_.resize(sets_);
        for (auto& row : lru_) {
            row.resize(assoc_);
            for (size_t i = 0; i < assoc_; i++) {
                row[i] = i;
            }
        }
    }

    ~PrivateCache() = default;
    PrivateCache(const PrivateCache&) = delete;
    PrivateCache(PrivateCache&&) = delete;
    PrivateCache& operator=(const PrivateCache&) = delete;
    PrivateCache& operator=(PrivateCache&&) = delete;

    void read(uintptr_t addr);
    void write(uintptr_t addr);
    uint32_t evict(uint32_t set_index);
    uint64_t cache_size() const noexcept;
    uint32_t sets() const noexcept;
    uint32_t associativity() const noexcept;
    uint32_t block_size() const noexcept;
    uint32_t tag_size() const noexcept;
    uint32_t misses() const noexcept;
    uint32_t hits() const noexcept;
private:
    enum class CacheLineState {
        VALID,
        DIRTY,
        INVALID
    };

    struct LocationInfo {
        uint32_t block_offset;
        uint32_t set_index;
        uint32_t tag;
    };

    struct CacheLine {
        CacheLineState state;
        uint32_t tag;

        explicit CacheLine() : state(CacheLineState::INVALID), tag(0) {}
    };

    uint64_t cache_size_;
    uint32_t sets_, assoc_, block_size_, tag_size_, misses_, hits_;
    std::vector<std::vector<CacheLine>> cache_;
    std::vector<std::vector<uint32_t>> lru_;

    uint32_t compute_sets() const noexcept;
    LocationInfo compute_location_info(uintptr_t addr) const noexcept;
    void update_lru(uint32_t way, uint32_t row, bool is_valid);
    void access(uintptr_t addr, bool write);
};
