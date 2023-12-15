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
    Cache pc = Cache(128, 2, 1);

    //128 / (1 * 2) = 64
    REQUIRE(pc.sets() == 64);

    //You were right with this test Kostas
    REQUIRE_THROWS(Cache(40, 1, 2));
}

TEST_CASE("Valid args", "cache") {
    //Cache not big enough
    REQUIRE_THROWS(Cache(128, 16, 16));

    //Invalid number of rows
    REQUIRE_THROWS(Cache(22, 2, 2));
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
    Cache pc = Cache(1024, 2, 8);

    //We should have log2(64)=6 bits for set index, 3 for offset

    //Tag=1, index = 32, off = 0
    uint32_t first_access = 3 << 8;

    REQUIRE(pc.misses() == 0);
    REQUIRE(pc.hits() == 0);

    pc.access(first_access);

    // First access should be a miss
    REQUIRE(pc.misses() == 1);

    pc.access(first_access);

    // Second access to same location should be a hit (loaded from physical memory)
    REQUIRE(pc.misses() == 1);
    REQUIRE(pc.hits() == 1);
}

TEST_CASE("Offsets work", "cache") {
    Cache pc = Cache(64, 2, 16);

    //Only block offset changes
    uint32_t first_access = (~0) << 4;
    uint32_t second_access = ~0;

    pc.access(first_access);

    pc.access(second_access);

    REQUIRE(pc.misses() == 1);
    REQUIRE(pc.hits() == 1);
}

//A cache with multiple ways should be able to store two blocks simultaneously
TEST_CASE("Assoc works", "cache") {
    Cache pc = Cache(128, 2, 16);

    //4 sets

    //Same index, different tags here
    uint32_t first_access = 3 << 4; //tag = 0
    uint32_t second_access = 7 << 4;// tag = 1

    pc.access(first_access);

    REQUIRE(pc.misses() == 1);

    pc.access(second_access);

    REQUIRE(pc.misses() == 2);
    REQUIRE(pc.hits() == 0);

    pc.access(first_access);
    pc.access(second_access);

    REQUIRE(pc.misses() == 2);
    REQUIRE(pc.hits() == 2);
}

//A cache with multiple ways should be able to store two blocks simultaneously
TEST_CASE("LRU policy works", "cache") {
    Cache pc = Cache(64, 1, 16);

    //4 sets

    //Same index, different tags here
    uint32_t first_access = 3 << 4; //tag = 0
    uint32_t second_access = 7 << 4;// tag = 1

    REQUIRE(pc.misses() == 0);

    pc.access(first_access);

    REQUIRE(pc.misses() == 1);

    pc.access(second_access);

    REQUIRE(pc.misses() == 2);

    pc.access(first_access);

    REQUIRE(pc.misses() == 3);

    pc.access(second_access);

    REQUIRE(pc.misses() == 4);

    REQUIRE(pc.hits() == 0);

    Cache pc2 = Cache(128, 2, 16);

    //4 sets

    pc2.access(first_access);
    pc2.access(second_access);
    pc2.access(first_access);
    pc2.access(second_access);

    REQUIRE(pc2.misses() == 2);
    REQUIRE(pc2.hits() == 2);

    uint32_t third_access = 15 << 4;

    pc2.access(third_access);

    REQUIRE(pc2.misses() == 3);

    pc2.access(second_access);

    REQUIRE(pc2.hits() == 3);
    REQUIRE(pc2.misses() == 3);

    pc2.access(first_access);
    pc2.access(third_access);

    REQUIRE(pc2.misses() == 5);
}

TEST_CASE("Way partitioning valid input", "Way partitioning") {
    vector<uint32_t> partition{1, 2, 1};

    //Not big enough cache
    REQUIRE_THROWS(WayPartitioning(4, 2, partition));
}

