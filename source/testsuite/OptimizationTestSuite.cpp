//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <array>

#include "tools/Benchmark.h"

// No reason to not apply these optimizations if applicable, minimum gain though
// Always check the assembly code generated (http://gcc.godbolt.org/)
// Always apply likely / unlikely in known branching (GCC)

// Generalities
// Prefer static linking and PDC
// - Prefer 64 - bit code, 32 - bit data
// - Prefer( 32 - bit ) array indexing to pointers
// - Prefer a[ i++ ] to a[ ++i ] (data dependencies)
// - Prefer regular memory access patterns
// - Minimize flow, avoid data dependencies
// Storage pecking order
// - Use static const for all immutables
//     Beware cache issues
// - Use stack for most variables
//     Hot
//     0 - cost addressing, like struct / class fields
// - Globals : aliasing issues
// - thread_local slowest, use local caching
//     1 instruction in Windows, Linux
//     3 - 4 in OSX

// Integrals
// - Prefer 32 - bit ints to all other sizes
//     64 bit may make some code 20x slower
//     8, 16 - bit computations use conversion to 32 bits and back
//     Use small ints in arrays
// - Prefer unsigned to signed
//     Except when converting to floating point
// - "Most numbers are small"

// Floating points
// - Double precision as fast as single precision
// - Extended precision just a bit slower
// - Do not mix the three
// - 1 - 2 FP addition / subtraction units
// - 1 - 2 FP multiplication / division units
// - SSE accelerates throughput for certain computation kernels
// - ints -> FPs cheap, FPs -> ints expensive

// Strength reduction
// - Don’t waste time replacing a /= 2 with a >>= 1
// - Speed hierarchy :
//      comparisons
//      ( u )int add, subtract, bitops, shift
//      FP add, sub( separate unit!)
//      ( u )int32 mul; FP mul
//      FP division, remainder
//      ( u )int division, remainder
BOOST_AUTO_TEST_SUITE( OptimizationTestSuite )

BOOST_AUTO_TEST_CASE( CompareTo0IfPossibleTest )
{
    // In typical processors testing against zero, or testing sign (negative/positive) are simple condition code checks. This means that instructions can be re-ordered to omit a test instruction
    auto test = 0;
    // Loop:
    // LOADCC r1, test // load test into register 1, and set condition codes
    // BCZS   Loop     // If zero was set, go to Loop
    if ( ! test )
        BOOST_CHECK( true );

    test = 1;
    // Loop:
    // LOAD   r1, test // load test into register 1
    // SUBT   r1, 1    // Subtract Test instruction, with destination suppressed
    // BCNE   Loop     // If not equal to 1, go to Loop
    if ( test == 1 )
        BOOST_CHECK( true );
}

BOOST_AUTO_TEST_CASE( StrengthReductionTest )
{
    auto digits10Division = []( uint64_t v )
    {
        uint32_t result = 0;
        do
        {
            ++result;
            v /= 10; // use integral division extensively
        } while ( v );
        return result;
    };

    // More comparisons and additions, fewer /=
    auto digits10LessDivision = []( uint64_t v )
    {
        uint32_t result = 1;
        for (;;)
        {
            if ( v < 10 ) return result;
            if ( v < 100 ) return result + 1;
            if ( v < 1000 ) return result + 2;
            if ( v < 10000 ) return result + 3;
            // Skip ahead by 4 orders of magnitude
            v /= 10000U;
            result += 4;
        }
    };

    auto test = [&] ( auto n )
    {
        double divisionT, lessDivisionT;
        std::tie( divisionT, lessDivisionT ) = tools::benchmark( n,
                                                                 [ & ]{ return digits10Division( n ); },
                                                                 [ & ]{ return digits10LessDivision( n ); } );

        BOOST_CHECK( divisionT >= lessDivisionT );
    };
    tools::run_test< int >( "division;less_division;", test, 100'000, 1'000'000, 10'000'000 );
}

// Always benchmark these refactoring
BOOST_AUTO_TEST_CASE( LoopRefactorizationTest )
{
    // ---- LOOP UNROLLING ----
    auto sum = 0;
    std::array< int , 1'000 > arr;
    //for ( auto i = 0; i < 1'000; ++i )
    //    sum += arr[i];

    // Increased code size, but execute fewer overhead instructions,
    // Might lost performance benefit after a while due to the added expense of fetchnig and decoding more isntructions
    int t1, t2, t3, t4;
    // Reduce data dependance on sum
    t1 = t2 = t3 = t4 = 0;
    for ( auto i = 0; i < 1'000; i += 4 )
    {
        t1 += arr[ i ];
        t2 += arr[ i + 1 ];
        t3 += arr[ i + 2 ];
        t4 += arr[ i + 3 ];
    }
    sum = t1 + t2 + t3 + t4;

    // ---- LOOP INTERCHANGING ----
    auto doOdd = [] ( auto i ) {};
    auto doEven = [] ( auto i ) {};
    //for ( auto i = 0; i < 1'000; ++i )
    //{
    //    if (i & 1)
    //        doOdd(i);
    //    else
    //        doEven(i);
    //}

    // Eliminating branching when possible, really important if branch missprediction is high
    for ( auto i = 0; i < 1'000; i += 2 )
    {
        doOdd(i);
        doEven(i);
    }

    int a[4];
    int b[4][4]= { { 2, 6 }, { 6, 7 }, { 3, 4 }, { 5, 0 } };
    //for ( auto i = 0; i < 4; ++i )
    //{
    //    a[i] = 0;
    //    for ( auto j = 0; j < 4; ++j )
    //        a[i] += b[j][i];
    //}

    // Favorize vectorization, and more cache friendly
    for ( auto i = 0; i < 4; ++i )
        a[i] = 0;

    for ( auto j = 0; j < 4; ++j )
        for ( auto i = 0; i < 4; ++i )
            a[i] += b[j][i];

    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // OptimizationTestSuite
