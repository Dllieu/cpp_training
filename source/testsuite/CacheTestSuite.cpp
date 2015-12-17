//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma warning( push )
#pragma warning( disable : 4996 ) // warning raise when using unit_test with multi_array instanciation
#include <boost/test/unit_test.hpp>
#pragma warning( pop )
#include <boost/multi_array.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <array>
#include <random>
#include <unordered_map>
#include <thread>
#include <type_traits>
#include <typeindex>

#include "generic/Typetraits.h"
#include "containers/PolymorphicCollection.h"

namespace sch = std::chrono;

namespace
{
    constexpr auto operator""   _KB( size_t s ) { return s * 1024; }
    constexpr auto operator""   _MB( size_t s ) { return s * 1024 * 1000; }

    // Intel Core i5-4460 (4 cores) (give a general idea)
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
    // Instruction cache
    // - L1  (32KB), 8-WAY, 64B / line
    //
    // Data Cache (8-WAY, 64B / line)
    // - L1  (32KB):    1   ns /   4 cycles
    //             * 2 per processor (hyperthreading delivers two processing threads per physical core)
    //             * 4 cycles for simple access via pointer (p), 5 cycles for access with complex address calculation (p[n])
    // - L2 (256KB):    3.1 ns /  12 cycles (per processor)
    // - L3   (6MB):    7.7 ns /  30 cycles (share among all the processors)
    //             * 29.5 cycles for cores (1, 2)
    //             * 30.5 cycles for cores (0, 3)
    // - DRAM  (XX):   60   ns / 237 cycles
    //
    //
    // -> Check for more infos: http://www.7-cpu.com/

    // About cache misses
    //  A cache miss refers to a failed attempt to read or write a piece of data in the cache, which results in a main memory access with much longer latency.
    //  There are three kinds of cache misses: instruction read miss, data read miss, and data write miss.
    //  - Cache read misses from an instruction cache generally cause the largest delay, because the processor, or at least the thread of execution, has to wait (stall) until the instruction is fetched from main memory.
    //  - Cache read misses from a data cache usually cause a smaller delay, because instructions not dependent on the cache read can be issued and continue execution until the data is returned from main memory,
    //    and the dependent instructions can resume execution.
    //  - Cache write misses to a data cache generally cause the shortest delay, because the write can be queued and there are few limitations on the execution of subsequent instructions; the processor can continue until the queue is full.

    // Max number of segment in L1 = 32KB / 64 = 512
    enum class CacheSize
    {
        L1 = 32_KB,
        L2 = 256_KB,
        L3 = 6_MB,
        DRAM
    };

    template < typename T >
    CacheSize   byteToAppropriateCacheSize( size_t numberElements )
    {
        auto byteSize = numberElements * sizeof( T );
        if ( byteSize < generics::enum_cast( CacheSize::L1 ) )
            return CacheSize::L1;

        if ( byteSize < generics::enum_cast( CacheSize::L2 ) )
            return CacheSize::L2;

        if ( byteSize < generics::enum_cast( CacheSize::L3 ) )
            return CacheSize::L3;

        return CacheSize::DRAM;
    }

    const char* toString( CacheSize cacheSize )
    {
        switch ( cacheSize )
        {
            case CacheSize::L1:
                return "L1";

            case CacheSize::L2:
                return "L2";

            case CacheSize::L3:
                return "L3";

            default:
                return "DRAM";
        }
    }

    template < typename T >
    void    display_information( size_t n )
    {
        auto byteNumber = n * sizeof( T );
        std::cout << "(CL=" << std::ceil( byteNumber / 64 );
        std::cout << "|SN=" << std::ceil( byteNumber / 1024. ) << "KB[" << toString( byteToAppropriateCacheSize< T >( n ) ) << "]);" << n << ";";
    }

    static constexpr int                NumberTrials = 20;
    static constexpr sch::milliseconds  MinTimePerTrial( 200 );

    // return average of microseconds per f() call
    template < typename S, typename F >
    auto    measure( S&& s, F&& f )
    {
        static_assert( !std::is_void< std::result_of_t< F() > >(), "F cannot be void" );

        volatile decltype( f() ) res; // to avoid optimizing f() away

        std::array< double, NumberTrials > trials;
        for ( auto i = 0; i < NumberTrials; ++i )
        {
            auto runs = 0;

            sch::high_resolution_clock::time_point now;
            auto startTimer = sch::high_resolution_clock::now();
            do
            {
                res = f();
                ++runs;
                now = sch::high_resolution_clock::now();
            } while ( now - startTimer < MinTimePerTrial );
            trials[ i ] = sch::duration_cast< sch::duration< double > >( now - startTimer ).count() / runs;
        }
        static_cast< void >( res );

        std::sort( trials.begin(), trials.end() );
        auto result = std::accumulate( trials.begin() + 2, trials.end() - 2, 0.0 ) / ( trials.size() - 4 ) * 1E6;
        //std::cout << "- " << s << ": " << result << " microseconds" << std::endl;
        return result;
    }