TEST_CASE("Way partitioning", "Way partitioning") {
    //Three clients, unequal partitions. This is the situation depicted on the stage 2 report (modulo the clients)
    vector<uint32_t> partition{1, 2, 1};

    WayPartitioning wp = WayPartitioning(256, 16, partition);

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

    wp.access(0, addr1);
    wp.access(0, addr1);

    REQUIRE(wp.misses(0) == 1);
    REQUIRE(wp.hits(0) == 1);

    //This operations should check, LRU replacement and associativity for each client
    wp.access(1, addr1);//Miss

    //make sure interleaving does not break anything
    wp.access(0, addr2);//Miss

    wp.access(2, addr1);//Miss
    wp.access(2, addr5);//Miss

    wp.access(1, addr2);//Miss, Set full
    wp.access(1, addr1);//Hit
    wp.access(1, addr5);//Miss, Eviction of 2

    wp.access(2, addr5);//Hit

    wp.access(1, addr2);//Miss, Eviction of 1
    wp.access(1, addr5);//Hit

    REQUIRE(wp.misses(0) == 2);
    REQUIRE(wp.hits(0) == 1);

    REQUIRE(wp.misses(2) == 2);
    REQUIRE(wp.hits(2) == 1);

    REQUIRE(wp.misses(1) == 4);
    REQUIRE(wp.hits(1) == 2);

    //Check that offsets are not broken
    wp.access(2, addr3);
    wp.access(2, addr4);

    REQUIRE(wp.misses(2) == 3);
    REQUIRE(wp.hits(2) == 2);

    //Check that other rows are still working
    wp.access(0, addr6);//Miss
    wp.access(0, addr7);//Miss
    wp.access(0, addr4);//Miss
    wp.access(0, addr2);//Hit
    wp.access(0, addr4);//Hit

    REQUIRE(wp.misses(0) == 5);
    REQUIRE(wp.hits(0) == 3);
}

// TODO(Kostas, Luis): Review this.
//TEST_CASE("Intra node valid args", "Intra node partitioning") {
//    vector<fixed_bits_t> aux_table2{
//            fixed_bits_t{bitset<32>{0x0}, 2},
//            fixed_bits_t{bitset<32>{0x3}, 2},
//            fixed_bits_t{bitset<32>{0x3}, 2}
//    };
//
//    //Not valid distribution of aux_table
//    REQUIRE_THROWS(IntraNodePartitioning(3, 256, 4, 16, aux_table2));
//
//    vector<fixed_bits_t> aux_table3{
//            fixed_bits_t{bitset<32>{0x0}, 0},
//            fixed_bits_t{bitset<32>{0x2}, 1},
//            fixed_bits_t{bitset<32>{0x3}, 2}
//    };
//
//    //Not valid distribution of aux_table
//    REQUIRE_THROWS(IntraNodePartitioning(3, 256, 4, 16, aux_table3));
//
//    vector<fixed_bits_t> aux_table4{
//            fixed_bits_t{bitset<32>{0x0}, 1},
//            fixed_bits_t{bitset<32>{0x2}, 1},
//            fixed_bits_t{bitset<32>{0x3}, 1}
//    };
//
//    //Not valid distribution of aux_table
//    REQUIRE_THROWS(IntraNodePartitioning(3, 256, 4, 16, aux_table4));
//
//     vector<fixed_bits_t> aux_table5{
//             fixed_bits_t{bitset<32>{0x0}, 1},
//             fixed_bits_t{bitset<32>{0x1}, 1}
//     };
//
//     //More clients than sets, set distribution impossible
//     REQUIRE_THROWS(IntraNodePartitioning(2, 64, 4, 16, aux_table5));
// }

