//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <iostream>
#include "tools/ScopeGuard.h"

BOOST_AUTO_TEST_SUITE( ScopeGuardTestSuite )

BOOST_AUTO_TEST_CASE( ScopeExitTest )
{
    SCOPE_EXIT{ BOOST_CHECK( true ); };
}

BOOST_AUTO_TEST_CASE( ScopeSuccessTest )
{
    SCOPE_SUCCESS{ BOOST_CHECK( true ); };
}

namespace
{
    void    willThrow()
    {
        SCOPE_SUCCESS{ BOOST_CHECK( false ); }; // won't be called
        SCOPE_FAIL{ BOOST_CHECK( true ); };

        throw std::invalid_argument( "" );
    }
}

BOOST_AUTO_TEST_CASE( ScopeFailTest )
{
    BOOST_CHECK_THROW( willThrow(), std::invalid_argument );
}

BOOST_AUTO_TEST_SUITE_END() // ScopeGuardTestSuite