    template < typename F >
    auto    measure( size_t n, F&& f )
    {
        volatile decltype( f() ) res; // to avoid optimizing f() away

        std::array< double, NumberTrials > trials;
        for ( auto i = 0; i < NumberTrials; ++i )
        {
            auto runs = 0;

            sch::high_resolution_clock::time_point now;
            auto startTimer = sch::high_resolution_clock::now();
            do
            {
                res = f();
                ++runs;
                now = sch::high_resolution_clock::now();
            } while ( now - startTimer < MinTimePerTrial );
            trials[ i ] = sch::duration_cast< sch::duration< double > >( now - startTimer ).count() / runs;
        }
        static_cast< void >( res );

        std::sort( trials.begin(), trials.end() );
        auto result = std::accumulate( trials.begin() + 2, trials.end() - 2, 0.0 ) / ( trials.size() - 4 ) * 1E6 / n;
        std::cout << result << ";";

        return result;
    }

    template < typename... Fs >
    auto    measure_test( size_t n, Fs&&... fs )
    {
        // std::make_tuple reverse the call oder of fs when executed (he start by the end?)
        // auto result = std::make_tuple( measure( n, std::forward< Fs >( fs ) )... );
        auto result = std::vector< double >( { measure( n, std::forward< Fs >( fs ) )... } );
        std::cout << std::endl;
        return result;
    }

    template < typename ELEMENT_TYPE, typename F, typename... Ns >
    void    run_test( const std::string& header, F&& f, Ns... range )
    {
        std::cout << "infos;n;" << header << std::endl;
        for ( auto n : { range... } )
        {
            display_information< ELEMENT_TYPE >( n );
            f( n );
        }
    }
}

// Classical big-O algorithmic complexity analysis proves insufficient to estimate program performance for modern computer architectures,
// current processors are equipped with several low-level components (hierarchical cache structures, pipelining, branch prediction)
// that greatly favor certain code and data layout patterns not taken into account by naive computation models.
BOOST_AUTO_TEST_SUITE( CacheTestSuite )