TEST_CASE("Intra node partitioning", "Intra node partitioning") {
    //This is the situation that appears on the stage 2 report, same as above for way partitioning
    vector<fixed_bits_t> aux_table{
            fixed_bits_t{bitset<32>{0x0}, 1},
            fixed_bits_t{bitset<32>{0x2}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2},
    };//Three clients, unequal partitions

    IntraNodePartitioning inp = IntraNodePartitioning(256, 4, 16, aux_table);

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
    inp.access(1, addr1);
    inp.access(1, addr2);
    inp.access(1, addr3);
    inp.access(1, addr6);//Note that since fully assoc, 6 should go to the same (unique) set

    inp.access(2, addr1);
    inp.access(2, addr2);
    inp.access(2, addr3);
    inp.access(2, addr6);

    //All hits
    inp.access(1, addr1);
    inp.access(1, addr2);
    inp.access(1, addr3);
    inp.access(1, addr6);

    inp.access(2, addr1);
    inp.access(2, addr2);
    inp.access(2, addr3);
    inp.access(2, addr6);

    REQUIRE(inp.misses(1) == 4);
    REQUIRE(inp.hits(1) == 4);

    REQUIRE(inp.misses(2) == 4);
    REQUIRE(inp.hits(2) == 4);

    //LRU policy should be working still
    //All misses
    inp.access(1, addr5);
    inp.access(1, addr1);
    inp.access(1, addr2);
    inp.access(1, addr3);
    inp.access(1, addr6);

    inp.access(1, addr2);//Hit

    REQUIRE(inp.misses(1) == 9);
    REQUIRE(inp.hits(1) == 5);

    //Client 0 should have two sets of assoc 4
    //All misses
    inp.access(0, addr1);
    inp.access(0, addr2);
    inp.access(0, addr3);
    inp.access(0, addr4);
    inp.access(0, addr6);//Different row, still miss

    REQUIRE(inp.misses(0) == 5);
    REQUIRE(inp.hits(0) == 0);

    inp.access(0, addr7);//Hit (same block as 6)

    REQUIRE(inp.hits(0) == 1);

    //Check that interleaving with another client does not affect this one
    //All hits
    inp.access(2, addr1);
    inp.access(2, addr2);
    inp.access(2, addr3);
    inp.access(2, addr6);

    REQUIRE(inp.hits(2) == 8);

    //All hits
    inp.access(0, addr1);
    inp.access(0, addr2);
    inp.access(0, addr3);
    inp.access(0, addr4);

    REQUIRE(inp.hits(0) == 5);
    REQUIRE(inp.misses(0) == 5);

    inp.access(0, addr5);//Miss, 1 evicted
    inp.access(0, addr2);//Hits

    REQUIRE(inp.hits(0) == 6);
    REQUIRE(inp.misses(0) == 6);
}

TEST_CASE("Intra node partitioning second case", "Intra node partitioning") {
    //This is the situation that appears on the stage 2 report, same as above for way partitioning
    vector<fixed_bits_t> aux_table{
            fixed_bits_t{bitset<32>{0x0}, 1},
            fixed_bits_t{bitset<32>{0x2}, 2},
            fixed_bits_t{bitset<32>{0x3}, 2},
    };//Three clients, unequal partitions

    IntraNodePartitioning inp = IntraNodePartitioning(256, 2, 16, aux_table);

    //8 sets divided into 4, 2 and 2

    for (int i = 0; i < 3; ++i) {
        REQUIRE(inp.misses(i) == 0);
        REQUIRE(inp.hits(i) == 0);
    }

    //In this case, assuming we have at most 4 clients (3 bits used to partition the cache), we still have
    //one remaining bit for set indexing (we need 3 in total)
    //The tag is of size 64 - 4 (offset) - (3 - 2) (set indexing, set index max - log2(max_clients))

    //index 0, different tags for client 0
    uint32_t addr0 = 0;
    uint32_t addr4 = 7 << 7;

    //2 and 3 have the same index 3 for client 0
    uint32_t addr1 = 15 << 4;
    uint32_t addr2 = 3 << 4;
    uint32_t addr3 = 7 << 4;

    //Index 2, tag 0 for client 0
    uint32_t addr5 = 2 << 4;

    //Index 1, tag 0 for client 0
    uint32_t addr6 = 1 << 4;

    //Misses
    inp.access(0, addr0);
    inp.access(0, addr1);
    inp.access(0, addr2);
    inp.access(0, addr3);//Evicts addr1, but not 0

    REQUIRE(inp.misses(0) == 4);
    REQUIRE(inp.hits(0) == 0);

    inp.access(0, addr0);//Hit
    inp.access(0, addr2);//Hit

    REQUIRE(inp.misses(0) == 4);
    REQUIRE(inp.hits(0) == 2);

    inp.access(0, addr1);//Miss, evicts address 3

    REQUIRE(inp.misses(0) == 5);

    //Fill the rest of the cache sets with at least one value
    inp.access(0, addr4);//Miss
    inp.access(0, addr5);//Miss
    inp.access(0, addr6);//Miss

    REQUIRE(inp.misses(0) == 8);

    //All hits
    inp.access(0, addr0);
    inp.access(0, addr1);
    inp.access(0, addr2);
    //Addr 3 has been evicted
    inp.access(0, addr4);
    inp.access(0, addr5);
    inp.access(0, addr6);

    REQUIRE(inp.misses(0) == 8);
    REQUIRE(inp.hits(0) == 8);
}

