//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <array>
#include <random>
#include <unordered_map>

namespace sch = std::chrono;

// Intel Core i7-3770K
// cache line : 64B
// - L1  (32KB):    1   ns /   4 cycles 
// - L2 (256KB):    3.1 ns /  12 cycles
// - L3   (8MB):    7.7 ns /  30 cycles
// - DRAM  (XX):   60   ns / 237 cycles
namespace
{
    static constexpr int                NumberTrials = 14;
    static constexpr sch::milliseconds  MinTimePerTrial( 200 );

    // return average of microseconds per f() call
    template < typename F >
    auto    measure( F&& f )
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
        return std::accumulate( trials.begin() + 2, trials.end() - 2, 0.0 ) / ( trials.size() - 4 ) * 1E6;
    }

    void    displaySpaceInformation( size_t numberElements )
    {
        std::cout << "-----\nnumber of elements to run: " << numberElements << "\n";

        auto byteNumber = numberElements * sizeof( int );
        std::cout << "(optimal) cache line needed: " << std::ceil( byteNumber / 64 ) << "\n";

        auto kbNeeded = byteNumber / 1024.;
        std::cout << "Need at least " << kbNeeded
                  << "KB space (enough for " << ( kbNeeded < 32 ? "L1"
                                                                : kbNeeded < 256 ? "L2"
                                                                                 : kbNeeded < 8'000 * 1'024 ? "L3" : "DRAM" )
                  << ")" << std::endl;
    }

    constexpr auto operator""   _KB( size_t s )
    {
        return s * 1024;
    }
}

// Classical big-O algorithmic complexity analysis proves insufficient to estimate program performance for modern computer architectures,
// current processors are equipped with several low-level components (hierarchical cache structures, pipelining, branch prediction)
// that greatly favor certain code and data layout patterns not taken into account by naive computation models.
BOOST_AUTO_TEST_SUITE( CacheTestSuite )

namespace
{
    template < typename S, typename Container >
    auto    measure_accumulate( S&& s, Container&& c )
    {
        return measure_accumulate_op( std::forward< S >( s ), std::forward< Container >( c ), []( auto r, auto n ) { return r + n; } );
    }

    // always use accumulate to ensure O(n) traversal
    template < typename S, typename Container, typename BinaryOperation >
    auto    measure_accumulate_op( S&& s, Container&& c, BinaryOperation&& op )
    {
        auto result = measure( [ & ] { return std::accumulate( std::begin( c ), std::end( c ), 0, op ); } );
        std::cout << "- " << s << ": " << result << " microseconds" << std::endl;
        return result;
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

BOOST_AUTO_TEST_CASE( LinearTraversalTest )
{
    for ( auto n : { 4096, 100'000, 1'000'000 } )
    {
        displaySpaceInformation( n );

        auto vectorTime         = measure_accumulate( "vector       ", std::vector< int >( n ) );
        auto listTime           = measure_accumulate( "list         ", std::list< int >( n ) );
        auto shuffledListTime   = measure_accumulate( "shuffled list", generateShuffledList( n ) );

        BOOST_CHECK( vectorTime <= listTime );
        // In case all the node could be hold in L1 cache
        if ( n > 32_KB )
            BOOST_CHECK( listTime <= shuffledListTime );
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
    for ( auto n : { 4096, 100'000, 1'000'000, 10'000'000 } )
    {
        displaySpaceInformation( n );

        measure_accumulate_op( "unordered_map", generateUnorderedMap( n ), binaryOperation );
        measure_accumulate_op( "map          ", generateMap( n ), binaryOperation );

        // unordered_map beat map for low N, at some point map is more cache friendly (in this case from 1M)
    }
    BOOST_CHECK( true );
}

namespace
{
    template < typename S, typename Container >
    auto    measure_traversal( S&& s, Container&& c )
    {
        auto result = measure( [ &c ] { auto n = 0; for ( auto i = 0; i < c.size(); ++i ) n += c[ i ]; return n; } );
        std::cout << "- " << s << ": " << result << " microseconds" << std::endl;
        return result;
    }
}

BOOST_AUTO_TEST_CASE( AssociativeTraversalTest )
{
    // Cache is less a factor than complexity in this test
    for ( auto n : { 4096, 100'000, 1'000'000 } )
    {
        displaySpaceInformation( n );

        auto unorderedMapTime   = measure_traversal( "unordered_map", generateUnorderedMap( n ) ); // hash + access(O(1))
        auto mapTime            = measure_traversal( "map          ", generateMap( n ) ); // access(O(log n))

        BOOST_CHECK( unorderedMapTime <= mapTime );
    }
}

BOOST_AUTO_TEST_SUITE_END() // ! CacheTestSuite
