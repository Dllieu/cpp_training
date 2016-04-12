//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma warning( push )
#pragma warning( disable : 4996 ) // warning raise when using unit_test with multi_array instanciation
#include <boost/test/unit_test.hpp>
#pragma warning( pop )
#include <boost/multi_array.hpp>

#include <iostream>
#include <array>
#include <random>
#include <unordered_map>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <atomic>

#include "generic/Typetraits.h"
#include "generic/TuplePrinter.h"
#include "containers/PolymorphicCollection.h"
#include "tools/Benchmark.h"

using namespace tools;

// Intel Core i5-4460 (4 cores) (give a general idea - most of the benchmark might give different result depending of the architecture (e.g. if running on a VM with appveyor))
//
// Translation Lookaside Buffer (TLB)
// CPU cache that memory management hardware uses to improve virtual address translation speed, it has a fixed number of slots that contain page table entries, which map virtual addresses to physical addresses.
// It is typically a content-addressable memory (CAM), in which the search key is the virtual address and the search result is a physical address.
// - 2MB pages mode
//   - L1  (32 entries), Miss Penalty = 16 cycles. Parallel miss: 20 cycles per access
// - 4KB page mode
//   - L1  (64 entries), 4-WAY
//                   * miss penalty : 7 cycles, parallel miss: 1 cycle per access
//   - L2 (512 entries), 4-WAY
//                   * miss penalty : 9 cycles, parallel miss: 21 cycle per access
//
//
// Instruction cache
// - L1  (32KB), 8-WAY, 64B / line
//
// Data Cache (8-WAY, 64B / line)
// - L1  (32KB):    1   ns /   4 cycles
//             * 1 per core (hyperthreading delivers two processing threads per physical core, shared by two HW thread)
//             * 4 cycles for simple access via pointer (p), 5 cycles for access with complex address calculation (p[n])
// - L2 (256KB):    3.1 ns /  12 cycles (per processor, shared by 2 HW threads)
// - L3   (6MB):    7.7 ns /  30 cycles (share among all the core (4 core -> 8 HW threads))
//             * 29.5 cycles for cores (1, 2)
//             * 30.5 cycles for cores (0, 3)
// - DRAM  (XX):   60   ns / 237 cycles
//
// cache size = cache line size * associativity * number of sets
//
// DRAM (dynamic ram) SRAM (static ram)
//
// Caches are small, assume 100MB program at runtime (code + data).
// - 8% fits in core-i79xx's L3 cache.
//   -> L3 cache shared by every running process (incl. OS).
// - 0.25% fits in each L2 cache.
// - 0.03% fits in each L1 cache.
//
// Check for more infos: http://www.7-cpu.com/

// Main memory to cache
// - Memory is transferred from the main memory into the caches in blocks which are smaller than the cache line size.Today 64 bits are transferred at once and the cache line size is (usually) 64 bytes (This means 8 or 16 transfers per cache line are needed)
// - The DRAM chips can transfer those 64-byte blocks in burst mode.This can fill the cache line without any further commands from the memory controller and the possibly associated delays. If the processor prefetches cache lines this is probably the best way to operate
// - The memory controller is free to request the words of the cache line in a different order.The processor can communicate which word the program is waiting on, the critical word, and the memory controller can request this word first.
//   Once the word arrives the program can continue while the rest of the cache line arrives and the cache is not yet in a consistent state.
//   This technique is called Critical Word First
//   Processors nowadays implement this technique but there are situations when that is not possible.If the processor prefetches data the critical word is not known.Should the processor request the cache line during the time
//   the prefetch operation is in flight it will have to wait until the critical word arrives without being able to influence the order.

// When does a cache line transfer have to happen from a processor to another? when one processor needs a cache line which is dirty in another processor's cache for reading or writing ("easily" implemented with MESI)
// MESI Protocol Transitions:
//   * Modified: The local processor has modified the cache line. This also implies it is the only copy in any cache.
//   * Exclusive: The cache line is not modified but known to not be loaded into any other processor’s cache.
//   * Shared: The cache line is not modified and might exist in another processor’s cache.
//   * Invalid: The cache line is invalid, i.e., unused
//
// About Modified transition:
//   - If a Modified cache line is read from or written to on the local processor, the instruction can use the current cache content and the state does not change.
//   - If a second processor wants to read from the cache line the first processor has to send the content of its cache to the second processor and then it can change the state to Shared.
//   - The data sent to the second processor is also received and processed by the memory controller which stores the content in memory.
//   - If this did not happen the cache line could not be marked as Shared.If the second processor wants to write to the cache line the first processor sends the cache line content and marks the cache line locally as invalid
//   - This is the infamous "Request For Ownership" (RFO) operation. Performing this operation in the last level cache, just like the I->M transition is comparatively expensive.
//   - For write-through caches we also have to add the time it takes to write the new cache line content to the next higher-level cache or the main memory, further increasing the cost