TEST_CASE("Inter node partitioning input", "Inter node partitioning") {
    vector<uint32_t> part = {2, 1, 1};

    //Invalid cache size (rows cannot be properly divided)
    REQUIRE_THROWS(InterNodePartitioning(127, 2, 2, part));

    InterNodePartitioning inp = InterNodePartitioning(128, 2, 16, part);

    REQUIRE(inp.memory_nodes(0)[0].sets() == 4);
    REQUIRE(inp.memory_nodes(0)[0].sets() == 4);

    REQUIRE(inp.memory_nodes(1)[0].sets() == 4);

    REQUIRE(inp.memory_nodes(2)[0].sets() == 4);
}

TEST_CASE("Inter node partitioning", "Inter node partitioning") {
    //Example taken from figure 3 of stage 2 report
    vector<uint32_t> part = {2, 1, 1};

    //Each slice has a single set of associativity 2. No need for more complex slices since they are alaccessy checked
    // in previous tests! We are only interested in how accesses are divided between nodes here.
    InterNodePartitioning inp = InterNodePartitioning(32, 2, 16, part);

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
    inp.access(0, addr1);
    inp.access(0, addr2);
    //Node0
    inp.access(0, addr5);
    inp.access(0, addr6);

    //Hits
    inp.access(0, addr1);
    inp.access(0, addr2);

    inp.access(0, addr5);
    inp.access(0, addr6);

    REQUIRE(inp.misses(0) == 4);
    REQUIRE(inp.hits(0) == 4);

    //Misses and evicts addr1 on node 1
    inp.access(0, addr4);
    inp.access(0, addr2);//Hit
    inp.access(0, addr1);//Miss

    REQUIRE(inp.misses(0) == 6);
    REQUIRE(inp.hits(0) == 5);

    REQUIRE(inp.memory_nodes(0)[0].hits() == 2);
    REQUIRE(inp.memory_nodes(0)[0].misses() == 2);

    REQUIRE(inp.memory_nodes(0)[1].hits() == 3);
    REQUIRE(inp.memory_nodes(0)[1].misses() == 4);

    //Misses
    inp.access(1, addr1);
    inp.access(2, addr2);

    //Hits
    inp.access(1, addr1);
    inp.access(2, addr2);

    REQUIRE(inp.memory_nodes(1)[0].misses() == 1);
    REQUIRE(inp.memory_nodes(1)[0].hits() == 1);

    REQUIRE(inp.memory_nodes(2)[0].misses() == 1);
    REQUIRE(inp.memory_nodes(2)[0].hits() == 1);
}

TEST_CASE("Cluster partitioning normal way input", "Cluster partitioning normal") {
    //Too many ways, they do not fit in the cache
    vector<uint32_t> n_ways = {2, 1, 1, 1, 4};
    REQUIRE_THROWS(ClusterWayPartitioning(4, 128, 16, n_ways));

    //Cluster number not a power of two
    n_ways = {2, 1};
    REQUIRE_THROWS(ClusterWayPartitioning(3, 128, 16, n_ways));
}

