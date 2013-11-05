#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

#include "tools/Timer.h"

BOOST_AUTO_TEST_SUITE( MetaProg )

namespace
{
    // Diff enum != static :
    // Enums aren't lvals, static member values are and if passed by reference the template will be instanciated
    // e.g. void f(const int&); f( Factorial<1>::value ); ( with static const unsigned value; )
    // undesired effect if you want to do pure compile ime calculation
    // but static can hold bigger type than enum (e.g. static const unsigned long long)
    template <unsigned long long N>
    struct Factorial
    {
        // const to be able to instantiate within the class
        static const unsigned long long value = N  * Factorial<N - 1>::value;
        //enum { value = N  * Factorial<N - 1>::value };
    };

    template <>
    struct Factorial< 0 >
    {
        static const unsigned long long value = 1;
        //enum { value = 1 };
    };
}

BOOST_AUTO_TEST_CASE( FactorialTestSuite )
{
    tools::Timer t;

    Factorial<20>::value;

    BOOST_CHECK( true );
}

namespace
{
    template <unsigned long long N>
    struct Fibonacci
    {
        static const unsigned long long value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
        static unsigned getValue( unsigned long long n )
        {
            if ( n == N )
                return value;
            return n < N ? Fibonacci<N - 1>::getValue( n )
                         : getValue( n - 1 ) + getValue( n - 2); // non optimal workaround
        }
    };

    template<>
    struct Fibonacci<1>
    {
        static const unsigned long long value = 1;
        static unsigned getValue( unsigned long long n ) { return value; }
    };

    template<>
    struct Fibonacci<0>
    {
        static const unsigned long long value = 1;
        static unsigned getValue( unsigned long long n ) { return value; }
    };

    unsigned long long  computeFibonacci( unsigned long long n )
    {
        if ( n == 0 || n == 1 )
            return 1;
        return computeFibonacci( n - 1 ) + computeFibonacci( n - 2 );
    }
}

BOOST_AUTO_TEST_CASE( FibonacciTestSuite )
{
    unsigned long long metaProgResult, basicResult;
    double metaProgElapsed, basicElapsed;

    {
        tools::Timer t;

        metaProgResult = Fibonacci<30>::getValue( 35 ); // 30 -> 35 will be computed at runtime
        metaProgElapsed = t.elapsed();
    }
    {
        tools::Timer t;

        basicResult = computeFibonacci( 35 );
        basicElapsed = t.elapsed();
    }

    BOOST_CHECK_EQUAL( metaProgResult, basicResult );
    BOOST_CHECK( metaProgElapsed < basicElapsed );
}

BOOST_AUTO_TEST_SUITE_END() // MetaProg
