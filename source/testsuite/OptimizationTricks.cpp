//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <array>

// http://gcc.godbolt.org/
// No reason to not apply these optimizations if applicable, minimum gain though
// Always check the assembly code generated
BOOST_AUTO_TEST_SUITE( OptimizationTricks )

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

BOOST_AUTO_TEST_SUITE_END() // OptimizationTricks
