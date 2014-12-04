//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/test/execution_monitor.hpp>

BOOST_AUTO_TEST_SUITE( BoostTestSuiteTricks )

namespace
{
    inline void letsAssert()
    {
        assert( false );
    }
}

BOOST_AUTO_TEST_CASE( RequireAssert )
{
    BOOST_MESSAGE( "Assert Required:" );
    BOOST_REQUIRE_THROW( letsAssert(), boost::execution_exception );
}

BOOST_AUTO_TEST_SUITE_END() // BoostTestSuiteTricks
