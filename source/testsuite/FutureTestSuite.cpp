//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#include <boost/test/unit_test.hpp>
#include <future>
#include <iostream>

// future is linked to a shared state (reads result from it)
// what create and modify the shared state are asynchronous provider: std::packaged_task / std::async / std::promise
BOOST_AUTO_TEST_SUITE( Future )

namespace
{
    unsigned    accumulate( unsigned from, unsigned to )
    {
        auto result = from;
        while ( from < to ) result += ++from;

        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
        return result;
    }
}

BOOST_AUTO_TEST_CASE( AsyncTestSuite )
{
    unsigned from = 0, to = 10000;
    
    // std::launch::async will execute the function in a separate thread
    // std::launch::deferred will defer the evaluation on the first wait on the future (does not spawn another thread)
    // If both the std::launch::async and std::launch::deferred flags are set in policy, it is up to the implementation whether to perform asynchronous execution or lazy evaluation.
    // the result or the exception of the function is stored in a shared state, accessible through future
    std::future< unsigned > f = std::async( std::launch::async, accumulate, from, to );
    std::chrono::milliseconds span( 100 );
    while ( f.wait_for( span ) == std::future_status::timeout ) // simulate a wait (wait for the shared state to be ready), but unlock the current thread every 100ms
        std::cout << '.' << std::endl;

    BOOST_CHECK( f.get() /* join until result is not received in the shared state */ == accumulate( from, to ) );
}

BOOST_AUTO_TEST_CASE( PackagedTaskTestSuite )
{
    // - A packaged_task wraps a callable element and allows its result to be retrieved asynchronously.
    // - It is similar to std::function, but transferring its result automatically to a future object.
    // - The object contains internally two elements:
    // A stored task, which is some callable object (such as a function pointer, pointer to member or function object) whose call signature shall take arguments of the types in Args... and return a value of type Ret.
    // A shared state, which is able to store the results of calling the stored task (of type Ret) and be accessed asynchronously through a future.
    // - The lifetime of the shared state lasts at least until the last object with which it is associated releases it or is destroyed.
    // Therefore it can survive the packaged_task object that obtained it in the first place if associated also to a future.

    // Do pretty much the same as std::async but can chose when the computation start
    const auto expectedValue = 10;
    std::packaged_task< int () > packagedTask( [ expectedValue ]
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
            return expectedValue;
        } );

    auto f = packagedTask.get_future();

    // Start the computation of the packagedTask in another thread
    std::thread t( std::move( packagedTask ) ); // std::packaged_task is movable

    std::chrono::milliseconds( 500 );
    BOOST_CHECK( f.get() == expectedValue ); // if packagedTask is not started before the get, this will wait indefinitly

    t.join();
}

BOOST_AUTO_TEST_CASE( PromiseTestSuite )
{
    // - A promise is an object that can store a value of type T to be retrieved by a future object (possibly in another thread), offering a synchronization point.
    // - On construction, promise objects are associated to a new shared state on which they can store either a value of type T or an exception derived from std::exception.
    // - This shared state can be associated to a future object by calling member get_future. After the call, both objects share the same shared state:
    //    The promise object is the asynchronous provider and is expected to set a value for the shared state at some point.
    //    The future object is an asynchronous return object that can retrieve the value of the shared state, waiting for it to be ready, if necessary.
    // - The lifetime of the shared state lasts at least until the last object with which it is associated releases it or is destroyed. Therefore it can survive the promise object that obtained it in the first place if associated also to a future.
    const auto expectedValue = 10;
    std::promise< int > p;

    auto f = p.get_future();
    std::thread t( [ &f, expectedValue ] { BOOST_CHECK( f.get() == expectedValue ); } );

    std::chrono::milliseconds( 200 );
    p.set_value( expectedValue );

    t.join();
}

BOOST_AUTO_TEST_SUITE_END() // Future
