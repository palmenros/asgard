#include "cache.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

Cache::Cache(uint64_t cache_size, uint32_t assoc, uint32_t block_size, uint32_t tag_size)
        : cache_size_(cache_size), block_size_(block_size),
          tag_size_(tag_size), misses_(0), hits_(0) {
    sets_ = compute_sets(assoc);
    cache_.resize(sets_, CacheSet(assoc));
}

uint64_t Cache::cache_size() const noexcept {
    return cache_size_;
}

uint32_t Cache::sets() const noexcept {
    return sets_;
}

uint32_t Cache::block_size() const noexcept {
    return block_size_;
}

uint32_t Cache::tag_size() const noexcept {
    return tag_size_;
}

uint32_t Cache::misses() const noexcept {
    return misses_;
}

uint32_t Cache::hits() const noexcept {
    return hits_;
}

void Cache::update_hits() noexcept {
    hits_++;
}

void Cache::update_misses() noexcept {
    misses_++;
}

uint32_t Cache::compute_sets(uint32_t assoc) const noexcept {
    uint32_t cache_lines = cache_size_ / ((uint64_t) block_size_ + (uint64_t) tag_size_);
    return cache_lines / assoc;
}

CacheSet::CacheSet(uint32_t assoc) : assoc_(assoc), lru_stats_(assoc), cache_lines_(assoc) {
    for (size_t i = 0; i < assoc; i++) {
        lru_stats_[i] = i;
    }
}

uint32_t CacheSet::associativity() const noexcept {
    return assoc_;
}

uint32_t CacheSet::evict() {
    uint32_t way = lru_stats_.at(0);

    cache_lines_[way].state = CacheLineState::INVALID;

    update_lru(way, false);

    return way;
}

void CacheSet::update_lru(uint32_t way, bool is_valid) {
    /*
     * An LRU vector has `associativity` items, initially set as
     * [0, 1, ..., associativity - 1].
     * The first item is the one that will be evicted (least recently used).
     * Upon access, we swap the accessed way to the end of the vector
     * and also swap the other ways accordingly so the least recently used
     * way is on position 0.
     */
    try {
        uint32_t next_way = way;

        if (is_valid) {
            ssize_t i = associativity() - 1;
            do {
                std::swap(lru_stats_.at(i), next_way);
                i--;
            } while (next_way != way);
        } else {
            size_t i = 0;
            do {
                std::swap(lru_stats_.at(i), next_way);
                i++;
            } while (next_way != way);
        }
    } catch (std::out_of_range &e) {
        std::cerr << e.what() << std::endl;
    }
}

CacheSet::CacheLine& CacheSet::cache_line(uint32_t assoc) {
    return cache_lines_[assoc];
}

void Cache::read(uintptr_t addr) {
    access(compute_location_info(addr), false);
}

void Cache::write(uintptr_t addr) {
    access(compute_location_info(addr), false);
}

// Extracts location information from the address, using masking.
LocationInfo Cache::compute_location_info(uintptr_t addr) const noexcept {
    auto block_bits = (uint32_t) std::log2(block_size());
    auto set_bits = (uint32_t) std::log2(sets());
    auto tag_bits = (uint32_t) std::log2(tag_size());

    return {
        .set_index = static_cast<uint32_t>((addr >> block_bits) & mask(set_bits)),
        .tag = static_cast<uint32_t>((addr >> (block_bits + set_bits)) & mask(tag_bits))
    };
}

void Cache::access(const LocationInfo& loc, bool write) {
    try {
        assert(loc.set_index < sets());
        auto &set = cache_[loc.set_index];
        int32_t way = -1;
        for (size_t i = 0; i < set.associativity(); i++) {
            auto& cache_line = set.cache_line(i);
            if (cache_line.tag == loc.tag && cache_line.state == CacheLineState::VALID) {
                way = i;
                break;
            }
        }

        if (way == -1) {
            way = set.evict();
        }

        auto& cache_line = set.cache_line(way);
        if (cache_line.state == CacheLineState::INVALID) {
            update_misses();
            cache_line.state = CacheLineState::VALID;
            cache_line.tag = loc.tag;
        } else {
            update_hits();
        }

        if (write) {
            cache_line.state = CacheLineState::DIRTY;
        }

        set.update_lru(way, true);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}
