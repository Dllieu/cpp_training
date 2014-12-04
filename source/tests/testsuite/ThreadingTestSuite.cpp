//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
// http://www.codeproject.com/Articles/153898/Yet-another-implementation-of-a-lock-free-circular
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <future>

BOOST_AUTO_TEST_SUITE( Threading )

namespace
{
    unsigned    accumulate( unsigned from, unsigned to )
    {
        auto result = from;
        while ( from < to ) result += ++from;
        return result;
    }
}

BOOST_AUTO_TEST_CASE( FutureTestSuite )
{
    unsigned from = 0, to = 100000;
    std::future< unsigned > f = std::async( accumulate, from, to );
    BOOST_CHECK( f.get() /* join until result is not received */ == accumulate( from, to ) );
}

BOOST_AUTO_TEST_CASE( ThreadGroupTestSuite )
{
    boost::thread_group threadGroup;

    unsigned threadNumber = 10;
    for ( unsigned i = 0; i < threadNumber; ++i )
        threadGroup.add_thread( new boost::thread( [] ( unsigned threadId ) { std::cout << threadId << std::endl; },
                                                   i ) );
    threadGroup.join_all();
    BOOST_CHECK( true );
}

namespace
{
    class ThreadSwitchEstimator
    {
    public:
        ThreadSwitchEstimator( unsigned timeoutInSeconds )
            : timeoutInSeconds_( timeoutInSeconds )
            , numberOfContext_( 0 )
            , clock_( std::chrono::high_resolution_clock::now() )
        {}

        ~ThreadSwitchEstimator()
        {
            double elapsedTime = elapsed();
            std::cout << "elapsed " << elapsedTime << " secs" << std::endl
                      << "Thread Context Switch per sec = " << ( elapsedTime > 0.0 ? numberOfContext_ / elapsedTime : 0. )
                      << " ( " << numberOfContext_ << " / " << elapsedTime << " )" << std::endl;
        }

        void    startComputing()
        {
            for ( ;; )
            {
                std::lock_guard< std::recursive_mutex > lock( mutex_ );
                ++numberOfContext_;
                if ( timeoutInSeconds_ < elapsed() )
                    return;
            }
        }

    private:
        unsigned                                                        timeoutInSeconds_;
        unsigned                                                        numberOfContext_;
        std::chrono::time_point< std::chrono::high_resolution_clock >   clock_;

        std::recursive_mutex                                            mutex_;


        double  elapsed()
        {
            std::lock_guard< std::recursive_mutex > lock( mutex_ );
            return std::chrono::duration_cast< std::chrono::duration< double, std::ratio<1> > >( std::chrono::high_resolution_clock::now() - clock_ ).count();
        }
    };
}

/*
Difference between Thread context switch and Process context switch :

- The main distinction between a thread switch and a process switch is that during a thread switch,
the virtual memory space remains the same, while it does not during a process switch. Both types involve
handing control over to the operating system kernel to perform the context switch. The process of switching
in and out of the OS kernel along with the cost of switching out the registers is the largest fixed cost of performing a context switch.

- A more fuzzy cost is that a context switch messes with the processors cacheing mechanisms.
Basically, when you context switch, all of the memory addresses that the processor "remembers"
in it's cache effectively become useless. The one big distinction here is that when you change virtual memory spaces,
the processor's Translation Lookaside Buffer (TLB) or equivalent gets flushed making memory accesses much more expensive for a while.
This does not happen during a thread switch.

*/
BOOST_AUTO_TEST_CASE( ThreadSwitchTestSuite )
{
    ThreadSwitchEstimator   threadEstimator( 2 );

    std::thread     thread1( std::bind( &ThreadSwitchEstimator::startComputing, std::ref( threadEstimator ) ) );
    std::thread     thread2( std::bind( &ThreadSwitchEstimator::startComputing, std::ref( threadEstimator ) ) );

    thread1.join();
    thread2.join();
}

BOOST_AUTO_TEST_SUITE_END() // Threading
