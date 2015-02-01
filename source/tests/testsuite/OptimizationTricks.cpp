//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

// http://gcc.godbolt.org/
// No reason to not apply these optimizations if applicable, minimum gain though
BOOST_AUTO_TEST_SUITE( OptimizationTricks )

BOOST_AUTO_TEST_CASE( CompareTo0IfPossibleTestSuite )
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

BOOST_AUTO_TEST_SUITE_END() // OptimizationTricks
