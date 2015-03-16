//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <mutex>

BOOST_AUTO_TEST_SUITE( Singleton )

namespace
{
    // They are generally used as a global instance, why is that so bad? Because you hide the dependencies of your application in your code,
    //    instead of exposing them through the interfaces. Making something global to avoid passing it around is a code smell.
    // They violate the single responsibility principle: by virtue of the fact that they control their own creation and lifecycle.
    // They inherently cause code to be tightly coupled. This makes faking them out under test rather difficult in many cases.
    // They carry state around for the lifetime of the application. Another hit to testing since you can end up with a situation where tests
    //    need to be ordered which is a big no no for unit tests. Why? Because each unit test should be independent from the other.

    // C++ and the Perils of Double-Checked Locking http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
    class ThreadSafeSingleton
    {
    public:
        static ThreadSafeSingleton* instance()
        {
            ThreadSafeSingleton* memoryBarrier = singleton_;
            if ( memoryBarrier == nullptr )
            {
                mutex_.lock();
                memoryBarrier = singleton_;
                if ( memoryBarrier == nullptr )
                {
                    memoryBarrier = new ThreadSafeSingleton;
                    singleton_ = memoryBarrier;
                }
                mutex_.unlock();
            }
            return memoryBarrier;
        }

    private:
        static ThreadSafeSingleton* singleton_;
        static std::mutex           mutex_;
    };

    ThreadSafeSingleton*    ThreadSafeSingleton::singleton_ = 0;
    std::mutex              ThreadSafeSingleton::mutex_;
}

BOOST_AUTO_TEST_CASE( ThreadSafeSingletonTestSuite )
{
    BOOST_CHECK( ThreadSafeSingleton::instance() != nullptr );

    // silly delete for boost memory leak warning
    delete ThreadSafeSingleton::instance();
}

BOOST_AUTO_TEST_SUITE_END() // Singleton
