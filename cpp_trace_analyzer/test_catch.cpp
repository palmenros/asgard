#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "cache.hpp"

TEST_CASE("Correct initialization", "private_cache") {
    Cache pc = Cache(64, 2, 8, 8);

    REQUIRE(pc.cache_size() == 64);
    REQUIRE(pc.block_size() == 8);
    REQUIRE(pc.tag_size() == 8);
}

TEST_CASE("Correct number of sets", "private_cache") {
    Cache pc = Cache(1024, 2, 8, 8);

    //1024 / ((8 + 8) * 2) = 64
    REQUIRE(pc.sets() == 32);

    Cache pc2 = Cache(320, 1, 16, 4);

    REQUIRE(pc2.sets() == 16);

    Cache pc3 = Cache(176, 2, 16, 6);

    REQUIRE(pc3.sets() == 4);
}

static uint32_t createMask(uint8_t i, uint8_t j) {
    uint32_t allOnes = ~0; // All 1s in the 32-bit unsigned integer
    uint32_t leftMask = allOnes << (j + 1); // Creating 1s in the left side (bit positions greater than j)
    uint32_t rightMask = (1 << i) - 1; // Creating 1s in the right side (bit positions less than i)

    return ~(leftMask | rightMask); // Combining both masks using bitwise OR
}

//Accessing an address should be a miss the first time, and a hit on the second one
// (after being fetched from main memory)
TEST_CASE("Basic accesses work", "private_cache") {
    Cache pc = Cache(1024, 2, 8, 8);

    //We should have log2(64)=6 bits for set index, 3 for offset and 8 for tag size for a total size of 17 bits addresses

    //Tag=1, index = 16, off = 0
    uint32_t first_access = 3 << 8;

    REQUIRE(pc.misses() == 0);
    REQUIRE(pc.hits() == 0);

    pc.read(first_access);

    // First access should be a miss
    REQUIRE(pc.misses() == 1);

    pc.read(first_access);

    // Second access to same location should be a hit (loaded from physical memory)
    REQUIRE(pc.misses() == 1);
    REQUIRE(pc.hits() == 1);
}

TEST_CASE("Offsets work", "private_cache") {
    Cache pc = Cache(176, 2, 16, 6);

    //Only block offset changes
    uint32_t first_access = (~0) << 4;
    uint32_t second_access = ~0;

    pc.read(first_access);

    pc.read(second_access);

    REQUIRE(pc.misses() == 1);
    REQUIRE(pc.hits() == 1);
}

//A cache with multiple ways should be able to store two blocks simultaneously
TEST_CASE("assoc works", "private_cache") {
    Cache pc = Cache(176, 2, 16, 6);

    //Same index, different tags here
    uint32_t first_access = 3 << 4; //tag = 0
    uint32_t second_access = 7 << 4; // tag = 1

    pc.read(first_access);

    REQUIRE(pc.misses() == 1);

    pc.read(second_access);

    REQUIRE(pc.misses() == 2);
    REQUIRE(pc.hits() == 0);

    pc.write(first_access);
    pc.write(second_access);

    REQUIRE(pc.misses() == 2);
    REQUIRE(pc.hits() == 2);
}

//A cache with multiple ways should be able to store two blocks simultaneously
TEST_CASE("LRU policy works", "private_cache") {
    Cache pc = Cache(88, 1, 16, 6);

    //Same index, different tags here
    uint32_t first_access = 3 << 4; //tag = 0
    uint32_t second_access = 7 << 4; // tag = 1

    REQUIRE(pc.misses() == 0);

    pc.read(first_access);

    REQUIRE(pc.misses() == 1);

    pc.read(second_access);

    REQUIRE(pc.misses() == 2);

    pc.read(first_access);

    REQUIRE(pc.misses() == 3);

    pc.read(second_access);

    REQUIRE(pc.misses() == 4);

    REQUIRE(pc.hits() == 0);

    Cache pc2 = Cache(176, 2, 16, 6);

    pc2.read(first_access);
    pc2.read(second_access);
    pc2.read(first_access);
    pc2.read(second_access);

    REQUIRE(pc2.misses() == 2);
    REQUIRE(pc2.hits() == 2);

    uint32_t third_access = 15 << 4;

    pc2.read(third_access);

    REQUIRE(pc2.misses() == 3);

    pc2.read(second_access);

    REQUIRE(pc2.hits() == 3);
    REQUIRE(pc2.misses() == 3);

    pc2.read(first_access);
    pc2.read(third_access);

    REQUIRE(pc2.misses() == 5);
}
