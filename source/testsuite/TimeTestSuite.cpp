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

#include <chrono>
#include <time.h>
#include <stdlib.h>

BOOST_AUTO_TEST_SUITE( TimeTestSuite )

BOOST_AUTO_TEST_CASE( SynchronousTimerTest )
{
    boost::asio::io_service io;
    boost::asio::deadline_timer t( io, boost::posix_time::seconds( 1 ) ); // start timer after 1sec
    auto count = 5; // number of time we will recursively call timerTicker

    // Recursive lambda
    std::function< void( const boost::system::error_code& ) > timerTicker = [ &timerTicker, &t, &count ] ( const boost::system::error_code& /*e*/ )
    {
        if ( count-- > 0 )
        {
            std::cout << ".";
            t.expires_at( t.expires_at() + boost::posix_time::milliseconds( 500 ) );
            t.async_wait( timerTicker );
        }
        else
        {
            std::cout << std::endl;
            BOOST_CHECK( true );
        }
    };

    t.async_wait( timerTicker );
    io.run();
}

namespace
{
    const constexpr char* ENV_TIMEZONE = "TZ";

    // East is positive, West is negative
    // TZ = zone[ -]offset[ dst[ offset ][ , start[ / time ], end[ / time ] ] ]
    // Linux also accept user friendly format, i.e. "Asia/Kolkata" (see tzselect)
    std::vector< const char * > TIMEZONES{
        "EST5EDT,M3.2.0/2,M11.1.0/2",   // U.S. Eastern Standard Time (5 hours west from Greenwich, daylight saving starts on the second Sunday in March at 2 AM and ends on the first Sunday in November at 2 AM)
        "GMT0",                         // Greenwich Mean Time
        "CET-1CEST,M3.5.0/2,M10.5.0/3", // Central European Time (1 hour east from Greenwich, daylight starts last Sunday in March at 2AM and ends on last sunday in October at 3AM)
        "IST-5:30",                     // India
        "UTC-8",                        // Hong Kong
    };

    void    display_time()
    {
        // init global variable used by _get_timezone / _get_tzname / _get_daylight
        // values it depends to:
        //    * init depending of the values found in ENV_TIMEZONE
        //    * if empty use the information provided by the OS (e.g. the one in Date/Time for windows (set in some registry))
        //    * if can't retrieve from the OS, default to PST8PDT (Pacific Time Zone)
        _tzset();

        // _get_timezone / _get_tzname / _get_daylight are just windows safe version for accessing global variables
        long timezone;
        _get_timezone( &timezone );

        size_t length;
        std::array< char, 100 > tzname;
        _get_tzname( &length, tzname.data(), tzname.size(), 0 );

        int daylight;
        _get_daylight( &daylight );

        BOOST_REQUIRE( daylight == 0 || daylight == 1 );

        auto timezoneMinute = timezone / 60;
        auto minute = std::abs( timezoneMinute ) % 60;
        BOOST_REQUIRE( minute % 15 == 0 );

        auto hour = timezoneMinute / 60;

        std::cout << tzname.data() << " "
                  << std::setfill(' ') << std::setw( 3 ) << hour
                  << ":" << std::setfill( '0' ) << std::setw( 2 ) << minute;

        auto now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        std::array< char, 100 > buffer;
        BOOST_REQUIRE( ctime_s( buffer.data(), buffer.size(), &now ) == 0 );

        if ( daylight != 0 )
            std::cout << " (daylight saving)";
        std::cout << " : " << buffer.data();
    }
}

BOOST_AUTO_TEST_CASE( TimezoneSystemCallTest )
{
    size_t length;
    std::array< char, 100 > buffer;
    BOOST_CHECK( getenv_s( &length, buffer.data(), buffer.size(), ENV_TIMEZONE ) == 0 );
    display_time();
    std::cout << "-----" << std::endl;

    for ( const auto& tz : TIMEZONES )
    {
        BOOST_REQUIRE( _putenv_s( ENV_TIMEZONE, tz ) == 0 );
        display_time();
    }

    // get back to original timezone for the remaining tests
    BOOST_REQUIRE( _putenv_s( ENV_TIMEZONE, buffer.data() ) == 0 );
}

BOOST_AUTO_TEST_SUITE_END() // TimeTestSuite
