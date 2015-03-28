//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
// http://www.codeproject.com/Articles/153898/Yet-another-implementation-of-a-lock-free-circular
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <string>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <numeric>
#include <queue>
#include <unordered_map>

BOOST_AUTO_TEST_SUITE( ThreadingTestSuite )

BOOST_AUTO_TEST_CASE( ThreadGroupTest )
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
        std::recursive_mutex                                            mutex_;
        unsigned                                                        timeoutInSeconds_;
        unsigned                                                        numberOfContext_;
        std::chrono::time_point< std::chrono::high_resolution_clock >   clock_;

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


See http://blog.tsunanet.net/2010/11/how-long-does-it-take-to-make-context.html
*/
BOOST_AUTO_TEST_CASE( ThreadSwitchTest )
{
    ThreadSwitchEstimator   threadEstimator( 2 );

    std::thread     thread1( std::bind( &ThreadSwitchEstimator::startComputing, std::ref( threadEstimator ) ) );
    std::thread     thread2( std::bind( &ThreadSwitchEstimator::startComputing, std::ref( threadEstimator ) ) );

    thread1.join();
    thread2.join();
}

namespace
{
    template <typename IT, typename T>
    struct AccumulateBlock
    {
        void    operator()( IT first, IT last, T& t )
        {
            t = std::accumulate( first, last, t );
        }
    };

    template <typename IT, typename T>
    T   ParallelAccumulate( IT first, IT last, T init )
    {
        auto size = std::distance( first, last );
        if ( ! size )
            return init;

        auto taskMinPerThread = 25;

        using ul_t = unsigned long;
        ul_t maxThread = ( size - 1 ) / taskMinPerThread + 1;
        ul_t hardwareThreadAvailable = std::thread::hardware_concurrency();

        auto threadSize = std::min( hardwareThreadAvailable != 0 ? hardwareThreadAvailable : 2, maxThread );
        auto blockSize = size / threadSize;

        std::vector< T > results( threadSize );
        std::vector< std::thread > threads( threadSize - 1 ); // minus current thread

        auto blockStart = first;
        for ( ul_t i = 0; i < threads.size(); ++i )
        {
            auto blockEnd = blockStart;
            std::advance( blockEnd, blockSize );

            threads[i] = std::thread( [ blockStart, blockEnd, i, &results ] { AccumulateBlock< IT, T >()( blockStart, blockEnd, results[i] ); } );
            blockStart = blockEnd;
        }
        AccumulateBlock< IT, T >()( blockStart, last, results.back() );

        std::for_each( std::begin( threads ), std::end( threads ), []( std::thread& thread ) { thread.join(); } );
        return std::accumulate( std::begin( results ), std::end( results ), init );
    }
}

BOOST_AUTO_TEST_CASE( ParallelAccumulateTest )
{
    auto n = 100;
    std::vector< int > v;
    v.reserve( n );

    while ( n > 0 )
        v.push_back( n-- );

    BOOST_CHECK( ParallelAccumulate( std::begin( v ), std::end( v ), 0 ) == ( 1 + v.size() ) * ( v.size() / 2 ) );
}

namespace
{
    class MultipleReadSingleWrite
    {
    public:
        boost::optional< int >     getEntry( int key ) const
        {
            // several thread can acquire this mutex
            boost::shared_lock< boost::shared_mutex >   lock( sharedMutex_ );

            auto it = map_.find( key );
            return it == std::end( map_ ) ? boost::optional< int >() : it->second;
        }

        void            updateOrInsert( int key, int value )
        {
            // This lock have more priority than the shared one, it will still wait the shared to unlock first
            std::lock_guard< boost::shared_mutex >   lock( sharedMutex_ );

            map_[ key ] = value;
        }

    private:
        // Make sense when multiple read, single write and vice versa
        mutable boost::shared_mutex        sharedMutex_;

        std::unordered_map< int, int >     map_;
    };
}

BOOST_AUTO_TEST_CASE( ReadWriteSharedMutexTest )
{
    MultipleReadSingleWrite m;

    const auto nbThread = 20;
    std::vector< std::thread >  threads;
    for ( auto i = 0; i < nbThread; ++i )
        threads.emplace_back( std::thread( [ &m, &nbThread, i ]
                {
                    m.updateOrInsert( i, i );
                    for ( auto j = 0; j < nbThread; ++j )
                        m.getEntry( j );
                } ) );

    for ( auto i = 0; i < nbThread; ++i )
        threads[ i ].join();

    for ( auto i = 0; i < nbThread; ++i )
        BOOST_CHECK( m.getEntry( i ).is_initialized() );
}

BOOST_AUTO_TEST_CASE( ConditionVariableTest )
{
    std::queue< int >           q;
    std::condition_variable     conditionVariable;
    std::mutex                  mutex;

    const auto valueExpected = 5;
    std::thread workerThread( [ & ]
        {
            // std::unique_lock allow more freedom compared to lock_guard, such as modifying explicitly the ownership of the lock (internal state to keep this information,
            // which make it less efficient compared to lock_guard, only use unique_lock if needed)
            std::unique_lock< std::mutex > lock( mutex );
            // release lock while waiting to be notified, reown the lock when checking the state and keep the lock active if the state is valid
            conditionVariable.wait( lock, [ &q ] { return ! q.empty(); } ); // since we wait only once in this example, future is more adapted in this context
            auto tmp = q.front();
            q.pop();
            // No need to keep the lock, can process tmp directly
            lock.unlock();

            BOOST_CHECK( tmp == valueExpected );
        } );

    std::thread producerThread( [ & ]
        {
            std::lock_guard< std::mutex > lock( mutex );
            q.push( valueExpected );
            conditionVariable.notify_one();
        } );

    workerThread.join();
    producerThread.join();
}

namespace
{
    struct WrapperWithMutex
    {
        explicit WrapperWithMutex( int initValue ) : n( initValue ) {}

        int         n;
        std::mutex  mutex;
    };
}

BOOST_AUTO_TEST_CASE( MultipleLockTest )
{
    WrapperWithMutex w1( 1 );
    WrapperWithMutex w2( 4 );

    auto l = []( WrapperWithMutex& lhs, WrapperWithMutex& rhs, int n )
        {
            std::unique_lock< std::mutex > l1( lhs.mutex, std::defer_lock );
            std::unique_lock< std::mutex > l2( rhs.mutex, std::defer_lock );

            std::lock( l1, l2 );

            lhs.n += n;
            rhs.n *= n;
        };

    std::thread t1( l, std::ref( w1 ), std::ref( w2 ), 2 );
    std::thread t2( l, std::ref( w2 ), std::ref( w1 ), 10 );

    t1.join();
    t2.join();

    BOOST_CHECK( w1.n == 30 && w2.n == 18 );
}

BOOST_AUTO_TEST_SUITE_END() // ThreadingTestSuite