namespace
{
    // always use accumulate to ensure O(n) traversal
    template < typename S, typename Container, typename BinaryOperation >
    auto    measure_accumulate( S&& s, Container&& c, BinaryOperation&& op )
    {
        return measure( std::forward< S >( s ), [ & ] { return std::accumulate( std::begin( c ), std::end( c ), 0, op ); } );
    }

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
// std::vector / std::array excels at both
// std::list sequentially allocated nodes provide some sort of non-guaranteed locality
// shuffled nodes is the worst scenario
BOOST_AUTO_TEST_CASE( LinearTraversalTest )
{
    auto f = [] ( auto r, auto n ) { return r + n; };
    for ( auto n : { 4'096, 100'000, 1'000'000 } )
    {
        //displaySpaceInformation< int >( n );

        auto vectorTime         = measure_accumulate( "vector       ", std::vector< int >( n ), f );
        auto listTime           = measure_accumulate( "list         ", std::list< int >( n ), f );
        auto shuffledListTime   = measure_accumulate( "shuffled list", generateShuffledList( n ), f );

        BOOST_CHECK( vectorTime <= listTime );
        // In case all the node could be hold in L1 cache
        if ( byteToAppropriateCacheSize< int >( n ) > CacheSize::L1 )
            BOOST_CHECK( listTime < shuffledListTime );
    }
}

BOOST_AUTO_TEST_CASE( MatrixTraversalTest )
{
    for ( auto n : { 124, 512, 1'024 } )
    {
        //displaySpaceInformation< int >( n * n );

        // multiple array with contiguous data
        boost::multi_array< int, 2 > m1( boost::extents[ n ][ n ] ), m2( boost::extents[ n ][ n ] );

        auto rowTraversalTime = measure( "row traversal", [ &m1, n ]
            {
                auto res = 0;
                for ( auto row = 0; row < n; ++row )
                    for ( auto col = 0; col < n; ++col )
                        res += m1[row][col];
                return res;
            } );

        auto colTraversalTime = measure( "col traversal", [ &m2, n ]
        {
            auto res = 0;
            for ( auto col = 0; col < n; ++col )
                for ( auto row = 0; row < n; ++row )
                    res += m2[ row ][ col ];
            return res;
        } );

        // In case all the node could be hold in L2 cache
        if ( byteToAppropriateCacheSize< int >( n * n ) > CacheSize::L2 )
            BOOST_CHECK( rowTraversalTime < colTraversalTime );
    }
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
BOOST_AUTO_TEST_CASE( AssociativeTraversalIteratorTest )
{
    auto binaryOperation = [] ( auto r, const auto& p ) { return r + p.second; };
    for ( auto n : { 4'096, 100'000, 1'000'000, 10'000'000 } )
    {
        //displaySpaceInformation< int >( n );

        measure_accumulate( "unordered_map", generateUnorderedMap( n ), binaryOperation );
        measure_accumulate( "map          ", generateMap( n ), binaryOperation );

        // unordered_map beat map for low N, at some point map is more cache friendly (in this case from 1M)
    }
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_CASE( AssociativeTraversalTest )
{
    auto test = [] ( auto& m ) { auto n = 0; for ( auto i = 0; i < m.size(); ++i ) n += m[ i ]; return n; };
    // Cache is less a factor than complexity in this test
    for ( auto n : { 4'096, 100'000, 1'000'000 } )
    {
        //displaySpaceInformation< int >( n );

        auto um = generateUnorderedMap( n );
        auto unorderedMapTime   = measure( "unordered_map", [ &um, &test ] { return test( um ); } ); // hash + access(O(1))

        auto m = generateMap( n );
        auto mapTime            = measure( "map          ", [ &m, &test ] { return test( m ); } ); // access(O(log n))

        BOOST_CHECK( unorderedMapTime < mapTime );
    }
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

BOOST_AUTO_TEST_CASE( AOSvsSOATest )
{
    for ( auto n : { 4'096, 16'384, 100'000, 1'000'000, 10'000'000 } )
    {
        //displaySpaceInformation< Particle >( n );

        AOSParticle aos( n );
        //init_aos_particle( aos, n );
        // 64 / ( 6 / 3 ) / sizeof( int ) = 8 useful values per fetch average (6 / 3 :  only need x, y, z)
        auto aosTime = measure( "aos", [ &aos, n ]{ auto res = 0; for ( auto i = 0; i < n; ++i ) res += aos[i].x + aos[i].y + aos[i].z; return res; } );


        SOAParticle soa( n );
        // 64 / sizeof( int ) = 16 useful values per fetch
        auto soaTime = measure( "soa", [ &soa, n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += soa.x[ i ] + soa.y[ i ] + soa.z[ i ]; return res; } );

        BOOST_CHECK( soaTime < aosTime );
    }
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

BOOST_AUTO_TEST_CASE( CompactAOSvsSOATest )
{
    for ( auto n : { 4'096, 16'384, 100'000, 1'000'000, 10'000'000, 20'000'000 } )
    {
        //displaySpaceInformation< CompactParticle >( n );

        // Both fetch only useful values
        // SOA is faster (diminishingly as n grows), because CPU can prefetch x, y, z in parallel
        // (e.g. big picture: aos need to prefetch (stale) every N bytes, but soa only need to prefetch every N * 3 (the is more expensive, but less than the stale depending of the cache layer))
        AOSCompactParticle aos( n );
        auto aosTime = measure( "aos", [ &aos, n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += aos[ i ].x + aos[ i ].y + aos[ i ].z; return res; } );

        SOACompactParticle soa( n );
        auto soaTime = measure( "soa", [ &soa, n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += soa.x[ i ] + soa.y[ i ] + soa.z[ i ]; return res; } );

        // SOA is faster (diminishingly as n grows)
        if ( byteToAppropriateCacheSize< CompactParticle >( n ) < CacheSize::DRAM )
            BOOST_CHECK( soaTime < aosTime );
    }
}

BOOST_AUTO_TEST_CASE( CompactRandomAccessAOSvsSOATest )
{
    for ( auto n : { 512, 4'096, 16'384, 100'000, 1'000'000, 1'200'000, 1'800'000 } )
    {
        //displaySpaceInformation< CompactParticle >( n );

        AOSCompactParticle aos( n );
        auto aosTime = measure( "aos", [ &aos, n ]
                        {
                            auto res = 0; std::uniform_int_distribution<> rnd( 0, n - 1 ); std::mt19937 gen;
                            for ( auto i = 0; i < n; ++i )
                            {
                                auto idx = rnd( gen );
                                res += aos[ idx ].x + aos[ idx ].y + aos[ idx ].z;
                            }
                            return res;
                        } );

        SOACompactParticle soa( n );
        auto soaTime = measure( "soa", [ &soa, n ]
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
            BOOST_CHECK( aosTime < soaTime );
    }
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
//
// - The time that is wasted in case of a branch misprediction is equal to the number of stages in the pipeline from the fetch stage to the execute stage.
//   Modern microprocessors tend to have quite long pipelines so that the misprediction delay is between 10 and 20 clock cycles.
//   As a result, making a pipeline longer increases the need for a more advanced branch predictor.
//
// - Flow:
//   - Waiting instructions
//   - Pipelining
//     1 - Fetch
//     2 - Decode
//     3 - Execute
//     4 - Write-back (writing the results of the instruction to processor registers or to memory)
//   - Completed instructions
BOOST_AUTO_TEST_CASE( BranchPredictionTest )
{
    for ( auto n : { 4'096, 16'384, 100'000 } )
    {
        //displaySpaceInformation< int >( n );

        auto v = generate_vector( n );
        auto test = [ &v ] { auto res = 0; for ( auto x : v ) if ( x > 128 ) res += x; return res; };

        auto unsortedTime = measure( "unsorted vector", test );

        std::sort( v.begin(), v.end() );
        auto sortedTime = measure( "sorted vector  ", test );

        // much faster
        BOOST_CHECK( sortedTime < unsortedTime );
    }
}

// In symmetric multiprocessor (SMP) systems, each processor has a local cache. The memory system must guarantee cache coherence.
// False sharing occurs when threads on different processors modify variables that reside on the same cache line. This invalidates the cache line and forces an update, which hurts performance.
// Coherence management requires full write to DRAM
// Rule of thumb: use shared write memory only to communicate
BOOST_AUTO_TEST_CASE( FalseSharingTest )
{
    for ( auto n : { 1'000'000, 10'000'000, 100'000'000 } )
    {
        //displaySpaceInformation< int >( n );

        auto v = generate_vector( n );
        auto test = [ &v, n ] ( int& r1, int& r2, int& r3, int& r4 )
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
        auto sameCachelineTime = measure( "same     cacheline", [ &res, &test ] { return test( res[ 0 ], res[ 1 ], res[ 2 ], res[ 3 ] ); } );
        auto separateCachelineTime = measure( "separate cacheline", [ &res, &test ] { return test( res[ 0 ], res[ 16 ], res[ 32 ], res[ 48 ] ); } );

        BOOST_CHECK( separateCachelineTime < sameCachelineTime );
    }
}

namespace
{
    struct ArrowWithState
    {
        int     x, y, z;
        double  dx, dy, dz;
        bool    isActive;

        double process() const { return x + y + z + dx + dy + dz; }
    };

    struct Arrow
    {
        int     x, y, z;
        double  dx, dy, dz;

        double process() const { return x + y + z + dx + dy + dz; }
    };
}

BOOST_AUTO_TEST_CASE( DataLayoutTest )
{
    for ( auto n : { 4'096, 16'384, 50'000, 100'000 } )
    {
        //displaySpaceInformation< ArrowWithState >( n );

        std::vector< ArrowWithState > arrowsWithState( n );

        // Remove active flag by smartly laying out objects
        // Better cache density, no branching, fewer elements processed
        std::vector< Arrow > arrows( n );
        std::vector< int > activeArrows; // index of active arrow

        std::uniform_int_distribution<> rnd( n / 100, n - 1 );
        std::mt19937 gen;
        for ( auto i = 0, size = rnd( gen ); i < size; ++i )
        {
            auto rdm = rnd( gen );
            arrowsWithState[ rdm ].isActive = true;
            activeArrows.push_back( rdm );
        }

        auto arrowTime = measure( "arrow           ", [ &arrows, &activeArrows ] { auto res = 0.0; for ( auto i : activeArrows ) res += arrows[ i ].process(); return res; } );
        auto arrowStateTime = measure( "arrow with state", [ &arrowsWithState ] { auto res = 0.0; for ( auto& a: arrowsWithState ) if ( a.isActive ) res += a.process(); return res; } );

        // result will vary due to random
        BOOST_CHECK( arrowTime < arrowStateTime );
    }
}

namespace
{
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
BOOST_AUTO_TEST_CASE( PolymorphicContainerTest )
{
    auto f = [] ( auto& v ) { auto res = 0; for ( const auto& e : v ) res += e->f(); return res; };
    auto test = [ &f ] ( auto n )
    {
        std::vector< std::unique_ptr< Base > > unsorted, sorted;
        containers::PolymorphicCollection< Base > collection;

        init_polymorphic_container( unsorted, sorted, collection, n );

        auto r = measure_test( n,
                               [ &unsorted, &f ] { return f( unsorted ); },
                               [ &sorted, &f ] { return f( sorted ); },
                               [ &collection ] { auto res = 0; collection.for_each( [ &res ] ( auto& e ) { res += e.f(); } ); return res; } );

        BOOST_CHECK( r[ 0 ] > r[ 1 ] );
        BOOST_CHECK( r[ 1 ] > r[ 2 ] );
    };

    run_test< int >( "unsorted;sorted;collection;", test, 15'000, 100'000, 500'000 );
}

BOOST_AUTO_TEST_SUITE_END() // ! CacheTestSuite
