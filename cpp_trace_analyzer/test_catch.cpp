#define CATCH_CONFIG_MAIN

#include "cache.hpp"
#include "catch.hpp"
#include "llc_partitioning.hpp"
#include <vector>

using namespace std;

TEST_CASE("Correct initialization", "cache") {
    Cache pc = Cache(64, 2, 8);

    REQUIRE(pc.cache_size() == 64);
    REQUIRE(pc.block_size() == 8);
}

TEST_CASE("Correct number of sets", "cache") {
    Cache pc = Cache(1024, 2, 8);

    //1024 / (8 * 2) = 64
    REQUIRE(pc.sets() == 64);

    Cache pc2 = Cache(320, 1, 16);

    REQUIRE(pc2.sets() == 20);
}

TEST_CASE("Valid args", "cache") {
    //Cache not big enough
    REQUIRE_THROWS(Cache(1024, 16, 128));

    //Invalid number of rows
    REQUIRE_THROWS(Cache(176, 2, 16));
}

static uint32_t createMask(uint8_t i, uint8_t j) {
    uint32_t allOnes = ~0;                 // All 1s in the 32-bit unsigned integer
    uint32_t leftMask = allOnes << (j + 1);// Creating 1s in the left side (bit positions greater than j)
    uint32_t rightMask = (1 << i) - 1;     // Creating 1s in the right side (bit positions less than i)

    return ~(leftMask | rightMask);// Combining both masks using bitwise OR
}

