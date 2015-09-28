//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio.hpp>
#pragma warning( pop )

#include <thread>
#include <iostream>

namespace ba = boost::asio;

BOOST_AUTO_TEST_SUITE( BasicNetworkingTestSuite )

BOOST_AUTO_TEST_CASE( SynchronousTimerTest )
{
    ba::io_service io;
    ba::deadline_timer t( io, boost::posix_time::seconds( 2 ) );

    std::function< void ( const boost::system::error_code& ) > timerTicker = [ &timerTicker, &t ] ( const boost::system::error_code& /*e*/ )
        {
            static int count = 5;
            if ( count-- > 0 )
            {
                t.expires_at( t.expires_at() + boost::posix_time::seconds( 1 ) );
                t.async_wait( timerTicker );
            }
        };


    t.async_wait( timerTicker );

    io.run();
}

BOOST_AUTO_TEST_SUITE_END() // ! BasicNetworkingTestSuite
