//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( SIMDTestSuite )

// Intrinsics resemble assembly language except that they leave the actual register allocation, instruction scheduling, and addressing modes to the compiler.
// Except for explicit unaligned load and store, compiler assume that packed memory operands of instrinsics are properly aligned
BOOST_AUTO_TEST_CASE( asdasd )
{
    // TODO
}

BOOST_AUTO_TEST_SUITE_END() // SIMDTestSuite
