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

#include "tools/Timer.h"

namespace sch = std::chrono;

// Classical big-O algorithmic complexity analysis proves insufficient to estimate program performance for modern computer architectures,
// current processors are equipped with several low-level components (hierarchical cache structures, pipelining, branch prediction)
// that greatly favor certain code and data layout patterns not taken into account by naïve computation models.
BOOST_AUTO_TEST_SUITE( CacheTestSuite )

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
        static_cast< void >( res ); // warning

        std::sort( trials.begin(), trials.end() );
        return std::accumulate( trials.begin() + 2, trials.end() - 2, 0.0 ) / ( trials.size() - 4 ) * 1E6;
    }

    template < typename S, typename Container >
    auto    start_measure( S&& s, Container&& c )
    {
        auto result = measure( [ & ] () { return std::accumulate( std::begin( c ), std::end( c ), 0 ); } );
        std::cout << s << ": " << result << " microseconds" << std::endl;
        return result;
    }

    std::list< int >    generateShuffledList( int size )
    {
        std::uniform_int_distribution<> rnd( 0, size - 1 );
        std::mt19937 gen;

        std::list<int> result( size );
        std::generate( result.begin(), result.end(), [ &rnd, &gen ] { return rnd( gen ); } );
        result.sort();

        return result;
    }

    void    displaySpaceInformation( size_t n )
    {
        std::cout << "-----\nnumber of elements to run: " << n << "\n";
        std::cout << "(optimal) cache line needed: " << std::ceil( n / 64 ) << "\n";

        auto kbNeeded = std::ceil( n / 1024 );
        std::cout << "Need at least " << kbNeeded
                  << "KB space (enough for " << (kbNeeded < 32 ? "L1" : kbNeeded < 256 ? "L2" : kbNeeded < 8'000 * 1'024 ? "L3" : "DRAM" ) << ")" << std::endl;
    }
}

BOOST_AUTO_TEST_CASE( LinearTraversalTest )
{
    for ( auto n : { 4096, 100'000, 1'000'000 } )
    {
        displaySpaceInformation( n );

        auto vectorTime         = start_measure( "vector       ", std::vector< int >( n ) );
        auto listTime           = start_measure( "list         ", std::list< int >( n ) );
        auto shuffledListTime   = start_measure( "shuffled list", generateShuffledList( n ) );

        BOOST_CHECK( vectorTime <= listTime && listTime <= shuffledListTime );
    }
}

BOOST_AUTO_TEST_SUITE_END() // ! CacheTestSuite
