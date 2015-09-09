//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4996 ) // std::equal
#include <boost/test/unit_test.hpp>
#pragma warning( pop )

#include <string>
#include <cctype>


// Exceptional C++ : Herb Sutter
BOOST_AUTO_TEST_SUITE( ExceptionalCppTestSuite )

namespace
{
    struct ci_char_traits : public std::char_traits< char >
    {
        static bool     eq( char c1, char c2 )
        {
            return std::tolower( c1 ) == std::tolower( c2 );
        }

        static bool     lt( char c1, char c2 )
        {
            return std::tolower( c1 ) < std::tolower( c2 );
        }

        static int      compare( const char* s1, const char *s2, size_t n )
        {
            while ( n-- )
            {
                if ( lt( *s1, *s2 ) )
                    return -1;

                if ( !eq( *s1++, *s2++ ) )
                    return 1;
            }
            return 0;
        }

        static const char*  find( const char* s, size_t n, char c )
        {
            auto end = s + n;
            auto result = std::find_if( s, end, [ c ] ( char c2 ) { return eq( c, c2 ); } );
            return result == end ? nullptr : result;
        }
    };

    using ci_string = std::basic_string< char, ci_char_traits >;
}

// real case: prefer an explicit comparison method ( e.g. bool insensitive_comparison( s1, s2 ) )
BOOST_AUTO_TEST_CASE( CaseInsensitiveStringTest )
{
    std::string str = "abcde";
    BOOST_CHECK( str != "Abcde" );

    ci_string cis = "abcde";
    BOOST_CHECK( cis == "AbCdE" );
    BOOST_CHECK( cis == "abcde" );
    BOOST_CHECK( cis != "abcd" );
    BOOST_CHECK( cis.find( 'B' ) == 1 );
    BOOST_CHECK( cis.find( 'z' ) == -1 );

    BOOST_CHECK( std::strcmp( cis.c_str(), "abcde" ) == 0 );
    BOOST_CHECK( std::strcmp( cis.c_str(), "AbCdE" ) != 0 );
}

BOOST_AUTO_TEST_SUITE_END() // ! ExceptionalCppTestSuite