// About cache misses
//  A cache miss refers to a failed attempt to read or write a piece of data in the cache, which results in a main memory access with much longer latency.
//  There are three kinds of cache misses: instruction read miss, data read miss, and data write miss.
//  - Cache read misses from an instruction cache generally cause the largest delay, because the processor, or at least the thread of execution, has to wait (stall) until the instruction is fetched from main memory.
//  - Cache read misses from a data cache usually cause a smaller delay, because instructions not dependent on the cache read can be issued and continue execution until the data is returned from main memory,
//    and the dependent instructions can resume execution.
//  - Cache write misses to a data cache generally cause the shortest delay, because the write can be queued and there are few limitations on the execution of subsequent instructions; the processor can continue until the queue is full
//  - could possibly have cache miss on unlinked data (two static data that are put on the same cache line, N global on different translation unit could also be put in same cache line, same thing for different dynamic allocation, or stack for that matter)
//  - As data cache is 8-way associative, we can know in which block a cache line is depending of the address of the data (i.e. beginning_adress_cache_line(address) % 8)
//    theoretically we could only work with data that are contains in the same cache block which could result in horrible performance as we would only use 1/8 of the cache available

// About Instruction Cache friendly
// Any code that changes the flow of execution affects the Instruction Cache. This includes function calls and loops as well as dereferencing function pointers.
// - When a branch or jump instruction is executed, the processor has to spend extra time deciding if the code is already in the instruction cache or whether it needs to reload the instruction cache( from the destination of the branch ).
// - For example, some processors may have a large enough instruction cache to hold the execution code for small loops.Some processors don't have a large instruction cache and simple reload it. Reloading of the instruction cache takes time that could be spent executing instructions.
// What can help
// - Reduce "if" statements
// - Define small functions as inline or macros
//     -> There is an overhead associated with calling functions, such as storing the return location and reloading the instruction cache.
//     -> it's not as straightforward though, this could incurr code bloat, specially if the function inlined is unlikely called (in branch) and that the function is inlined in several place in the code (code duplication reduces effective cache size)
// - Unroll loops
// - Use table lookups, not "if" statements
// - Change data or data structures (For example, a program handling message packets could base its operations based on the packet IDs (think array of function pointers (cache miss?)). Functions would be optimized for packet processing)

// Classical big-O algorithmic complexity analysis proves insufficient to estimate program performance for modern computer architectures,
// current processors are equipped with several low-level components (hierarchical cache structures, pipelining, branch prediction)
// that greatly favor certain code and data layout patterns not taken into account by naive computation models.

// About Hyper-threading
// - Hyper-Threads are implemented by the CPU and are a special case since the individual threads cannot really run concurrently.They all share almost all the processing resources except for the register set
// - The real advantage is that the CPU can schedule another hyper-thread and take advantage of available resources such as arithmetic logic units (ALUs) when the currently running hyper-thread is delayed
// - In most cases this is a delay caused by memory accesses
// - If two threads are running on one hyper-threaded core the program is only more efficient than the single-threaded code if the combined runtime of both threads is lower than the runtime of the single-threaded code
// - Might only be achievable if single thread have a low cache hit rate (need to take into account the overhead for parallelizing the code)
// - Could be used as a mere thread to prefetch data into L2 or L1d for the "main" working thread (as they share the same cache) (imply proper affinity)

// Take advantage of
// - PGO (Profile-guided optimization) (i.e. -fprofile-generate in gcc), It works by placing extra code to count the number of times each codepath is taken. The profiling test must be representative to the production
//   When you compile a second time the compiler uses the knowledge gained about execution of your program that it could only guess at before (frequency statemants, branching, ...). There are a couple things PGO can work toward:
//     Deciding which functions should be inlined or not depending on how often they are called.
//     Deciding how to place hints about which branch of an "if" statement should be predicted on based on the percentage of calls going one way or the other.
//     Deciding how to optimize loops based on how many iterations get taken each time that loop is called.
//   It help the compiler to be less reliant on heuristics when making compilation decisions
//   Requires important representative use cases. Tends to be most helpful for large, non - loop - bound applications.
//     Hundreds to thousands of functions.
//     Most time spent in branches, calls / returns.
//   Designed for use after source code freeze., By default, source code changes invalidate instrumentation data. Resource - intensive during builds and instrumented runs (e.g. Instrumentation insertion / execution / analysis not cheap)
// - WPO (whole program optimization (O3 + unsafe unloop + omit frame ptr + specific target arch instructions + ...))
BOOST_AUTO_TEST_SUITE( CacheTestSuite )

namespace
{
    auto    generateShuffledList( int size )
    {
        std::uniform_int_distribution<> rnd( 0, size - 1 );
        std::mt19937 gen;

        std::list<int> result( size );
        std::generate( result.begin(), result.end(), [ &rnd, &gen ] { return rnd( gen ); } );
        result.sort();

        return result;
    }
}

