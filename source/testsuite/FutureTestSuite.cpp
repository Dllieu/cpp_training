//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include <boost/test/unit_test.hpp>
#include <future>
#include <iostream>
#include "threading\SpawnTask.h"

// - A future is an object that can retrieve a value from some provider object or function, properly synchronizing this access if in different threads.
// - "Valid" futures are future objects associated to a shared state, and are constructed by calling one of the following functions:
//      std::async
//      std::promise::get_future
//      std::packaged_task::get_future
// 
// - future objects are only useful when they are valid. Default-constructed future objects are not valid (unless move-assigned a valid future).
// - Calling future::get on a valid future blocks the thread until the provider makes the shared state ready (either by setting a value or an exception to it). This way, two threads can be synchronized by one waiting for the other to set a value.
// - The lifetime of the shared state lasts at least until the last object with which it is associated releases it or is destroyed. Therefore, if associated to a future, the shared state can survive the object from which it was obtained in the first place (if any).
BOOST_AUTO_TEST_SUITE( FutureTestSuite )

namespace
{
    // test purpose
    const int EXPECTED_VALUE = 10;

    unsigned    accumulate( unsigned from, unsigned to )
    {
        auto result = from;
        while ( from < to ) result += ++from;

        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
        return result;
    }
}

BOOST_AUTO_TEST_CASE( AsyncTest )
{
    unsigned from = 0, to = 10000;
    
    // First argument is an optional policy:
    // std::launch::async will execute the function in a separate thread
    // std::launch::deferred will defer the evaluation on the first wait on the future (does not spawn another thread)
    // If both the std::launch::async and std::launch::deferred flags are set in policy, it is up to the implementation whether to perform asynchronous execution or lazy evaluation.
    // the result or the exception of the function is stored in a shared state, accessible through future
    std::future< unsigned > f = std::async( std::launch::async, accumulate, from, to );
    std::chrono::milliseconds span( 100 );
    while ( f.wait_for( span ) == std::future_status::timeout ) // simulate a wait (wait for the shared state to be ready), but unlock the current thread every 100ms
        std::cout << '.' << std::endl;

    BOOST_CHECK( f.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready );
    BOOST_CHECK( f.get() /* join until result is not received in the shared state */ == accumulate( from, to ) );
}

BOOST_AUTO_TEST_CASE( PackagedTaskTest )
{
    // - A packaged_task wraps a callable element and allows its result to be retrieved asynchronously.
    // - It is similar to std::function, but transferring its result automatically to a future object.
    // - The object contains internally two elements:
    // A stored task, which is some callable object (such as a function pointer, pointer to member or function object) whose call signature shall take arguments of the types in Args... and return a value of type Ret.
    // A shared state, which is able to store the results of calling the stored task (of type Ret) and be accessed asynchronously through a future.
    // - The lifetime of the shared state lasts at least until the last object with which it is associated releases it or is destroyed.
    // Therefore it can survive the packaged_task object that obtained it in the first place if associated also to a future.

    // Do pretty much the same as std::async but can chose when the computation start
    std::packaged_task< int () > packagedTask( []
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
            return EXPECTED_VALUE;
        } );

    auto f = packagedTask.get_future();

    // Start the computation of the packagedTask in another thread
    std::thread t( std::move( packagedTask ) ); // std::packaged_task is movable

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    BOOST_CHECK( f.get() == EXPECTED_VALUE ); // if packagedTask is not started before the get, this will wait indefinitly

    t.join();
}

BOOST_AUTO_TEST_CASE( SpawnTaskTest )
{
    auto f = threading::spawnTask( []( int lhs, int rhs ) { std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) ); return lhs + rhs; }, EXPECTED_VALUE, EXPECTED_VALUE );
    BOOST_CHECK( f.get() == EXPECTED_VALUE * 2 );
}

