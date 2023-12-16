#include "cache.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

Cache::Cache(uint64_t cache_size, uint32_t sets, uint32_t assoc, uint32_t block_size)
    : cache_size_(cache_size), block_size_(block_size), misses_(0), hits_(0) {

    cache_.resize(sets, CacheSet(assoc));

    auto block_bits = (uint32_t) std::log2(block_size);
    auto set_bits = (uint32_t) std::log2(cache_.size());

    tag_bits_ = 64 - set_bits - block_bits;
}

Cache::Cache(uint64_t cache_size, uint32_t assoc, uint32_t block_size)
        : cache_size_(cache_size), block_size_(block_size), misses_(0), hits_(0) {

    if (!Cache::is_power_of_2(cache_size)) {
        throw std::invalid_argument("Cache size should be power of 2!");
    }

    cache_.resize(compute_sets(assoc), CacheSet(assoc));

    if (cache_.empty() || (block_size * assoc > cache_size)) {
        throw std::invalid_argument("Invalid cache size (not big enough)!");
    }

    auto block_bits = (uint32_t) std::log2(block_size);
    auto set_bits = (uint32_t) std::log2(cache_.size());

    tag_bits_ = ADDRESS_SIZE - set_bits - block_bits;
}

uint64_t Cache::cache_size() const noexcept {
    return cache_size_;
}

uint32_t Cache::sets() const noexcept {
    return cache_.size();
}

uint32_t Cache::block_size() const noexcept {
    return block_size_;
}

uint32_t Cache::tag_bits() const noexcept {
    return tag_bits_;
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

uint32_t Cache::compute_sets(uint32_t assoc) const {
    if (cache_size() % (block_size() * assoc) != 0) {
        throw std::invalid_argument("Block size * associativity should be a multiple of cache size!");
    }
    return cache_size() / ((uint64_t) block_size() * assoc);
}

CacheSet::CacheSet(uint32_t assoc) : assoc_(assoc), lru_stats_(assoc), cache_lines_(assoc) {
    for (size_t i = 0; i < assoc; i++) {
        lru_stats_[i] = i;
    }
}

bool Cache::access(uintptr_t addr) {
    return access(Cache::compute_location_info(addr, block_size(), sets(), tag_bits()), addr);
}

bool Cache::access(const LocationInfo& loc, uintptr_t addr) {
    bool hit = false;

    try {
        assert(loc.set_index < sets());
        if (loc.set_index >= sets()) {
            throw std::exception();
        }
        auto &set = cache_[loc.set_index];
        int32_t way = -1;
        for (int i = 0; i < set.associativity(); i++) {
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
            cache_line.addr = addr;
        } else {
            hit = true;
            update_hits();
        }

        set.update_lru(way, true);

    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
    }

    return hit;
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

bool Cache::exists(uintptr_t addr) {
    for (auto& cache_set: cache_) {
        for (size_t a = 0; a < cache_set.associativity(); a++) {
            if (cache_set.cache_line(a).addr == addr) {
                return true;
            }
        }
    }

    return false;
}

bool Cache::access(uint32_t client_id, uintptr_t addr) {
    return access(addr);
}

uint32_t Cache::misses(uint32_t client_id) const noexcept {
    return misses();
}
