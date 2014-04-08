#include <boost/test/unit_test.hpp>
#include <thread>
#include <atomic>

BOOST_AUTO_TEST_SUITE( LockFree )

namespace
{
    std::atomic< bool >   ready( false );
    std::atomic< bool >   winner( false );

    void    count( unsigned id )
    {
        while ( ! ready )
            std::this_thread::yield();

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

BOOST_AUTO_TEST_SUITE_END() // LockFree
