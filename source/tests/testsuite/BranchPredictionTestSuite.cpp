//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "tools/Timer.h"

BOOST_AUTO_TEST_SUITE( BranchPrediction )

namespace
{
    #define BURNSOMETIMEFUNCTION( functionName ) \
    void functionName() \
    { \
        int result = 0; \
        for ( int i = 0; i < 1000000; ++i ) \
            result += i; \
    }
    BURNSOMETIMEFUNCTION( fooA );
    BURNSOMETIMEFUNCTION( fooB );
    BURNSOMETIMEFUNCTION( fooC );
    #undef BURNSOMETIMEFUNCTION

    // Regrouping call of non related function
    double  elapsedCase1( unsigned maxIteration )
    {
        tools::Timer t( __FUNCTION__ );

        for ( unsigned i = 0; i < maxIteration; ++i )
        {
            fooA();
            fooB();
            fooC();
        }
        return t.elapsed();
    }

    // Dispatching call of non related function (easier to find it in the instruction cache)
    double  elapsedCase2( unsigned maxIteration )
    {
        tools::Timer t( __FUNCTION__ );

        for ( unsigned i = 0; i < maxIteration; ++i )
            fooA();

        for ( unsigned i = 0; i < maxIteration; ++i )
            fooB();

        for ( unsigned i = 0; i < maxIteration; ++i )
            fooC();
        return t.elapsed();
    }
}

// these are only reasons why it could be faster (of course it depends of what exactly are A B and C)
// case1
//    - only a single occurrence of loop prologue/epilogue (less code to run)
//    - better scheduling of A B and C generated code (more parallelism)
//    - may factorize code (no dependency on output, but A B and C may read the same inputs)
// case2
//    - lower register pressure in each loop (avoid spilling)
//    - more likely to unroll loop (when A, B or C is trivial)
//    - more likely the entire loop being into instruction cache (useful when N is big)

// Assuming no dependencies between A, B and C, Case 2 would normally be faster than case 1 because of:
//    - Data locality
//    - Code locality
//    - Branch prediction
// However, if the code blocks are very short, then theoretically the extra loop overhead in Case 2 might dominate
// Of course, if there are truly no dependencies, then the compiler is free to transform Case 1 into Case 2, and vice versa.

// In addition to the reasons already mentionned (locality favoring 2, loop overhead favoring 1),
// it's possible that the compiler could optimize them differently: case 1 gives it the possibility of interleaving instructions from the three blocks,
// possibliy avoiding pipeline stall if there are dependencies in the instructions of any one block.
BOOST_AUTO_TEST_CASE( PerfTestSuite )
{
    unsigned maxIteration = 1000;
    BOOST_CHECK( elapsedCase1( maxIteration ) > elapsedCase2( maxIteration ) );
}

BOOST_AUTO_TEST_SUITE_END() // BranchPrediction
