//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Feature )

BOOST_AUTO_TEST_CASE( DeclTypeTest )
{
    {
        int a = 0;
        // if parenthesis expression
        //   type: T& if lvalue
        //   type: T&& if rvalue
        /* int& */ decltype( ( a ) ) b = a;
        ++b;
        BOOST_CHECK( a == b && b == 1 );
    }
    {
        int a = 0;
        // if not a parenthesis expression
        //   type: T
        /* int */ decltype( a ) b = a;
        ++b;
        BOOST_CHECK( a + 1 == b && b == 1 );
    }
}

namespace
{
    struct alignas( 16 ) A
    {
        int     a;
    };
    static_assert( sizeof( A ) == 16 && alignof( A ) == 16, "not aligned" );

    struct alignas( 16 ) B
    {
        char    b[17];
    };
    static_assert( sizeof( B ) == 32 && alignof( B ) == 16, "not aligned" );


    //struct alignas(1) C // Alignment specifier is less than actual alignment (4) (vc140 limitation?)
    #pragma pack(1)
    struct C
    {
        char    a;
        int     b;
        char    c;
    };
    static_assert( sizeof( C ) == 6, "not aligned" );

    #pragma pack() // reset default alignment
    struct D
    {
        char    a;
        int     b;
        char    c;
    };
    static_assert( sizeof( D ) == 12, "not aligned" );
}

BOOST_AUTO_TEST_SUITE_END() // Feature
