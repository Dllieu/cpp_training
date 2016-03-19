//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <csignal>

BOOST_AUTO_TEST_SUITE( ExceptionTestSuite )

// Modern C++ implementations reduce the overhead of using exceptions to a few percent (3%) and that’s compared to no error handling. Writing code with error-return codes and tests is not free either.
// As a rule of thumb, exception handling is extremely cheap when you don’t throw an exception. It costs nothing on some implementations.
// All the cost is incurred when you throw an exception: that is, “normal code” is faster than code using error-return codes and tests. You incur cost only when you have an error.

// The old model :
//   - Visual Studio (vc100 only?), when building a 32-bit target, will register a handler in every function that has local variables with non-trivial destructor. Basically, it sets up a try/finally handler
// The main model used today for exceptions (Itanium ABI, VC++ 64 bits) is the Zero-Cost model exceptions.
// The idea is that instead of losing time by setting up a guard and explicitly checking for the presence of exceptions everywhere, the compiler generates a side table that maps any point that may throw an exception (Program Counter) to the a list of handlers.
// When an exception is thrown, this list is consulted to pick the right handler (if any) and stack is unwound.
// Compared to the typical if (error) strategy:
//   - the Zero-Cost model, as the name implies, is free when no exception occurs
//   - it cost around 10x/20x an if when an exception does occur
//   - The cost, however, is not trivial to measure:
// The side-table is generally cold, and thus fetching it from memory takes a long time
// Determining the right handler involves RTTI: many RTTI descriptors to fetch, scattered around memory, and complex operations to run (basically a dynamic_cast test for each handler)
// so, mostly cache misses, and thus not trivial compared to pure CPU code.
// So, yes, exceptions are slow on the exceptional path, but they are otherwise quicker than explicit checks (if strategy) in general.

namespace
{
    /*
    An object that is partially constructed or partially destroyed will have destructors executed for all of its fully constructed base classes and non-variant members,
    that is, for subobjects for which the principal constructor has completed execution and the destructor has not yet begun execution. Similarly,
    if the non-delegating constructor for an object has completed execution and a delegating constructor for that object exits with an exception,
    the object’s destructor will be invoked. If the object was allocated in a new-expression, the matching deallocation function, if any, is called to free the storage occupied by the object


    If a constructor throws an exception, the corresponding destructor is not called
    The memory will be automatically freed before the exception propagates (meaning if new Foo throw, Foo won't leak by itself, but it's member could)

    When an exception is thrown, the stack will be unwound, which means all objects get their destructor's called as the code gets to the closest corresponding catch clause
    */
    class Foo
    {
    public:
        Foo( int i )
            try : x( ++i ) // not undefined because ',' make each statements independents
                , y( ++i )
                , z( std::make_unique< int >(++i) ) // during the stack unwinding, in case of a throw, this won't leak
        {
            // constructor body
        }
        catch (...)
        {
            // Handle exception
            // either throw an exception, or init Foo's member with safe values
        }

    private:
        int                     x;
        int                     y;
        std::unique_ptr<int>    z;
    };
}

BOOST_AUTO_TEST_CASE( InitListTest )
{
    // If the object was allocated in a new-expression, the matching deallocation function, if any, is called to free the storage occupied by the object
    // Exception thrown from a constructor invoked by new does not cause memory leak
    Foo* f = new Foo( 2 ); // if Foo throw, no memory is leaked
    delete f;
}

namespace
{
    struct A
    {
        ~A()
        {
            BOOST_CHECK( std::uncaught_exception() ); // can do fancy stuff like if ( std::uncaught_exception() ) ...

            // Returns: An exception_ptr object that refers to the currently handled exception (15.3) or a copy of the currently handled exception, or a null exception_ptr object if no exception is being handled.
            // The referenced object shall remain valid at least as long as there is an exception_ptr object that refers to it.
            // std::exception_ptr   current_exception() noexcept;
            BOOST_CHECK( ! std::current_exception() );
        }
    };
}

BOOST_AUTO_TEST_CASE( CurrentAdnUncaughtExceptionTest )
{
    try
    {
        A a;
        throw std::runtime_error( "Let's throw" );
        // at this point the exception is uncaught
    }
    catch (...)
    {
        // Exception is now caught
        BOOST_CHECK( ! std::uncaught_exception() );
        BOOST_CHECK( std::current_exception() );
    }
}

BOOST_AUTO_TEST_CASE( HandlingSignalTest )
{
    // SIGTERM  termination request, sent to the program
    // SIGSEGV  invalid memory access (segmentation fault)
    // SIGINT   external interrupt, usually initiated by the user
    // SIGILL   invalid program image, such as invalid instruction
    // SIGABRT  abnormal termination condition, as is e.g. initiated by std::abort()
    // SIGFPE   erroneous arithmetic operation such as divide by zero
    auto handler = std::signal( SIGSEGV, []( int signal ) { BOOST_CHECK( true ); } );

    // Real signal will be handled but will still terminate the runtime execution
    std::raise( SIGSEGV ); // *(int*) 0 = 0; // memory access violation
}

BOOST_AUTO_TEST_SUITE_END() // ExceptionTestSuite
