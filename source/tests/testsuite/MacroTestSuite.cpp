//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Macro )

#define LOG_TYPE( type ) \
    BOOST_TEST_MESSAGE( #type << ": " << sizeof( type ) )

BOOST_AUTO_TEST_CASE( TypeTestSuite )
{
    LOG_TYPE( bool );
    LOG_TYPE( char );
    LOG_TYPE( short );
    LOG_TYPE( int );
    LOG_TYPE( long );
    LOG_TYPE( float );
    LOG_TYPE( double );
    LOG_TYPE( long double );
    LOG_TYPE( std::string );

    BOOST_CHECK( true );
}

#undef LOG_TYPE

BOOST_AUTO_TEST_SUITE_END() // MacroTestSuite