TEST_CASE("Cluster partitioning normal way", "Cluster partitioning normal") {
    //Note that there is an unused way
    // TODO(kostas-to-luis): Check me!
    vector<uint32_t> n_ways = {2, 1, 1};
    ClusterWayPartitioning cwp = ClusterWayPartitioning(4, 128, 16, n_ways);

    //We have two clients with a different distribution of cores among clusters
    //Each cluster has two sets with four ways each

    //Slice 0, index 0
    uint32_t addr0 = 0;
    uint32_t addr1 = 0b1000 << 4;

    //Slice 0, index 1, different tags except for 4
    uint32_t addr2 = 0b1100 << 4;
    uint32_t addr3 = 0b0100 << 4;
    uint32_t addr4 = 0b01001111;//Same as 3, different offset
    uint32_t addr5 = 0b11100 << 4;

    //Slice 1, index 0
    uint32_t addr6 = 0b001 << 4;

    //Slice 3, index0
    uint32_t addr7 = 0b011 << 4;
    uint32_t addr8 = 0b1011 << 4;
    uint32_t addr9 = 0b11011 << 4;
    uint32_t addr10 = 0b111011 << 4;
    uint32_t addr11 = 0b1111011 << 4;

    //Slice 2, index 1
    uint32_t addr12 = 0b110 << 4;
    uint32_t addr13 = 0b1110 << 4;

    //Misses, let's fill the caches with some values
    cwp.access(0, addr0);
    cwp.access(0, addr1);
    cwp.access(0, addr2);
    cwp.access(0, addr3);

    cwp.access(1, addr0);

    cwp.access(1, addr6);

    cwp.access(0, addr7);
    cwp.access(0, addr8);

    cwp.access(0, addr12);
    cwp.access(0, addr13);

    REQUIRE(cwp.misses(0) == 8);
    REQUIRE(cwp.misses(1) == 2);
    REQUIRE(cwp.hits(0) == 0);
    REQUIRE(cwp.hits(1) == 0);

    cwp.access(0, addr4); //Hit
    cwp.access(0, addr5); //Miss and eviction of 2
    cwp.access(0, addr10);//Miss and eviction of 7
    cwp.access(0, addr11);//Miss and eviction of 8
    cwp.access(1, addr1); //Miss and eviction of 0

    REQUIRE(cwp.misses(0) == 11);
    REQUIRE(cwp.hits(0) == 1);

    REQUIRE(cwp.misses(1) == 3);

    //All hits, check whole cache state
    cwp.access(0, addr0);
    cwp.access(0, addr1);
    cwp.access(0, addr3);
    cwp.access(0, addr5);

    cwp.access(0, addr12);
    cwp.access(0, addr13);

    cwp.access(0, addr10);
    cwp.access(0, addr11);

    cwp.access(1, addr6);
    cwp.access(1, addr1);

    REQUIRE(cwp.misses(0) == 11);
    REQUIRE(cwp.hits(0) == 9);

    REQUIRE(cwp.misses(1) == 3);
    REQUIRE(cwp.hits(1) == 2);

    // TODO(kostas-to-luis): Check me!
    REQUIRE(cwp.clusters()[0].misses(0) == 5);
    REQUIRE(cwp.clusters()[1].misses(0) == 0);
    REQUIRE(cwp.clusters()[2].misses(0) == 2);
    REQUIRE(cwp.clusters()[3].misses(0) == 4);

    REQUIRE(cwp.clusters()[0].misses(1) == 2);

    // TODO(kostas-to-luis): Check me!
    REQUIRE(cwp.clusters()[0].hits(0) == 5);
    REQUIRE(cwp.clusters()[1].hits(0) == 0);
    REQUIRE(cwp.clusters()[2].hits(0) == 2);
    REQUIRE(cwp.clusters()[3].hits(0) == 2);
}

TEST_CASE("Inter-intra node partitioning input", "Inter-intra node partitioning") {
    //Non divisible number of rows
    vector<vector<uint32_t>> cache_sizes = {{60, 64, 0}, {64, 32, 32}, {0, 0, 128}, {32, 32, 32}};
    vector<inter_intra_aux_table_t> aux_table = {
            {5, {{0, 1}, {1, 3}, {3, 4}}},
            {4, {{0, 1}, {1, 2}, {3, 3}}},
            {6, {{1, 0}, {2, 4}, {3, 5}}}
    };
    REQUIRE_THROWS(InterIntraNodePartitioning(2, 16,cache_sizes,aux_table,4));

    //Invalid number of clients
    cache_sizes = {{64, 64, 0, 0}, {64, 32, 32}, {0, 0, 128}, {32, 32, 32}};
    REQUIRE_THROWS(InterIntraNodePartitioning(2, 16,cache_sizes,aux_table,4));

    //Invalid number of clients between cache sizes and aux table
    cache_sizes = {{64, 64, 0}, {64, 32, 32}, {0, 0, 128}, {32, 32, 32}};
    aux_table = {
            {5, {{0, 1}, {1, 3}, {3, 4}}},
            {4, {{0, 1}, {1, 2}, {3, 3}}},
            {6, {{1, 0}, {2, 4}, {3, 5}}},
            {6, {{1, 0}, {2, 4}, {3, 5}}}
    };
    REQUIRE_THROWS(InterIntraNodePartitioning(2, 16,cache_sizes,aux_table,4));
}

