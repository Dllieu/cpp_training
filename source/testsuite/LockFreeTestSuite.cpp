//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <thread>
#include <atomic>
#include <iostream>

BOOST_AUTO_TEST_SUITE( LockFreeTestSuite )

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

        for ( volatile unsigned i = 0; i < 1000000; ++i )
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
        Node* oldHead = listHead;
        Node* newHead = new Node;
        newHead->value = value;
        newHead->next = oldHead;

        // Compares the contents of the atomic object's contained value with expected:
        //     - if true, it replaces the contained value with val (like store).
        //     - if false, it replaces expected with the contained value .
        while ( ! listHead.compare_exchange_weak( oldHead /* expected */, newHead /* new value */ ) )
            newHead->next = oldHead;
    }
}

BOOST_AUTO_TEST_CASE( CompareExchange )
{
    std::vector< std::thread > threads;

    // Spawn N threads who count to 1 Million
    for ( unsigned i = 0; i < 10; ++i )
        threads.push_back( std::thread( append, i ) );

    for ( auto& thread : threads )
        thread.join();

    unsigned size = 0;
    Node* it = listHead;
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
