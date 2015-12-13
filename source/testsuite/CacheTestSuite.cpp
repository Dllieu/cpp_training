//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/multi_array.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <array>
#include <random>
#include <unordered_map>

#include "generic/Typetraits.h"

namespace sch = std::chrono;

namespace
{
    constexpr auto operator""   _KB( size_t s ) { return s * 1024; }
    constexpr auto operator""   _MB( size_t s ) { return s * 1024 * 1000; }

    // Intel Core i5-4460 (4 cores)
    // cache line : 64B
    // L1 instruction cache : 32KB
    // - L1  (32KB):    1   ns /   4 cycles (2 per processor (hyperthreading can have 2 thread per processor))
    // - L2 (256KB):    3.1 ns /  12 cycles (per processor)
    // - L3   (6MB):    7.7 ns /  30 cycles (share among all the processors)
    // - DRAM  (XX):   60   ns / 237 cycles

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
    void    displaySpaceInformation( size_t numberElements )
    {
        std::cout << "-----\nnumber of elements to run: " << numberElements << "\n";

        auto byteNumber = numberElements * sizeof( T );
        std::cout << "(optimal) cache line needed: " << std::ceil( byteNumber / 64 ) << "\n";
        std::cout << "Need at least " << byteNumber / 1024. << "KB space (" << toString( byteToAppropriateCacheSize< T >( numberElements ) ) << ")" << std::endl;
    }

    static constexpr int                NumberTrials = 14;
    static constexpr sch::milliseconds  MinTimePerTrial( 200 );

    // return average of microseconds per f() call
    template < typename S, typename F >
    auto    measure( S&& s, F&& f )
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
        auto result = std::accumulate( trials.begin() + 2, trials.end() - 2, 0.0 ) / ( trials.size() - 4 ) * 1E6;
        std::cout << "- " << s << ": " << result << " microseconds" << std::endl;
        return result;
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
        displaySpaceInformation< int >( n );

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
        displaySpaceInformation< int >( n * n );

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
        displaySpaceInformation< int >( n );

        measure_accumulate( "unordered_map", generateUnorderedMap( n ), binaryOperation );
        measure_accumulate( "map          ", generateMap( n ), binaryOperation );

        // unordered_map beat map for low N, at some point map is more cache friendly (in this case from 1M)
    }
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_CASE( AssociativeTraversalTest )
{
    // Cache is less a factor than complexity in this test
    for ( auto n : { 4'096, 100'000, 1'000'000 } )
    {
        displaySpaceInformation< int >( n );

        auto um = generateUnorderedMap( n );
        auto unorderedMapTime   = measure( "unordered_map", [ &um ] { auto n = 0; for ( auto i = 0; i < um.size(); ++i ) n += um[ i ]; return n; } ); // hash + access(O(1))

        auto m = generateMap( n );
        auto mapTime            = measure( "map          ", [ &m ] { auto n = 0; for ( auto i = 0; i < m.size(); ++i ) n += m[ i ]; return n; } ); // access(O(log n))

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
    for ( auto n : { 4'096, 8'192, 16'384 } )
    {
        displaySpaceInformation< Particle >( n );

        AOSParticle aos( n );
        // 64 / ( 6 / 3 ) / sizeof( int ) = 8 useful values per fetch average (6 / 3 :  only need x, y, z)
        auto aosTime = measure( "aos", [ &aos, n ]{ auto res = 0; for ( auto i = 0; i < n; ++i ) res += aos[i].x + aos[i].y + aos[i].z; return res; } );


        SOAParticle soa( n );
        // 64 / sizeof( int ) = 16 useful values per fetch
        auto soaTime = measure( "soa", [ &soa, n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += soa.x[ i ] + soa.y[ i ] + soa.z[ i ]; return res; } );

        BOOST_CHECK( aosTime > soaTime );
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
    for ( auto n : { 4'096, 16'384, 100'000, 1'000'000 } )
    {
        displaySpaceInformation< CompactParticle >( n );

        // Both fetch only useful values
        // SOA is faster (diminishingly as n grows), because CPU can prefetch x, y, z in parallel
        // (e.g. big picture: aos need to prefetch (stale) every N bytes, but soa only need to prefetch every N * 3 (the is more expensive, but less than the stale depending of the cache layer))
        AOSCompactParticle aos( n );
        auto aosTime = measure( "aos", [ &aos, n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += aos[ i ].x + aos[ i ].y + aos[ i ].z; return res; } );

        SOACompactParticle soa( n );
        auto soaTime = measure( "soa", [ &soa, n ] { auto res = 0; for ( auto i = 0; i < n; ++i ) res += soa.x[ i ] + soa.y[ i ] + soa.z[ i ]; return res; } );


        if ( byteToAppropriateCacheSize< CompactParticle >( n ) < CacheSize::DRAM )
            BOOST_CHECK( aosTime > soaTime );
    }
}

BOOST_AUTO_TEST_CASE( CompactRandomAccessAOSvsSOATest )
{
    for ( auto n : { 512, 4'096, 16'384, 100'000, 1'000'000 } )
    {
        displaySpaceInformation< CompactParticle >( n );

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

        // Similar results, but passed L2 cache, aos is more efficient (less staling due to the prefetch)
        if ( byteToAppropriateCacheSize< CompactParticle >( n ) > CacheSize::L2 )
            BOOST_CHECK( aosTime < soaTime );
    }
}

BOOST_AUTO_TEST_SUITE_END() // ! CacheTestSuite