uint32_t createAddressIntraInterFirstCluster(uint32_t after_tag, uint32_t slice, uint32_t index, uint32_t offset) {
    //Get the maximal values and no more (avoid mistakes)
    slice = slice & 0b1111;
    index = index & 0b1;
    offset = offset & 0b1111;
    return (after_tag << 9) | (slice << 5) | (index << 4) | offset;
}

uint32_t createAddressIntraInterSecondCluster(uint32_t after_tag, uint32_t slice, uint32_t index, uint32_t offset) {
    //Get the maximal values and no more (avoid mistakes)
    slice = slice & 0b1111;
    index = index & 0b11;
    offset = offset & 0b1111;
    return (after_tag << 9) | (slice << 5) | (index << 4) | offset;
}

uint32_t createAddressIntraInterThirdCluster(uint32_t after_tag, uint32_t slice, uint32_t index, uint32_t offset) {
    //Get the maximal values and no more (avoid mistakes)
    slice = slice & 0b1111;
    offset = offset & 0b1111;
    return (after_tag << 8) | (slice << 4) | offset;
}

TEST_CASE("Inter-intra node partitioning", "Inter-intra node partitioning") {
    //We have three clients with unequal distributions. Note that the last cluster has an unused row!
    vector<vector<uint32_t>> cache_sizes = {{64, 64, 0}, {64, 32, 32}, {0, 0, 128}, {32, 32, 32}};
    vector<inter_intra_aux_table_t> aux_table = {
            {5, {{0, 1}, {1, 3}, {3, 4}}},
            {4, {{0, 1}, {1, 2}, {3, 3}}},
            {6, {{1, 0}, {2, 4}, {3, 5}}}};
    auto inp = InterIntraNodePartitioning(2, 16,
                               cache_sizes,
                               aux_table,
                               4);

    //We have 4 clusters, with caches with 4 sets each and assoc 2.

    uint32_t addr0 = createAddressIntraInterFirstCluster(0, 0, 0, 0);
    uint32_t addr1 = createAddressIntraInterFirstCluster(1, 0, 0, 0);
    uint32_t addr2 = createAddressIntraInterFirstCluster(0, 0, 1, 0);
    uint32_t addr3 = createAddressIntraInterFirstCluster(1, 0, 1, 0);
    uint32_t addr4 = createAddressIntraInterFirstCluster(2, 0, 1, 0);
    uint32_t addr5 = createAddressIntraInterFirstCluster(1, 0, 1, 4);

    uint32_t addr6 = createAddressIntraInterFirstCluster(0, 6, 0, 0);
    uint32_t addr7 = createAddressIntraInterFirstCluster(1, 5, 0, 0);

    uint32_t addr8 = createAddressIntraInterSecondCluster(0, 2, 3, 0);
    uint32_t addr9 = createAddressIntraInterSecondCluster(1, 3, 3, 0);
    uint32_t addr10 = createAddressIntraInterSecondCluster(2, 8, 3, 0);
    uint32_t addr11 = createAddressIntraInterSecondCluster(3, 0, 3, 4);

    uint32_t addr12 = createAddressIntraInterThirdCluster(0, 4, 0, 0);
    uint32_t addr13 = createAddressIntraInterThirdCluster(1, 9, 0, 0);
    uint32_t addr14 = createAddressIntraInterThirdCluster(2, 9, 0, 0);

    uint32_t addr15 = createAddressIntraInterThirdCluster(0, 3, 0, 0);
    uint32_t addr16 = createAddressIntraInterThirdCluster(1, 7, 0, 0);
    uint32_t addr17 = createAddressIntraInterThirdCluster(2, 7, 0, 0);

    uint32_t addr18 = createAddressIntraInterThirdCluster(0, 5, 0, 0);
    uint32_t addr19 = createAddressIntraInterThirdCluster(1, 11, 0, 0);
    uint32_t addr20 = createAddressIntraInterThirdCluster(2, 17, 0, 0);

    uint32_t addr21 = createAddressIntraInterSecondCluster(0, 2, 1, 0);
    uint32_t addr22 = createAddressIntraInterSecondCluster(1, 3, 1, 0);

    //All misses, let's fill the caches
    inp.access(0, addr0);
    inp.access(0, addr1);
    inp.access(0, addr2);
    inp.access(0, addr3);

    inp.access(1, addr0);
    inp.access(1, addr1);
    inp.access(1, addr2);
    inp.access(1, addr3);

    inp.access(2, addr8);
    inp.access(2, addr9);

    inp.access(0, addr12);
    inp.access(1, addr15);
    inp.access(2, addr18);

    inp.access(0, addr13);
    inp.access(1, addr16);
    inp.access(2, addr19);

    REQUIRE(inp.misses(0) == 6);
    REQUIRE(inp.misses(1) == 6);
    REQUIRE(inp.misses(2) == 4);

    inp.access(0, addr5); //Hit

    inp.access(0, addr4); //Miss + evict 2
    inp.access(1, addr4); //Miss + evict 2

    inp.access(0, addr6); //Miss + evict 3
    inp.access(0, addr4); //Hit
    inp.access(0, addr7); //Miss + evict 6

    //All hits
    inp.access(0, addr4);
    inp.access(0, addr7);
    inp.access(1, addr4);
    inp.access(1, addr3);

    REQUIRE(inp.misses(0) == 9);
    REQUIRE(inp.misses(1) == 7);

    REQUIRE(inp.hits(0) == 4);
    REQUIRE(inp.hits(1) == 2);

    REQUIRE(inp.get_cache_slice(0, 0).misses() == 7);
    REQUIRE(inp.get_cache_slice(1, 0).misses() == 5);
    REQUIRE(inp.get_cache_slice(2, 2).misses() == 2);
    REQUIRE(inp.get_cache_slice(2, 3).misses() == 2);
    REQUIRE(inp.get_cache_slice(1, 3).misses() == 2);
    REQUIRE(inp.get_cache_slice(0, 3).misses() == 2);

    REQUIRE(inp.get_cache_slice(0, 0).hits() == 4);
    REQUIRE(inp.get_cache_slice(1, 0).hits() == 2);

    //Misses
    inp.access(2, addr21);
    inp.access(2, addr22);

    //Hit and evict and call again 8 and 9
    inp.access(2, addr8);
    inp.access(2, addr9);

    inp.access(2, addr10);
    inp.access(2, addr11);

    inp.access(2, addr8);
    inp.access(2, addr9);

    //Hits
    inp.access(2, addr21);
    inp.access(2, addr22);

    REQUIRE(inp.get_cache_slice(2, 2).misses() == 8);
    REQUIRE(inp.get_cache_slice(2, 2).hits() == 4);

    REQUIRE(inp.hits(2) == 4);
    REQUIRE(inp.misses(2) == 10);

    //Misses + Evictions
    inp.access(0, addr14);
    inp.access(1, addr17);
    inp.access(2, addr20);

    //Hits
    inp.access(0, addr13);
    inp.access(1, addr16);
    inp.access(2, addr19);

    REQUIRE(inp.get_cache_slice(0, 3).misses() == 3);
    REQUIRE(inp.get_cache_slice(1, 3).misses() == 3);
    REQUIRE(inp.get_cache_slice(2, 3).misses() == 3);

    REQUIRE(inp.get_cache_slice(0, 3).hits() == 1);
    REQUIRE(inp.get_cache_slice(1, 3).hits() == 1);
    REQUIRE(inp.get_cache_slice(2, 3).hits() == 1);

    REQUIRE(inp.hits(2) == 5);
    REQUIRE(inp.misses(2) == 11);
}