// |oooooooooooooooooooo|-----------------------|
// | current cache line | prefetched cache line |
// Two aspects to watch out for
// - Locality
// - Prefetching
// hardware speculatively prefetches cache lines, thus can generally prefecth for you as long as the access is consistent (i.e. i++ or i += 4 or even i--), will prefetch (if applicable) line n - 1 or n + 1 depending of the access pattern
// std::vector / std::array excels at both
// std::list sequentially allocated nodes provide some sort of non-guaranteed locality
// shuffled nodes is the worst scenario
BOOST_AUTO_TEST_CASE( LinearTraversalBenchmark )
{
    auto test = [] ( auto n )
    {
        auto v = std::vector< int >( n );

        double vectorT, listT, shuffledListT;
        std::tie( vectorT, listT, shuffledListT ) = benchmark( n,
                                                               [ v = std::vector< int >( n ) ] { return std::accumulate( std::begin( v ), std::end( v ), 0 ); },
                                                               [ l = std::list< int >( n ) ] { return std::accumulate( std::begin( l ), std::end( l ), 0 ); },
                                                               [ sl = generateShuffledList( n ) ] { return std::accumulate( std::begin( sl ), std::end( sl ), 0 ); } );

        BOOST_CHECK( vectorT <= listT );
        if ( byteToAppropriateCacheSize< int >( n ) > CacheSize::L1 )
            BOOST_CHECK( listT < shuffledListT );
    };
    run_test< int >( "vector;list;shuffled_list;", test, 4'096, 100'000, 1'000'000 );
}

