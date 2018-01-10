//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <mutex>

#include "generic/ThreadSafeSingleton.h"

BOOST_AUTO_TEST_SUITE( SingletonTestSuite )

// Why Singleton should be avoided
// They are generally used as a global instance, why is that so bad? Because you hide the dependencies of your application in your code,
//    instead of exposing them through the interfaces. Making something global to avoid passing it around is a code smell.
// They violate the single responsibility principle: by virtue of the fact that they control their own creation and lifecycle.
// They inherently cause code to be tightly coupled. This makes faking them out under test rather difficult in many cases.
// They carry state around for the lifetime of the application. Another hit to testing since you can end up with a situation where tests
//    need to be ordered which is a big no no for unit tests. Why? Because each unit test should be independent from the other.
namespace
{
    // C++ and the Perils of Double-Checked Locking http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
    template < typename T >
    class ThreadSafeDoubleCheckSingleton // deprecated, use std::call_once
    {
    public:
        static std::shared_ptr< T > instance()
        {
            static std::shared_ptr< T >     singleton;
            static std::mutex               mutex;

            auto memoryBarrier = singleton;
            if ( memoryBarrier == nullptr )
            {
                std::lock_guard< std::mutex > lock( mutex );
                memoryBarrier = singleton;
                if ( memoryBarrier == nullptr )
                {
                    memoryBarrier.reset( new T{} );
                    singleton = memoryBarrier;
                }
            }
            return memoryBarrier;
        }
    };
}

BOOST_AUTO_TEST_CASE( ThreadSafeSingletonTest )
{
    BOOST_CHECK( ThreadSafeDoubleCheckSingleton< int >::instance() != nullptr );

    designpattern::ThreadSafeSingleton< int >::instance() = 5;
    BOOST_CHECK( designpattern::ThreadSafeSingleton< int >::instance() == 5 );
}

BOOST_AUTO_TEST_SUITE_END() // SingletonTestSuite
