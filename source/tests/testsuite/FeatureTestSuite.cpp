//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Feature )

BOOST_AUTO_TEST_CASE( DeclTypeTestSuite )
{
    {
        int a = 0;
        // if parenthesis expression
        //   type: T& if lvalue
        //   type: T&& if rvalue
        /* int& */ decltype( ( a ) ) b = a;
        ++b;
        BOOST_CHECK( a == b && b == 1 );
    }
    {
        int a = 0;
        // if not a parenthesis expression
        //   type: T
        /* int */ decltype( a ) b = a;
        ++b;
        BOOST_CHECK( a + 1 == b && b == 1 );
    }
}

BOOST_AUTO_TEST_SUITE_END() // Feature
