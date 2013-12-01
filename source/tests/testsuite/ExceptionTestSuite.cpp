#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Exception )

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
                , z( new int(++i) ) // during the stack unwinding, in case of a throw, this won't leak
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

BOOST_AUTO_TEST_CASE( InitListTestSuite )
{
    // If the object was allocated in a new-expression, the matching deallocation function, if any, is called to free the storage occupied by the object
    Foo* f = new Foo( 2 ); // if Foo throw, no memory is leaked
    delete f;
}

BOOST_AUTO_TEST_SUITE_END() // Exception
