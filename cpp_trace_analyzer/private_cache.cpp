#include <bitset>
#include <cmath>
#include <iostream>
#include "private_cache.h"

void PrivateCache::read(uintptr_t addr) {
    access(addr, false);
}

void PrivateCache::write(uintptr_t addr) {
    access(addr, false);
}


uint32_t PrivateCache::evict(uint32_t set_index) {
    uint32_t way = lru_.at(set_index).at(0);

    cache_[set_index][way].state = CacheLineState::INVALID;

    update_lru(way, set_index, false);

    return way;
}

uint64_t PrivateCache::cache_size() const noexcept {
    return cache_size_;
}

uint32_t PrivateCache::sets() const noexcept {
    return sets_;
}

uint32_t PrivateCache::associativity() const noexcept {
    return assoc_;
}

uint32_t PrivateCache::block_size() const noexcept {
    return block_size_;
}

uint32_t PrivateCache::tag_size() const noexcept {
    return tag_size_;
}

uint32_t PrivateCache::misses() const noexcept {
    return misses_;
}

uint32_t PrivateCache::hits() const noexcept {
    return hits_;
}

uint32_t PrivateCache::compute_sets() const noexcept {
    uint32_t cache_lines = cache_size_ / (uint64_t) block_size_;
    return cache_lines / assoc_;
}

static uint32_t mask(uint32_t bits) {
    int result = 0;
    for (size_t i = 0; i < bits; i++) {
        result = (result << 1) | 0x1;
    }
    return result;
}

PrivateCache::LocationInfo PrivateCache::compute_location_info(uintptr_t addr) const noexcept {
    auto block_n_bits = (uint32_t) std::log2(block_size());
    auto set_n_bits = (uint32_t) std::log2(sets());
    auto tag_n_bits = (uint32_t) std::log2(tag_size());

    return {
       .block_offset = static_cast<uint32_t>(addr & mask(block_n_bits)),
       .set_index = static_cast<uint32_t>((addr >> block_n_bits) & mask(set_n_bits)),
       .tag = static_cast<uint32_t>((addr >> (block_n_bits + set_n_bits)) & mask(tag_n_bits))
    };
}

void PrivateCache::update_lru(uint32_t way, uint32_t row, bool is_valid) {
    try {
        std::vector<uint32_t> &row_stats = lru_.at(row);
        uint32_t next_way = way;

        if (is_valid) {
            ssize_t i = associativity() - 1;
            do {
                std::swap(row_stats.at(i), next_way);
                i--;
            } while (next_way != way);
        } else {
            size_t i = 0;
            do {
                std::swap(row_stats.at(i), next_way);
                i++;
            } while (next_way != way);
        }
    } catch (std::out_of_range &e) {
        std::cerr << e.what() << std::endl;
    }
}

void PrivateCache::access(uintptr_t addr, bool write) {
    LocationInfo loc = compute_location_info(addr);

    try {
        auto &row = cache_[loc.set_index];
        int32_t way = -1;
        for (size_t i = 0; i < associativity(); i++) {
            if (row[i].tag == loc.tag && row[i].state == CacheLineState::VALID) {
                way = i;
                break;
            }
        }

        if (way == -1) {
            way = evict(loc.set_index);
        }

        auto& cache_line = cache_[loc.set_index][way];
        if (cache_line.state == CacheLineState::INVALID) {
            // TODO(kostas): Read from upper level.
            misses_++;
            cache_line.state = CacheLineState::VALID;
            cache_line.tag = loc.tag;
        } else {
            hits_++;
        }

        if (write) {
            cache_line.state = CacheLineState::DIRTY;
        }

        update_lru(way, loc.set_index, true);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }
}
