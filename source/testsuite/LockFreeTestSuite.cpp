//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <thread>
#include <atomic>
#include <mutex>
#include <iostream>

#include "tools/Timer.h"

// http://preshing.com/20120612/an-introduction-to-lock-free-programming/
BOOST_AUTO_TEST_SUITE( LockFreeTestSuite )

namespace
{
    double      mutexLoop()
    {
        return tools::Timer::named_elapsed( "mutex", []
        {
            std::mutex m;
            auto j = 0;
            for ( auto i = 0; i < 1'000'000; ++i )
            {
                std::lock_guard< std::mutex > l( m );
                ++j;
            }
        });
    }

    double      atomicFlagLoop()
    {
        return tools::Timer::named_elapsed( "atomic_flag", []
        {
            // only type to be guaranteed to be lock free
            std::atomic_flag lock = ATOMIC_FLAG_INIT; // can be either set or clear (here we init with a clear state)
            auto j = 0;
            for ( auto i = 0; i < 1'000'000; ++i )
            {
                while ( lock.test_and_set( std::memory_order_acquire ) ) // acquire lock
                    ; // spin

                ++j;
                lock.clear(); // release lock
            }
        });
    }
}

BOOST_AUTO_TEST_CASE( AtomicFlagTest )
{
    // Locks actually suspend thread execution, freeing up cpu resources for other tasks, but incurring (possibly) in obvious context-switching overhead when stopping/restarting the thread.
    // On the contrary, threads attempting atomic operations don't wait and keep trying until success (so-called busy-waiting) (they have the option to suspend themselves though with std::this_thread::yield),
    //    so they don't incur in context-switching overhead, but neither free up cpu resources.
    // Summing up, in general atomic operations are faster if contention between threads is sufficiently low.
    // You should definitely do benchmarking as there's no other reliable method of knowing what's the lowest overhead between context-switching and busy-waiting.
    BOOST_CHECK( mutexLoop() > atomicFlagLoop() );
}

namespace
{
    // There are three separate issues that "atomic" types in C++11 address:
    //     1.     tearing: a read or write involves multiple bus cycles, and a thread switch occurs in the middle of the operation; this can produce incorrect values.
    //     2.     cache coherence: a write from one thread updates its processor's cache, but does not update global memory; a read from a different thread reads global memory,
    //            and doesn't see the updated value in the other processor's cache.
    //     3.     compiler optimization: the compiler shuffles the order of reads and writes under the assumption that the values are not accessed from another thread, resulting in chaos.
    // Using std::atomic<bool> ensures that all three of these issues are managed correctly. Not using std::atomic<bool> leaves you guessing, with, at best, non-portable code.
    std::atomic< bool >   ready( false );
    std::atomic< bool >   winner( false );

    void    count( unsigned id )
    {
        while ( ! ready )
            std::this_thread::yield(); // thread waits for other threads to advance without blocking

        for ( volatile unsigned i = 0; i < 1'000'000; ++i ) // useless loop so we precise volatile to force compiler to use it
            ;

        // Replaces the contained value by val and returns the value it had immediately before
        if ( ! winner.exchange( true ) )
            std::cout << "Thread #" << id << " won!" << std::endl;
    }
}

BOOST_AUTO_TEST_CASE( Exchange )
{
    std::vector< std::thread > threads;

    // Spawn N threads who count to 1 Million
    for ( unsigned i = 0; i < 10; ++i )
        threads.push_back( std::thread( count, i ) );

    ready = true;
    for ( auto& thread : threads )
        thread.join();

    BOOST_CHECK( true );
}

namespace
{
    struct Node
    {
        int     value;
        Node*   next;
    };

    std::atomic< Node* > listHead( nullptr );

    void    append( int value )
    {
        while ( ! ready )
            std::this_thread::yield(); // thread waits for other threads to advance without blocking

        Node* oldHead = listHead;
        Node* newHead = new Node { value, oldHead };

        // CAS idiom (compare and swap)
        // Atomically compares the object representation of listHead with the object representation of expected, as if by std::memcmp,
        // and if those are bitwise-equal, replaces the former with desired (performs read-modify-write operation).
        // Otherwise, loads the actual value stored in listHead into expected (performs load operation). Copying is performed as if by std::memcpy.
        // As CAS perform a load and store operation, we usually pass std::memory_order_seq_cst (acquire for the load, release for the store)
        while ( ! listHead.compare_exchange_weak( oldHead /* expected */, newHead /* desired */, std::memory_order_seq_cst /* default memory order */ ) )
            newHead->next = oldHead; // might need to update oldHead as another thread might update the current head
            
        // Main difference between weak and strong is that weak have spurious failure, that is return false for no "apparent reason" without performing the CAS
            
        // - Why doing exchange in a loop?
        // Usually, you want your work to be done before you move on, thus, you put compare_exchange_weak into a loop so that it tries to exchange until it succeeds (i.e., returns true).
        // Note that also compare_exchange_strong is often used in a loop. It does not fail due to spurious failure, but it does fail due to concurrent writes.
        // 
        // - Why to use weak instead of strong?
        // Quite easy: Spurious failure does not happen often, so it is no big performance hit. In constrast, tolerating such a failure allows
        // for a much more efficient implementation of the weak version (in comparison to strong) on some platforms: strong must always check for spurious failure and mask it. This is expensive.
        // Thus, weak is used because it is a lot faster than strong on some platforms
        // 
        // - When should you use weak and when strong?
        // The reference states hints when to use weak and when to use strong:
        //  When a compare-and-exchange is in a loop, the weak version will yield better performance on some platforms.
        //  When a weak compare-and-exchange would require a loop and a strong one would not, the strong one is preferable.
        // So the answer seems to be quite simple to remember: If you would have to introduce a loop only because of spurious failure, don't do it; use strong. If you have a loop anyway, then use weak.
    }
}

BOOST_AUTO_TEST_CASE( CompareExchange )
{
    std::vector< std::thread > threads;

    // Spawn N threads who count to 1 Million
    for ( unsigned i = 0; i < 10; ++i )
        threads.push_back( std::thread( append, i ) );

    ready = true;
    for ( auto& thread : threads )
        thread.join();

    unsigned size = 0;
    Node* it = listHead.load();
    while ( it )
    {
        std::cout << it->value;
        auto tmp = it;
        it = it->next;
        delete tmp;

        ++size;
    }
    std::cout << std::endl;

    BOOST_CHECK( size == 10 );
}

BOOST_AUTO_TEST_SUITE_END() // LockFreeTestSuite
