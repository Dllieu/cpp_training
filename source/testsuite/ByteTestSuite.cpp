//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <array>

BOOST_AUTO_TEST_SUITE( ByteTestSuite )

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

BOOST_AUTO_TEST_CASE( LongLongToCharArray )
{
    unsigned long long ll = 545178863445;
    std::array< unsigned char, sizeof( ll ) > buffer;

    for ( auto i = 0; i < sizeof( ll ); ++i )
        buffer[ i ] = ( ll >> i * 8 ) & 255;

    decltype( ll ) ll2 = 0;
    for ( auto i = ( int )sizeof( ll ) - 1; i >= 0; --i )
        ll2 = ( ll2 << 8 ) | buffer[ i ];

    BOOST_CHECK( ll == ll2 );
}

namespace
{
    // About Bit Fields...
    //
    // If the specified size of the bit field is greater than the size of its type, the value is limited by the type : a std::uint8_t b : 1000; would still hold values between 0 and 255. the extra bits become unused padding.
    // Because bit fields do not necessarily begin at the beginning of a byte, address of a bit field cannot be taken.Pointers and non - const references to bit fields are not possible.
    // When initializing a const reference from a bit field, a temporary is created( its type is the type of the bit field ), copy initialized with the value of the bit field, and the reference is bound to that temporary.
    // The type of a bit field can only be integral or enumeration type.
    // A bit field cannot be a static data member.
    // There are no bit field prvalues : lvalue - to - rvalue conversion always produces an object of the underlying type of the bit field.

    // Multiple adjacent bit fields are usually packed together to share and straddle the individual bytes
    struct BF0
    {
        // will usually occupy 2 bytes (ceil(13. / 8)):
        // 3 bits: value of b1
        // 2 bits: unused
        // 6 bits: value of b2
        // 2 bits: value of b3
        // 3 bits: unused
        unsigned char b1 : 3,
                         : 2,
                      b2 : 6,
                      b3 : 2;
    };
    static_assert( sizeof( BF0 ) == 2, "invalid size" );

    struct BF1
    {
        // will usually occupy 2 bytes:
        // 3 bits: value of b1
        // 5 bits: unused
        // 6 bits: value of b2
        // 2 bits: value of b3
        unsigned char b1 : 3;
        unsigned char : 0; // start a new byte
        unsigned char b2 : 6;
        unsigned char b3 : 2;
    };
    static_assert( sizeof( BF1 ) == 2, "invalid size" );

    struct BF2
    {
        // three-bit unsigned field,
        // allowed values are 0...7
        unsigned int b : 3;
    };
    static_assert( sizeof( BF2 ) == 4, "invalid size" );

    struct BF3
    {
        // three-bit unsigned field,
        // allowed values are 0...7
        unsigned int b : 3,
                       : 29; // == 32 bits, if 30 -> sizeof() == 8
    };
    static_assert( sizeof( BF3 ) == 4, "invalid size" );
}

BOOST_AUTO_TEST_SUITE_END() // ByteTestSuite
