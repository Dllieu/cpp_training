//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <sstream>
#include <iostream>
#include <array>
#include <tuple>

#include "tools/Timer.h"


BOOST_AUTO_TEST_SUITE( MetaProg )

namespace
{
    struct FooA { void method() const { BOOST_CHECK( true ); } };
    struct FooB { void method() const { BOOST_CHECK( true ); } };
    struct FooC { void method() const { BOOST_CHECK( true ); } };

    template<class F, class... Ts>
    void    for_all( F&& f, Ts&&... ts )
    {
        using swallow = int[];
        (void)swallow{ 0, ( f( std::forward< Ts >( ts ) ), 0 )... }; // non-recursive call
    }
}

BOOST_AUTO_TEST_CASE( ForAllTest )
{
    FooA a;
    FooB b;
    FooC c;

    for_all( [] ( auto& o ) { o.method(); }, a, b, c );
}

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

BOOST_AUTO_TEST_CASE( FactorialTest )
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

BOOST_AUTO_TEST_CASE( FibonacciTest )
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

namespace
{
    template < size_t N, size_t c > 
    struct IsPrimeImpl
    { 
        using type = typename boost::mpl::if_< boost::mpl::bool_< ( c * c > N ) >,
                                               boost::mpl::true_,
                                               typename boost::mpl::if_< boost::mpl::bool_< ( N % c == 0 ) >,
                                                                         boost::mpl::false_,
                                                                         IsPrimeImpl< N, c + 1 >
                                                                       >::type
                                             >::type;
        enum { value = type::value };
    };
 
    template<size_t N> 
    struct IsPrime
    {
      enum { value = IsPrimeImpl< N, 2 >::value };
    };
 
    template <>
    struct IsPrime< 0 >
    {
      enum { value = 0 };
    };
 
    template <>
    struct IsPrime< 1 >
    {
      enum { value = 0 };
    };
}

BOOST_AUTO_TEST_CASE( PrimeTest )
{
    BOOST_CHECK( ! IsPrime< 27 >::value );
    BOOST_CHECK( IsPrime< 29 >::value );
    BOOST_CHECK( ! IsPrime< 33 >::value );
}


namespace
{
    // Functions shall not have a return type of type array or function, although they may have a return type of type pointer or reference to such things
    using yes = char (&)[1];
    using no = char (&)[2];

    template < typename B, typename D >
    struct Host
    {
      operator B*   () const; // const to make this cast less important -> ensure that D doesn't derive from B before calling this cast (while calling no check)
      operator D*   ();
    };

    template < typename B, typename D >
    struct is_base_of
    {
      template < typename T > 
      static yes    check( D*, T ); // check( D* ( Host< B, D > ), int );

      // if D derive from B -> ( B* ( Host< B, D > ) ) would be ambigous because both cast operator would work -> the template version is chosen (yes check)
      // if D not derive from B -> not ambigous anymore since only 1 operator would work -> this version is chosen
      static no     check( B*, int );

      static const bool value = sizeof( check( Host< B, D >(), int() ) ) == sizeof( yes );
    };

    class Base {};
    class Derived : private Base {};
}

// http://stackoverflow.com/questions/2910979/how-is-base-of-works
BOOST_AUTO_TEST_CASE( IsBaseOfTest )
{
    BOOST_CHECK( ( is_base_of< Base, Derived >::value ) );
    BOOST_CHECK( ! ( is_base_of< Derived, Base >::value ) );
}

/*
typename before 'Foo' won't compile (until C++17):
 - class keyword is forced when dealing with template template parameters

Stroustrup originally used class to specify types in templates to avoid introducing a new keyword.
Some in the committee worried that this overloading of the keyword led to confusion. Later,
the committee introduced a new keyword typename to resolve syntactic ambiguity, and decided to let
it also be used to specify template types to reduce confusion, but for backward compatibility,
class kept its overloaded meaning

Except this, the difference is 'nothing', advice is to use 'class' if it's expected
typename if other types (int, char, ...) may be expected
*/
template <template <typename> class/*typename*/ Foo>
class MyContainer
{ /*...*/ };

namespace
{
    void    customPrint()
    {
        std::cout << std::endl;
    }

    template <typename T, typename ...Ts>
    void    customPrint( T& t, Ts&... ts )
    {
        std::cout << t;
        if (sizeof...(ts) != 0)
            std::cout << ' ';

        customPrint( ts... );
    }

    template <typename ...Ts>
    void forwardToCustomPrint( Ts&&... ts )
    {
        customPrint( ts... );
    }

    template <typename ...Ts>
    bool forwardTest( Ts&&... ts )
    {
        forwardToCustomPrint( std::forward<Ts>( ts )... );
        return true;
    }
}

BOOST_AUTO_TEST_CASE( ForwardTest )
{
    BOOST_CHECK( forwardTest( 5, 98.21, 78 ) );
}

namespace
{
    template < size_t Dim, typename T, typename F >
    struct dot_class;

    template < typename T, typename F >
    struct dot_class< 1, T, F > {
        static inline T dot( const T* a, const T* b, F&& f )
        {
            return f( *a, *b );
        }
    };

    template < size_t Dim, typename T, typename F >
    struct dot_class {
        static inline T dot( const T* a, const T* b, F&& f )
        {
            return dot_class< Dim - 1, T, F >::dot( a + 1, b + 1, std::forward< F >( f ) ) + f( *a, *b );
        }
    };

    template < size_t Dim, typename T, typename F >
    inline T dot( const T* a, const T* b, F&& f )
    {
        return dot_class< Dim, T, F >::dot( a, b, std::forward< F >( f ) );
    }
}

BOOST_AUTO_TEST_CASE( GeneratedInlineCodeTest )
{
    std::array< int, 3 > a{ 3, 0, 1 },
                         b{ 2, 0, 1 };

    // will be replaced at translation unit by : a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
    BOOST_CHECK( dot< 3 >( a.data(), b.data(), [] ( int a, int b ) { return a * b; } ) == 7 );
}

namespace
{
    enum class None {};
    enum class Fizz {};
    enum class Buzz {};
    enum class FizzBuzz {};

    template < size_t N, typename = void >
    struct  FizzBuzzImpl { using type = None; };

    template <>
    struct  FizzBuzzImpl< 0, void > { using type = None; };

    template < size_t N >
    struct  FizzBuzzImpl< N, typename std::enable_if_t< N % 3 == 0 && N % 5 > >      { using type = Fizz; };

    template < size_t N >
    struct  FizzBuzzImpl< N, typename std::enable_if_t< N % 3      && N % 5 == 0 > > { using type = Buzz; };

    template < size_t N >
    struct  FizzBuzzImpl< N, typename std::enable_if_t< N % 3 == 0 && N % 5 == 0 > > { using type = FizzBuzz; };

    template < size_t... Ns >
    struct  FizzBuzzGenerator { using type = std::tuple< typename FizzBuzzImpl< Ns >::type... >; };
}

BOOST_AUTO_TEST_CASE( FizzBuzzTest )
{
    static_assert( std::is_same< FizzBuzzGenerator< 0, 3, 5, 15, 4 >::type, std::tuple< None, Fizz, Buzz, FizzBuzz, None > >::value, "" );
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // MetaProg