//Accessing an address should be a miss the first time, and a hit on the second one
// (after being fetched from main memory)
TEST_CASE("Basic accesses work", "cache") {
    Cache pc = Cache(8192, 2, 8);

    //We should have log2(64)=6 bits for set index, 3 for offset

    //Tag=1, index = 32, off = 0
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

TEST_CASE("Offsets work", "cache") {
    Cache pc = Cache(512, 2, 128);

    //Only block offset changes
    uint32_t first_access = (~0) << 4;
    uint32_t second_access = ~0;

    pc.read(first_access);

    pc.read(second_access);

    REQUIRE(pc.misses() == 1);
    REQUIRE(pc.hits() == 1);
}

//A cache with multiple ways should be able to store two blocks simultaneously
TEST_CASE("Assoc works", "cache") {
    Cache pc = Cache(1024, 2, 128);

    //4 sets

    //Same index, different tags here
    uint32_t first_access = 3 << 4; //tag = 0
    uint32_t second_access = 7 << 4;// tag = 1

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
TEST_CASE("LRU policy works", "cache") {
    Cache pc = Cache(512, 1, 128);

    //4 sets

    //Same index, different tags here
    uint32_t first_access = 3 << 4; //tag = 0
    uint32_t second_access = 7 << 4;// tag = 1

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

    Cache pc2 = Cache(1024, 2, 128);

    //4 sets

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

TEST_CASE("Way partitioning valid input", "Way partitioning") {
    vector<uint32_t> partition{1, 2, 1};

    //Not big enough cache
    REQUIRE_THROWS(WayPartitioning(partition, 32, 16));
}

TEST_CASE("Way partitioning", "Way partitioning") {
    //Three clients, unequal partitions. This is the situation depicted on the stage 2 report (modulo the clients)
    vector<uint32_t> partition{1, 2, 1};

    WayPartitioning wp = WayPartitioning(partition, 2048, 128);

    //4 sets

    for (int i = 0; i < 3; ++i) {
        REQUIRE(wp.misses(i) == 0);
        REQUIRE(wp.hits(i) == 0);
    }

    //Addr 1 and 2, 5 share index 3, different tags
    uint32_t addr1 = 3 << 4;
    uint32_t addr2 = 7 << 4;
    uint32_t addr5 = 15 << 4;

    //Addr 3 and 4 share index 1, different offset
    uint32_t addr3 = 3 << 3;
    uint32_t addr4 = 7 << 2;

    //Index 0
    uint32_t addr6 = 0;
    //Index 2
    uint32_t addr7 = 1 << 5;

    wp.read(0, addr1);
    wp.write(0, addr1);

    REQUIRE(wp.misses(0) == 1);
    REQUIRE(wp.hits(0) == 1);

    //This operations should check, LRU replacement and associativity for each client
    wp.read(1, addr1);//Miss

    //make sure interleaving does not break anything
    wp.read(0, addr2);//Miss

    wp.read(2, addr1);//Miss
    wp.read(2, addr5);//Miss

    wp.read(1, addr2);//Miss, Set full
    wp.read(1, addr1);//Hit
    wp.read(1, addr5);//Miss, Eviction of 2

    wp.write(2, addr5);//Hit

    wp.read(1, addr2);//Miss, Eviction of 1
    wp.read(1, addr5);//Hit

    REQUIRE(wp.misses(0) == 2);
    REQUIRE(wp.hits(0) == 1);

    REQUIRE(wp.misses(2) == 2);
    REQUIRE(wp.hits(2) == 1);

    REQUIRE(wp.misses(1) == 4);
    REQUIRE(wp.hits(1) == 2);

    //Check that offsets are not broken
    wp.write(2, addr3);
    wp.write(2, addr4);

    REQUIRE(wp.misses(2) == 3);
    REQUIRE(wp.hits(2) == 2);

    //Check that other rows are still working
    wp.read(0, addr6);//Miss
    wp.read(0, addr7);//Miss
    wp.read(0, addr4);//Miss
    wp.read(0, addr2);//Hit
    wp.read(0, addr4);//Hit

    REQUIRE(wp.misses(0) == 5);
    REQUIRE(wp.hits(0) == 3);
}

TEST_CASE("Intra node valid args", "Intra node partitioning") {
    vector<fixed_bits_t> aux_table{
            fixed_bits_t{bitset<32>{0x0}, 1},
            fixed_bits_t{bitset<32>{0x2}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2},
    };

    //Not correct number of clients
    REQUIRE_THROWS(IntraNodePartitioning(3, 2048, 4, 128, aux_table));

    vector<fixed_bits_t> aux_table2{
            fixed_bits_t{bitset<32>{0x0}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2}
    };

    //Not valid distribution of aux_table
    REQUIRE_THROWS(IntraNodePartitioning(3, 2048, 4, 128, aux_table2));

    vector<fixed_bits_t> aux_table3{
            fixed_bits_t{bitset<32>{0x0}, 0},
            fixed_bits_t{bitset<32>{0x2}, 1},
            fixed_bits_t{bitset<32>{0x3}, 2}
    };

    //Not valid distribution of aux_table
    REQUIRE_THROWS(IntraNodePartitioning(3, 2048, 4, 128, aux_table3));

    vector<fixed_bits_t> aux_table4{
            fixed_bits_t{bitset<32>{0x0}, 1},
            fixed_bits_t{bitset<32>{0x2}, 1},
            fixed_bits_t{bitset<32>{0x3}, 1}
    };

    //Not valid distribution of aux_table
    REQUIRE_THROWS(IntraNodePartitioning(3, 2048, 4, 128, aux_table4));
}

TEST_CASE("Intra node partitioning", "Intra node partitioning") {
    //This is the situation that appears on the stage 2 report, same as above for way partitioning
    vector<fixed_bits_t> aux_table{
            fixed_bits_t{bitset<32>{0x0}, 1},
            fixed_bits_t{bitset<32>{0x2}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2},
    };//Three clients, unequal partitions

    IntraNodePartitioning inp = IntraNodePartitioning(3, 2048, 4, 128, aux_table);

    //4 sets

    for (int i = 0; i < 3; ++i) {
        REQUIRE(inp.misses(i) == 0);
        REQUIRE(inp.hits(i) == 0);
    }

    //Under these circumstances, client 1 and 2 only have a fully associative cache, no indexing at all.
    //Client 0 has two sets, so only one bit is used for the index. Note that the tag should be of size 7 then!

    //Addr 1 and 2, 3, 4 and 5 share index 1 for client 0, different tags. addr2 is important, if it fails, there is an error!
    uint32_t addr1 = 1 << 4;
    uint32_t addr2 = 3 << 4;
    uint32_t addr3 = 7 << 4;
    uint32_t addr4 = 15 << 4;
    uint32_t addr5 = 31 << 4;

    //Addr 6 and 7 share index 0 for client 0, different offset
    uint32_t addr6 = 15;
    uint32_t addr7 = 3;

    //Clients 1 and 2 should be fully associative (with assoc of 4)

    //All misses, filling cache
    inp.write(1, addr1);
    inp.write(1, addr2);
    inp.write(1, addr3);
    inp.write(1, addr6); //Note that since fully assoc, 6 should go to the same (unique) set

    inp.write(2, addr1);
    inp.write(2, addr2);
    inp.write(2, addr3);
    inp.write(2, addr6);

    //All hits
    inp.write(1, addr1);
    inp.write(1, addr2);
    inp.write(1, addr3);
    inp.write(1, addr6);

    inp.write(2, addr1);
    inp.write(2, addr2);
    inp.write(2, addr3);
    inp.write(2, addr6);

    REQUIRE(inp.misses(1) == 4);
    REQUIRE(inp.hits(1) == 4);

    REQUIRE(inp.misses(2) == 4);
    REQUIRE(inp.hits(2) == 4);

    //LRU policy should be working still
    //All misses
    inp.write(1, addr5);
    inp.write(1, addr1);
    inp.write(1, addr2);
    inp.write(1, addr3);
    inp.write(1, addr6);

    inp.write(1, addr2); //Hit

    REQUIRE(inp.misses(1) == 9);
    REQUIRE(inp.hits(1) == 5);

    //Client 0 should have two sets of assoc 4
    //All misses
    inp.write(0, addr1);
    inp.write(0, addr2);
    inp.write(0, addr3);
    inp.write(0, addr4);
    inp.write(0, addr6); //Different row, still miss

    REQUIRE(inp.misses(0) == 5);
    REQUIRE(inp.hits(0) == 0);

    inp.write(0, addr7); //Hit (same block as 6)

    REQUIRE(inp.hits(0) == 1);

    //Check that interleaving with another client does not affect this one
    //All hits
    inp.write(2, addr1);
    inp.write(2, addr2);
    inp.write(2, addr3);
    inp.write(2, addr6);

    REQUIRE(inp.hits(2) == 8);

    //All hits
    inp.write(0, addr1);
    inp.write(0, addr2);
    inp.write(0, addr3);
    inp.write(0, addr4);

    REQUIRE(inp.hits(0) == 5);
    REQUIRE(inp.misses(0) == 5);

    inp.write(0, addr5); //Miss, 1 evicted
    inp.write(0, addr2); //Hits

    REQUIRE(inp.hits(0) == 6);
    REQUIRE(inp.misses(0) == 6);
}

TEST_CASE("Inter node partitioning input", "Inter node partitioning") {
    vector<uint32_t> part = {2, 1, 1};

    //Not proper number of clients
    REQUIRE_THROWS(InterNodePartitioning(2, part, 1024, 2, 16));
    REQUIRE_THROWS(InterNodePartitioning(4, part, 1024, 2, 16));

    //Invalid cache size (rows cannot be properly divided)
    REQUIRE_THROWS(InterNodePartitioning(3, part, 1023, 2, 16));

    InterNodePartitioning inp = InterNodePartitioning(3, part, 1024, 2, 16);

    REQUIRE(inp.memory_nodes(0)[0].sets() == 4);
    REQUIRE(inp.memory_nodes(0)[0].sets() == 4);

    REQUIRE(inp.memory_nodes(1)[0].sets() == 4);

    REQUIRE(inp.memory_nodes(2)[0].sets() == 4);
}

TEST_CASE("Inter node partitioning", "Inter node partitioning") {
    //Example taken from figure 3 of stage 2 report
    vector<uint32_t> part = {2, 1, 1};

    //Each slice has a single set of associativity 2. No need for more complex slices since they are already checked
    // in previous tests! We are only interested in how accesses are divided between nodes here.
    InterNodePartitioning inp = InterNodePartitioning(3, part, 256, 2, 128);

    //For this test, since we do not know the page size, I am assuming that the node selection bits are taken
    // from just after the block offset bits

    //Node selection = 3 and tag = 7
    uint32_t addr1 = 7 << 4;
    //Node selection = 3 and tag = 15
    uint32_t addr2 = 15 << 4;
    //Node selection = 3 and tag = 31
    uint32_t addr3 = 31 << 4;
    //Node selection = 1 and tag = 1
    uint32_t addr4 = 1 << 4;
    //Node selection = 0 and tag = 4
    uint32_t addr5 = 1 << 6;
    //Node selection = 0 and tag = 12
    uint32_t addr6 = 3 << 6;

    for (int i = 0; i < 3; ++i) {
        REQUIRE(inp.misses(i) == 0);
        REQUIRE(inp.hits(i) == 0);
    }

    //Misses
    //Node3 -> 1 because of the modulo
    inp.read(0, addr1);
    inp.read(0, addr2);
    //Node0
    inp.read(0, addr5);
    inp.read(0, addr6);

    //Hits
    inp.read(0, addr1);
    inp.read(0, addr2);

    inp.read(0, addr5);
    inp.read(0, addr6);

    REQUIRE(inp.misses(0) == 4);
    REQUIRE(inp.hits(0) == 4);

    //Misses and evicts addr1 on node 1
    inp.read(0, addr4);
    inp.read(0, addr2); //Hit
    inp.read(0, addr1); //Miss

    REQUIRE(inp.misses(0) == 6);
    REQUIRE(inp.hits(0) == 5);

    REQUIRE(inp.memory_nodes(0)[0].hits() == 2);
    REQUIRE(inp.memory_nodes(0)[0].misses() == 2);

    REQUIRE(inp.memory_nodes(0)[1].hits() == 3);
    REQUIRE(inp.memory_nodes(0)[1].misses() == 4);

    //Misses
    inp.read(1, addr1);
    inp.read(2, addr2);

    //Hits
    inp.read(1, addr1);
    inp.read(2, addr2);

    REQUIRE(inp.memory_nodes(1)[0].misses() == 1);
    REQUIRE(inp.memory_nodes(1)[0].hits() == 1);

    REQUIRE(inp.memory_nodes(2)[0].misses() == 1);
    REQUIRE(inp.memory_nodes(2)[0].hits() == 1);
}
