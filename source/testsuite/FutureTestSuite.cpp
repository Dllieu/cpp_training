//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include <boost/test/unit_test.hpp>
#include <future>

BOOST_AUTO_TEST_SUITE( Future )

namespace
{
    unsigned    accumulate( unsigned from, unsigned to )
    {
        auto result = from;
        while ( from < to ) result += ++from;
        return result;
    }
}

BOOST_AUTO_TEST_CASE( AsyncTestSuite )
{
    unsigned from = 0, to = 100000;
    
    // std::launch::async will execute the function in a separate thread
    // std::launch::deferred will defer the evaluation on the first wait on the future (does not spawn another thread)
    // If both the std::launch::async and std::launch::deferred flags are set in policy, it is up to the implementation whether to perform asynchronous execution or lazy evaluation.
    // the result or the exception of the function is stored in a shared state, accessible through future
    std::future< unsigned > f = std::async( std::launch::async | std::launch::deferred, accumulate, from, to );
    BOOST_CHECK( f.get() /* join until result is not received in the shared state */ == accumulate( from, to ) );
}

BOOST_AUTO_TEST_SUITE_END() // Future
