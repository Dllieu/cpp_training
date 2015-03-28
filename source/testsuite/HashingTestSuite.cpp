//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "generic/HashCombine.h"

BOOST_AUTO_TEST_SUITE( Hashing )

BOOST_AUTO_TEST_CASE( HashCombineTest )
{
    BOOST_CHECK( generics::hashCombine(5) == std::hash<int>()(5) );
    BOOST_CHECK( generics::hashCombine(5, 3, 2) != 0 );
}

BOOST_AUTO_TEST_SUITE_END() // Hashing
