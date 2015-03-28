//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Byte )

namespace
{
    unsigned    add( unsigned a, unsigned b )
    {
        /* e.g.
            a = 9        0b1001
            b = 8        0b1000
            --
            carry = 16   0b1000
            a = 1        0b0001
            b = 16      0b10000
            --
            carry = 0       0b0
            a = 17      0b10001
            b = 0           0b0
        */
        while ( b )
        {
            unsigned carry = a & b;

            a ^= b;
            b = carry << 1;
        }
        return a;
    }
}


BOOST_AUTO_TEST_CASE( AddTest )
{
    BOOST_CHECK( 9 + 8 == add( 9, 8 ) );
}

namespace
{
    unsigned    multiply( unsigned a, unsigned b )
    {
        unsigned    result = 0;

        while ( b )
        {
            if ( b & 1 )
                result += a;

            a <<= 1;
            b >>= 1;
        }
        return result;
    }
}

BOOST_AUTO_TEST_CASE( MultiplyTest )
{
    BOOST_CHECK( 9 * 8 == multiply( 9, 8 ) );
}

namespace
{
    unsigned    divide( unsigned a, unsigned b )
    {
        // ???

        int quotient = 0;
        while ( a >= b )
        {
            a -= b; // could use a pseudo opti with a -= b * n and quotient *= n until (b * n < a) (if not the case --n, else ++n)
            ++quotient;
        }
        return quotient;
    }
}


BOOST_AUTO_TEST_CASE( DivideTest )
{
    BOOST_CHECK( 53 / 6 == divide( 53, 6 ) );
}

namespace
{
    bool    isPowerOf2( int a )
    {
        // must be 0b10 or 0b100 or 0b1000 or 0b10000 etc...
        return ( a & ( a - 1 ) ) == 0;
    }
}

BOOST_AUTO_TEST_CASE( PowTest )
{
    BOOST_CHECK( isPowerOf2( 4 ) && ! isPowerOf2( 5 ) );
}

BOOST_AUTO_TEST_SUITE_END() // Byte