BOOST_AUTO_TEST_CASE( PromiseTest )
{
    // - A promise is an object that can store a value of type T to be retrieved by a future object (possibly in another thread), offering a synchronization point.
    // - On construction, promise objects are associated to a new shared state on which they can store either a value of type T or an exception derived from std::exception.
    // - This shared state can be associated to a future object by calling member get_future. After the call, both objects share the same shared state:
    //    The promise object is the asynchronous provider and is expected to set a value for the shared state at some point.
    //    The future object is an asynchronous return object that can retrieve the value of the shared state, waiting for it to be ready, if necessary.
    // - The lifetime of the shared state lasts at least until the last object with which it is associated releases it or is destroyed. Therefore it can survive the promise object that obtained it in the first place if associated also to a future.
    std::promise< int > p;

    auto f = p.get_future();
    std::thread t( [ &f ] { BOOST_CHECK( f.get() == EXPECTED_VALUE ); } );

    std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );

    // Useless test
    if ( EXPECTED_VALUE == 0 )
        // Another way to store an exception in a future is to destroy the std::promise or std::packaged_task associated with the future without calling either of the set functions on the promise or invoking the packaged task
        p.set_exception( std::make_exception_ptr( std::logic_error( "this can't happen" ) ) );
    else
        p.set_value( EXPECTED_VALUE );

    t.join();
}

BOOST_AUTO_TEST_CASE( SharedFutureTest )
{
    std::promise< void >        readyPromise, p1, p2;
    // steal ownership (future -> shared_future) future always let the ownership to the shared_future
    // from this point it's thread safe to manipulate shared_future
    std::shared_future< void >  sharedFuture( readyPromise.get_future() ); 

    auto f1 = std::async( std::launch::async, [ &p1, sharedFuture ]()
        {
            p1.set_value();
            sharedFuture.wait(); // waits for the signal from readyPromise
        } );

    auto f2 = std::async( std::launch::async, [ &p2, sharedFuture ]()
        {
            p2.set_value();
            sharedFuture.wait(); // waits for the signal from readyPromise
        } );
 
    // wait for the threads to become ready
    p1.get_future().wait();
    p2.get_future().wait();

    // signal the threads to go
    readyPromise.set_value();

    BOOST_CHECK( true );
}

namespace
{
    // input: 5 3 4 6 1
    // pivot: 5
    // quickSort( 3 1 4 ) // order not preserved with partition
    // quickSort( 6 )
    template < typename T >
    std::list< T >  parallelQuickSort( std::list< T > input /* copy */ )
    {
        if ( input.empty() || input.size() == 1 )
            return input;

        std::list< T > result;
        // Transfers elements from 'input' into 'result', inserting them at 'std::begin( result )'. This effectively inserts those elements into the container and removes them from input, altering the sizes of both containers.
        result.splice( std::begin( result ), input, std::begin( input ) /* only transfer std::begin( input ) to result */ );
        const auto& pivot = result.front();

        // Reorders the elements in the range [first, last) in such a way that all elements for which the predicate p returns true precede the elements for which predicate p returns false.
        // Relative order of the elements is not preserved.
        // Return iterator to the first element of the second group
        auto partitionedChunk = std::partition( std::begin( input ), std::end( input ), [ &pivot ]( const T& t ) { return t < pivot; } ); // Not optimum

        std::list< T > unsortedLowerChunk;
        unsortedLowerChunk.splice( std::end( unsortedLowerChunk ), input, std::begin( input ), partitionedChunk );

        // Only call quickSort on one of the chunk as we can process the other chunk with the current thread
        // possibily start the sorting process of the upperChunk in another thread
        auto futureSortedUpperChunk = std::async( parallelQuickSort< T >, std::move( input ) ); // let the implem chose the policy to avoid oversubscription

        result.splice( std::begin( result ), parallelQuickSort( std::move( unsortedLowerChunk ) ) );
        result.splice( std::end( result ), futureSortedUpperChunk.get() );

        return result;
    }
}

BOOST_AUTO_TEST_CASE( ParallelQuickSortTest )
{
    // list to use splice
    std::list< int >  v { 5, 3, 6, 9, 4, 2, 8, 10 };

    auto isSorted = [ &v ] { return std::is_sorted( std::begin( v ), std::end( v ) ); };

    BOOST_CHECK( ! isSorted() );
    v = parallelQuickSort( v );
    BOOST_CHECK( isSorted() );
}

BOOST_AUTO_TEST_SUITE_END() // FutureTestSuite