BOOST_AUTO_TEST_CASE( MatrixTraversalBenchmark )
{
    auto test = [] ( auto n )
    {
        // multiple array with contiguous data
        double rowT, colT;
        std::tie( rowT, colT ) = benchmark( n,
                                            [ m1 = boost::multi_array< int, 2 >( boost::extents[ n ][ n ] ), n ]
                                            {
                                                auto res = 0;
                                                for ( auto row = 0; row < n; ++row )
                                                    for ( auto col = 0; col < n; ++col )
                                                        res += m1[row][col];
                                                return res;
                                            },
                                            [ m2 = boost::multi_array< int, 2 >( boost::extents[ n ][ n ] ), n ]
                                            {
                                                auto res = 0;
                                                for ( auto col = 0; col < n; ++col )
                                                    for ( auto row = 0; row < n; ++row )
                                                        res += m2[ row ][ col ];
                                                return res;
                                            } );

        // In case all the node could be hold in L2 cache
        if ( byteToAppropriateCacheSize< int >( n * n ) > CacheSize::L2 )
            BOOST_CHECK( rowT < colT );
    };
    run_test< int >( "row;col;", test, 124, 512, 1'024 );
}

namespace
{
    auto    generateMap( size_t size )
    {
        std::map< int, int > result;
        for ( auto i = 0; i < size; ++i ) result[ i ] = 0;
        return result;
    }

    auto    generateUnorderedMap( size_t size )
    {
        std::unordered_map< int, int > result;
        result.reserve( size ); // seems to help to have better cache locality for some reason
        for ( auto i = 0; i < size; ++i ) result[ i ] = 0;

        return result;
    }
}

// Unordered maps can be implemented in a variety of ways, with implications for the memory usage.The fundamental expectation is that there'll be a contiguous array of key/value "buckets",
// but in real-world implementations the basic design tradeoffs may involve:
// 
// - two or more contiguous regions to reduce the performance cost when growing the container capacity, and separately
// - when there's a collision, an implementation may
//      (A) use an algorithm to select a sequence of alternative buckets or
//      (B) they may have each bucket be / point - to a resizable container of the key / value pairs.
//
//  Trying to make this latter choice more tangible : at its academic simplest, you can imagine :
//      (A) the hash table finding alternative "buckets" - as an array containing of key / value pairs, with empty / unused values scattered amongst the meaningful ones, akin to vector<optional<pair<key, value>>>.
//      (B) the hash table that instead uses containers is like a vector<list<pair<key, value>>> where every vector element is populated,
//          but getting from the vector elements to the lists involves extra pointers and discontiguous memory regions : this will be a little slower to deallocate as there are more distinct memory areas to delete.
//
// If the ratio of used to unused buckets is kept low, then there will be less collisions but more wasted memory.
// Another consideration : as the size of key / value pairs increase, the memory allocation overheads and pointers become less significant in comparison,
// so maps tend to use less memory than a hash map with low utilisation and values stored directly in the buckets.But, you can often create a hash map of key / pointers - to - value which mitigates that problem.
// So, there's the potential for a hash map to use less overall memory (particularly with small key/value types and a high used-to-unusued bucket ratio)
// and do less distinct allocations and deallocations as well as working better with caches, but it's far from guaranteed.
BOOST_AUTO_TEST_CASE( AssociativeTraversalIteratorBenchmark )
{
    auto op = [] ( auto r, const auto& p ) { return r + p.second; };
    auto test = [ &op ] ( auto n )
    {
        double unorderedMapT, mapT;
        std::tie( unorderedMapT, mapT ) = benchmark( n,
                                                     [ unorderedMap = generateUnorderedMap( n ), &op ] { return std::accumulate( std::begin( unorderedMap ), std::end( unorderedMap ), 0, op ); },
                                                     [ map = generateMap( n ), &op ] { return std::accumulate( std::begin( map ), std::end( map ), 0, op ); } );

        // unordered_map beat map for low N, at some point map is more cache friendly (in this case from 1M)
    };
    run_test< int >( "unordered_map;map;", test, 4'096, 100'000, 1'000'000, 10'000'000 );
}

BOOST_AUTO_TEST_CASE( AssociativeTraversalBenchmark )
{
    auto f = [] ( auto& m ) { auto n = 0; for ( auto i = 0; i < m.size(); ++i ) n += m.at( i ); return n; };
    // Cache is less a factor than complexity in this test
    auto test = [ &f ] ( auto n )
    {
        double unorderedMapT, mapT;
        std::tie( unorderedMapT, mapT ) = benchmark( n,
                                                     [ um = generateUnorderedMap( n ), &f ] { return f( um ); }, // hash + access(O(1))
                                                     [ m = generateMap( n ), &f ] { return f( m ); } ); // access(O(log n))

        BOOST_CHECK( unorderedMapT < mapT );
    };
    run_test< int >( "unordered_map;map;", test, 4'096, 100'000, 1'000'000 );
}

// Array-Of-Structure vs Structure-Of-Array
namespace
{
    struct Particle { int x, y, z, dx, dy, dz; };
    using AOSParticle = std::vector< Particle >;

    struct SOAParticle
    {
        SOAParticle( size_t n ) : x( n ), y( n ), z( n ), dx( n ), dy( n ), dz( n ) {}
        std::vector<int> x, y, z, dx, dy, dz;
    };
}

BOOST_AUTO_TEST_CASE( AOSvsSOABenchmark )
{
    auto test = [] ( auto n )
    {
        double aosT, soaT;
        std::tie( aosT, soaT ) = benchmark( n,
                                            // 64 / ( 6 / 3 ) / sizeof( int ) = 8 useful values per fetch average (6 / 3 :  only need x, y, z)
                                            [ aos = AOSParticle( n ), n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += aos[ i ].x + aos[ i ].y + aos[ i ].z; return res; },

                                            // 64 / sizeof( int ) = 16 useful values per fetch
                                            [ soa = SOAParticle( n ), n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += soa.x[ i ] + soa.y[ i ] + soa.z[ i ]; return res; } );

        BOOST_CHECK( aosT > soaT );
    };
    run_test< int >( "aos;soa;", test, 4'096, 16'384, 100'000, 1'000'000, 10'000'000 );
}

namespace
{
    struct CompactParticle { int x, y, z; };
    using AOSCompactParticle = std::vector< CompactParticle >;

    struct SOACompactParticle
    {
        SOACompactParticle( size_t n ) : x( n ), y( n ), z( n ) {}
        std::vector<int> x, y, z;
    };
}

BOOST_AUTO_TEST_CASE( CompactAOSvsSOABenchmark )
{
    auto test = [] ( auto n )
    {
        // Both fetch only useful values
        // SOA is faster (diminishingly as n grows), because CPU can prefetch x, y, z in parallel
        // (e.g. big picture: aos need to prefetch (stale) every N bytes, but soa only need to prefetch every N * 3 (the is more expensive, but less than the stale depending of the cache layer))

        double aosT, soaT;
        std::tie( aosT, soaT ) = benchmark( n,
                                            [ aos = AOSCompactParticle( n ), n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += aos[ i ].x + aos[ i ].y + aos[ i ].z; return res; },
                                            [ soa = SOACompactParticle( n ), n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += soa.x[ i ] + soa.y[ i ] + soa.z[ i ]; return res; } );

        // SOA is faster (diminishingly as n grows)
        if ( byteToAppropriateCacheSize< CompactParticle >( n ) < CacheSize::DRAM )
            BOOST_CHECK( aosT > soaT );
    };
    run_test< int >( "aos;soa;", test, 4'096, 16'384, 100'000, 1'000'000, 10'000'000, 20'000'000 );
}

BOOST_AUTO_TEST_CASE( CompactRandomAccessAOSvsSOABenchmark )
{
    auto test = [] ( auto n )
    {
        double aosT, soaT;
        std::tie( aosT, soaT ) = benchmark( n,
                                            [ aos = AOSCompactParticle( n ), n ]
                                            {
                                                auto res = 0; std::uniform_int_distribution<> rnd( 0, n - 1 ); std::mt19937 gen;
                                                for ( auto i = 0; i < n; ++i )
                                                {
                                                    auto idx = rnd( gen );
                                                    res += aos[ idx ].x + aos[ idx ].y + aos[ idx ].z;
                                                }
                                                return res;
                                            },
                                            [ soa = SOACompactParticle( n ), n ]
                                            {
                                                auto res = 0; std::uniform_int_distribution<> rnd( 0, n - 1 ); std::mt19937 gen;
                                                for ( auto i = 0; i < n; ++i )
                                                {
                                                    auto idx = rnd( gen );
                                                    res += soa.x[ idx ] + soa.y[ idx ] + soa.z[ idx ];
                                                }
                                                return res;
                                            } );

        // Similar results, but passed L2 cache, aos is more efficient (less staling due to the 1 prefetch instead of 3)
        if ( byteToAppropriateCacheSize< CompactParticle >( n ) > CacheSize::L2 )
            BOOST_CHECK( aosT < soaT );
    };
    run_test< int >( "aos;soa;", test, 512, 4'096, 16'384, 100'000, 1'000'000, 1'200'000, 1'800'000 );
}

namespace
{
    std::vector< int >  generate_vector( int size )
    {
        std::uniform_int_distribution<> rnd( 0, 256 );
        std::mt19937 gen;

        std::vector< int > result( size );
        std::generate( result.begin(), result.end(), [ &rnd, &gen ] { return rnd( gen ); } );
        return result;
    }
}

// About branch-prediction and pipelining
//
// Branch instructions represent 20% of dynamic instruction count of most programs
//
// - A branch predictor is a digital circuit that tries to guess which way a branch (e.g. an if-then-else structure) will go before this is known for sure.
//   The purpose of the branch predictor is to improve the flow in the instruction pipeline. 
//   Without branch prediction, the processor would have to wait until the conditional jump instruction has passed the execute stage before the next instruction can enter the fetch stage in the pipeline.
//   The branch predictor attempts to avoid this waste of time by trying to guess whether the conditional jump is most likely to be taken or not taken.
//   The branch that is guessed to be the most likely is then fetched and speculatively executed.
//   If it is later detected that the guess was wrong then the speculatively executed or partially executed instructions are discarded and the pipeline starts over with the correct branch, incurring a delay.
//   Analogy: When browsing a web page, we could pre-fetch image + other webpages that are linked in the current page,
//            We could reduce the overhead of prefetching everything by prefetching a smaller subset that we would speculate from the common usage pattern on that website,
//            or more advanced, by community use history (per collected usage stats), or per-user use history (per collected usage stats), if using stats, we also could do so either during runtime, or using PGO
//
// - The branch predictor keeps records of whether branches are taken or not taken. When it encounters a conditional jump that has been seen several times before then it can base the prediction on the history.
//   The branch predictor may, for example, recognize that the conditional jump is taken more often than not, or that it is taken every second time.
//   The processor may or may not branch, depending on a calculation that has not yet occurred. Various processors may stall, may attempt branch prediction,
//       and may be able to begin to execute two different program sequences (eager execution), both assuming the branch is and is not taken, discarding all work that pertains to the incorrect guess.
//   Programs written for a pipelined processor deliberately avoid branching to minimize possible loss of speed.
//       For example, the programmer can handle the usual case with sequential execution and branch only on detecting unusual cases.
//
// - Instruction pipelining is a technique that implements a form of parallelism called instruction-level parallelism within a single processor.
//   It therefore allows faster CPU throughput (the number of instructions that can be executed in a unit of time) than would otherwise be possible at a given clock rate.
//   The basic instruction cycle is broken up into a series called a pipeline. Rather than processing each instruction sequentially (finishing one instruction before starting the next),
//   each instruction is split up into a sequence of steps so different steps can be executed in parallel and instructions can be processed concurrently (starting one instruction before finishing the previous one).
// - Pipelining increases instruction throughput by performing multiple operations at the same time, but does not reduce instruction latency, which is the time to complete a single instruction from start to finish,
//   as it still must go through all steps. Indeed, it may increase latency due to additional overhead from breaking the computation into separate steps and worse, the pipeline may stall (or even need to be flushed),
//   further increasing the latency. Thus, pipelining increases throughput at the cost of latency
// 
// - Pipelining increase instruction execution throughput by N (latency remains the same)
//   Each pipeline stage is expected to complete in one clock cycle
//   /!\ The clock period should be long enough to let the slowest pipeline stage to complete, faster stages can only wait for the slowest one to complete (stall), called hazard
//     - i.e. if one stage finish in one cycle, and another on the same pipeline finish in 3 cycle, the pipeline is said to have been stalled for two clock cycles
//     - i.e. if we have two register instructions : r5 += 1; r6 = r5; second instruction is dependant of the first instructions : compiler might rearrange the code to generate machine code that avoid such hazards
//   If each instruction needs to be fetched from main memory, pipeline is almost useless
//   - Sandy bridge: 14 - 17 stages
//   - Ivy bridge:   14 - 19 stage
//
// - CPU STALL: The time taken to fetch one cache line from memory (read latency) matters because the CPU will run out of things to do while waiting for the cache line. When a CPU reaches this state, it is called a stall.
//   As CPUs become faster compared to main memory, stalls due to cache misses displace more potential computation; modern CPUs can execute hundreds of instructions in the time taken to fetch a single cache line from main memory.
//   Various techniques have been employed to keep the CPU busy during this time, including out - of - order execution in which the CPU( Pentium Pro and later Intel designs, for example )
//   attempts to execute independent instructions after the instruction that is waiting for the cache miss data.Another technology, used by many processors, is simultaneous multithreading( SMT ),
//   or-​in Intel's terminology—​hyper-threading (HT), which allows an alternate thread to use the CPU core while the first thread waits for required CPU resources to become available.
//
// - the execution of an instruction happens in stages. First an instruction is decoded, then the parameters are prepared, and finally it is executed.
//   Such a pipeline can be quite long (> 20 stages for Intel's Netburst architecture).
//   A long pipeline means that if the pipeline stalls (i.e., the instruction flow through it is interrupted) it takes a while to get up to speed again
//   Pipeline stalls happen, for instance, if the location of the next instruction cannot be correctly predicted or if it takes too long to load the next instruction (e.g., when it has to be read from memory).
//   As a result CPU designers spend a lot of time and chip real estate on branch prediction so that pipeline stalls happen as infrequently as possible
//
// - The time that is wasted in case of a branch misprediction is equal to the number of stages in the pipeline from the fetch stage to the execute stage.
//   Modern microprocessors tend to have quite long pipelines so that the misprediction delay is between 10 and 20 clock cycles.
//   As a result, making a pipeline longer increases the need for a more advanced branch predictor.
//   * In recent years the processors do not cache the raw byte sequence of the instructions inL1i but instead they cache the decoded instructions (can skip decode step of the pipeline)
//   * To achieve best performance of instruction cache: Generate code which is as small as possible (can be some exceptions) / Help the processor making good prefetching decisions (code layout / [un]likely / prefetching)
//
// - Flow:
//   - Waiting instructions
//   - Pipelining
//     1 - Fetch
//     2 - Decode
//     3 - Execute
//     4 - Write-back (writing the results of the instruction to processor registers or to memory)
//   - Completed instructions
BOOST_AUTO_TEST_CASE( BranchPredictionBenchmark )
{
    // could remove the branch (if x >= 128 ) with int t = (x - 128) >> 31; res += ~t & x;
    auto f = [] ( auto& v ) { auto res = 0; for ( auto x : v ) if ( x >= 128 ) res += x; return res; };
    auto test = [ &f ] ( auto n )
    {
        auto sorted = generate_vector( n );
        std::sort( sorted.begin(), sorted.end() );

        double sortedT, unsortedT;
        std::tie( sortedT, unsortedT ) = benchmark( n,
                                                    [ &sorted, &f ] { return f( sorted ); },
                                                    [ unsorted = generate_vector( n ), &f ] { return f( unsorted ); } );

        // much faster
        BOOST_CHECK( sortedT < unsortedT );
    };
    run_test< int >( "sorted;unsorted;", test, 1'000'000, 10'000'000, 100'000'000 );
}

// In symmetric multiprocessor (SMP) systems, each processor has a local cache. The memory system must guarantee cache coherence.
// False sharing occurs when threads on different processors (i.e. dosnt apply on HW on the same core) modify variables that reside on the same cache line. This invalidates the cache line and forces an update, which hurts performance.
// Coherence management requires full write to DRAM
// Variables which are never written to and those which are only initialized once are basically constants.Since RFO messages are only needed for write operations, constants can be shared in the cache( ‘S’ state ).
// So, these variables do not have to be treated specially; grouping them together is fine.If the programmer marks the variables correctly with const, the tool chain will move the variables away from the normal variables into the .rodata
// (read-only data) or .data.rel.ro( read - only after relocation ) No other special action is required. If, for some reason, variables cannot be marked correctly with const, the programmer can influence their placement by assigning them to a special section
// global variable non const will most likely reside in the same cache line with other global variable, even if these global have been declared in different translation unit, this could inccur hidden false sharing
// Rules of thumb:
//  - Separate at least read-only (after initialization) and read-write variables. Maybe extend this separation to read-mostly variables as a third category
//  - Group read-write variables which are used together into a structure.Using a structure is the only way to ensure the memory locations for all of those variables are close together in a way which is translated consistently by all gcc versions..
//  - Move read-write variables which are often written to by different threads onto their own cache line. This might mean adding padding at the end to fill a remainder of the cache line.
//  - If a variable is used by multiple threads, but every use is independent, move the variable into TLS (thread local storage)
BOOST_AUTO_TEST_CASE( FalseSharing1Benchmark )
{
    auto test = [] ( auto n )
    {
        auto v = generate_vector( n );
        auto f = [ &v, n ] ( int& r1, int& r2, int& r3, int& r4 )
        {
            auto processFunctor = [] ( int& r, int* first, int* last )
            {
                r = 0;
                while ( first != last )
                    r += *first++; // read-write
            };

            // thread management is costly and will bloat the result timing (making the false sharing effect less important that it is)
            std::thread t1( processFunctor, std::ref( r1 ), v.data(), v.data() + n / 4 );
            std::thread t2( processFunctor, std::ref( r2 ), v.data() + n / 4, v.data() + n / 2 );
            std::thread t3( processFunctor, std::ref( r3 ), v.data() + n / 2, v.data() + n * 3 / 4 );
            std::thread t4( processFunctor, std::ref( r4 ), v.data() + n * 3 / 4, v.data() + n );

            t1.join(); t2.join(); t3.join(); t4.join();
            return r1 + r2 + r3 + r4;
        };

        std::array< int, 49 > res;
        double sameCacheLineT, separateCacheLineT;
        std::tie( sameCacheLineT, separateCacheLineT ) = benchmark( n,
                                                                    [ &res, &f ] { return f( res[ 0 ], res[ 1 ], res[ 2 ], res[ 3 ] ); },
                                                                    [ &res, &f ] { return f( res[ 0 ], res[ 16 ], res[ 32 ], res[ 48 ] ); } );

        BOOST_CHECK( sameCacheLineT > separateCacheLineT );
    };
    run_test< int >( "sameCacheLine;separateCacheLine;", test, 10'000'000, 50'000'000 );
}

namespace
{
    static constexpr const size_t MatrixThreadNumber = 10;
}

BOOST_AUTO_TEST_CASE( FalseSharing2Benchmark )
{
    auto test = [ & ] ( auto dimension )
    {
        std::vector< int > matrix( dimension * dimension );
        std::generate( matrix.begin(), matrix.end(), std::rand );

        double notCacheFriendlyT, temporaryT;
        std::tie( notCacheFriendlyT, temporaryT ) = benchmark( dimension,
                                                     [ & ]
                                                     {
                                                         std::vector< std::thread > threads;
                                                         threads.reserve( MatrixThreadNumber );
                                                         std::array< int, MatrixThreadNumber > resultPerThread{ 0 };
                                                         for ( auto p = 0; p < MatrixThreadNumber; ++p )
                                                         {
                                                             threads.emplace_back( [&]( int threadNumber )
                                                             {
                                                                 auto startIndex = p * dimension;
                                                                 auto endIndex = std::min( startIndex + dimension, dimension );
                                                                 for ( auto i = startIndex; i < endIndex; ++i )
                                                                     for ( auto j = 0; j < dimension; ++j )
                                                                         if ( ( matrix[ i * dimension + j ] & 2 ) != 0 )
                                                                             ++resultPerThread[ threadNumber ]; // each write will invalidate the cache line holding this adress in the other cores
                                                             }, p );
                                                         }

                                                         for ( auto& thread : threads )
                                                             thread.join();
                                                         return std::accumulate( resultPerThread.begin(), resultPerThread.end(), 0 );
                                                     },
                                                     [ & ]
                                                     {
                                                         std::vector< std::thread > threads;
                                                         threads.reserve( MatrixThreadNumber );
                                                         std::array< int, MatrixThreadNumber > resultPerThread{ 0 };
                                                         for ( auto p = 0; p < MatrixThreadNumber; ++p )
                                                         {
                                                             threads.emplace_back( [ & ]( int threadNumber )
                                                             {
                                                                 auto result = 0;
                                                                 auto startIndex = threadNumber * dimension;
                                                                 auto endIndex = std::min( startIndex + dimension, dimension );
                                                                 for ( auto i = startIndex; i < endIndex; ++i )
                                                                     for ( auto j = 0; j < dimension; ++j )
                                                                         if ( ( matrix[ i * dimension + j ] & 2 ) != 0 )
                                                                             ++result;

                                                                 resultPerThread[ threadNumber ] = result; // Diminushing the effect of false sharing
                                                             }, p );
                                                         }

                                                         for ( auto& thread : threads )
                                                             thread.join();
                                                         return std::accumulate( resultPerThread.begin(), resultPerThread.end(), 0 );
                                                     } );

        BOOST_CHECK( temporaryT < notCacheFriendlyT );
    };
    // Effect is more apparent with bigger dimension
    run_test< long long >( "notCacheFriendly;temporary;", test, 1'000, 5'000, 10'000 );
}

namespace
{
    struct ArrowWithState
    {
        int     x, y, z;
        double  dx, dy, dz;
        bool    isInactive;

        double process() const { return x + y + z + dx + dy + dz; }
    };

    struct Arrow
    {
        int     x, y, z;
        double  dx, dy, dz;

        double process() const { return x + y + z + dx + dy + dz; }
    };
}

// Having a bool in a structure is most likely a anti pattern
BOOST_AUTO_TEST_CASE( DataLayoutBenchmark )
{
    auto test = [] ( auto n )
    {
        std::vector< ArrowWithState > arrowsWithState( n );

        // Remove active flag by smartly laying out objects
        // Better cache density, no branching, fewer elements processed
        std::vector< Arrow > arrows( n );
        std::vector< int > inactiveArrows; // index of active arrow

        // In the case only few arrows can be actives
        std::uniform_int_distribution<> rnd( 1, n / 100 );
        std::mt19937 gen;
        for ( auto i = 0, size = rnd( gen ); i < size; ++i )
        {
            auto rdm = rnd( gen );
            arrowsWithState[ rdm ].isInactive = true;
            inactiveArrows.push_back( rdm );
        }

        double arrowT, arrowStateT;
        std::tie( arrowT, arrowStateT ) = benchmark( n,
                                                     [ &arrows, &inactiveArrows ] { auto res = 0.0; for ( auto i : inactiveArrows ) res += arrows[ i ].process(); return res; },
                                                     [ &arrowsWithState ] { auto res = 0.0; for ( auto& a : arrowsWithState ) if ( a.isInactive ) res += a.process(); return res; } );

        BOOST_CHECK( arrowT < arrowStateT );
    };
    run_test< int >( "arrow;arrowState;", test, 15'000, 70'000, 500'000 );
}

namespace
{
    // The big cost of virtual functions isn't really the lookup of a function pointer in the vtable (that's usually just a single cycle),
    // but that the indirect jump usually cannot be branch-predicted. This can cause a large pipeline bubble as the processor
    // cannot fetch any instructions until the indirect jump (the call through the function pointer) has retired and a new instruction pointer computed.
    // So, the cost of a virtual function call is much bigger than it might seem from looking at the assembly

    // a virtual function call may cause an instruction cache miss: if you jump to a code address that is not in cache then the whole program comes
    // to a dead halt while the instructions are fetched from main memory. This is always a significant stall: on Xenon, about 650 cycles (by my tests).
    // However this isn't a problem specific to virtual functions because even a direct function call will cause a miss if you jump to instructions
    // that aren't in cache. What matters is whether the function has been run before recently (making it more likely to be in cache),
    // and whether your architecture can predict static (not virtual) branches and fetch those instructions into cache ahead of time.
    struct Base { virtual ~Base() {}; virtual int f() const = 0; };
    struct Derived1 : Base { virtual int f() const override final { return 1; } };
    struct Derived2 : Base { virtual int f() const override final { return 2; } };
    struct Derived3 : Base { virtual int f() const override final { return 3; } };

    template < typename T, typename V, typename C >
    void    polymorphism_container_add_element( V& v1, V& v2, C& p )
    {
        v1.emplace_back( std::make_unique< T >() );
        v2.emplace_back( std::make_unique< T >() );
        p.insert( T() );
    }

    template < typename V, typename C >
    void    init_polymorphic_container( V& v1, V& v2, C& p, size_t n )
    {
        v1.reserve( n );
        v2.reserve( n );

        std::mt19937 gen;
        std::uniform_int_distribution<> rnd( 1, 3 );
        for ( auto i = 0; i < n; ++i )
        {
            switch ( rnd( gen ) )
            {
                case 1: polymorphism_container_add_element< Derived1 >( v1, v2, p ); break;
                case 2: polymorphism_container_add_element< Derived2 >( v1, v2, p ); break;
                case 3: default: polymorphism_container_add_element< Derived3 >( v1, v2, p ); break;
            }
        }

        // Shuffle to avoid same derived class instances to be in the same cache line (as they are being allocated in order)
        std::shuffle( v1.begin(), v1.end(), gen );

        std::shuffle( v2.begin(), v2.end(), gen );
        std::sort( v2.begin(), v2.end(), [] ( auto& p, auto& q ) { return std::type_index( typeid( *p ) ) < std::type_index( typeid( *q ) ); } );
    }
}

// Sorting improve branch prediction even without data locality
// In this test, it helps greatly the branch prediction as the same virtual table will be used for a long period of time
BOOST_AUTO_TEST_CASE( PolymorphicContainerBenchmark )
{
    auto f = [] ( auto& v ) { auto res = 0; for ( const auto& e : v ) res += e->f(); return res; };
    auto test = [ &f ] ( auto n )
    {
        std::vector< std::unique_ptr< Base > > unsorted, sorted;
        containers::PolymorphicCollection< Base > collection;

        init_polymorphic_container( unsorted, sorted, collection, n );

        double unsortedT, sortedT, collectionT;
        std::tie( unsortedT, sortedT, collectionT ) = benchmark( n,
            [ &unsorted, &f ] { return f( unsorted ); },
            [ &sorted, &f ] { return f( sorted ); },
            [ &collection ] { auto res = 0; collection.for_each( [ &res ] ( auto& e ) { res += e.f(); } ); return res; } );

        BOOST_CHECK( unsortedT > sortedT );
        BOOST_CHECK( sortedT > collectionT );
    };

    run_test< int >( "unsorted;sorted;collection;", test, 15'000, 100'000, 500'000 );
}

BOOST_AUTO_TEST_SUITE_END() // ! CacheTestSuite
