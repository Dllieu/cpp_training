//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( SmartPointer )

namespace
{
    struct A
    {
        A() {}
        A( const std::string& s ) : s( s ) {}
        std::string s;
    };

    A*      legacyMakeA( const std::string s ) { return new A( s ); }
}

// always use make shared_ptr : http://herbsutter.com/2013/05/29/gotw-89-solution-smart-pointers/
// only case you can't : - shared ptr need to be made from a raw pointer (legacy code)
//                       - or if you need custom deletor
BOOST_AUTO_TEST_CASE( MakeSmartPointer )
{
    std::unique_ptr< A > legacy( legacyMakeA( "legacy" ) );
    auto goodWay = std::make_shared< A >( "good way" ); // make_unique in C++14

    std::string stringDeleted;
    {
        std::unique_ptr< A, std::function< void ( A*) > > customDeleter( new A( "custom deleter" ), [ &stringDeleted ]( A* a ) { stringDeleted = a->s; delete a; } );
    }
    BOOST_CHECK( stringDeleted == "custom deleter" );
}

namespace
{
    void    takeOwnership( std::unique_ptr< A > a )
    {
        BOOST_CHECK( a );
    }
}

BOOST_AUTO_TEST_CASE( UniquePtr )
{
    std::unique_ptr< A > legacy( legacyMakeA( "legacy" ) );
    takeOwnership( std::move( legacy ) );
}

BOOST_AUTO_TEST_CASE( Array )
{
    {
        // can handle either delete / or delete []
        std::unique_ptr< A[] > arrayUnique( new A[2] );
    }
    {
        // shared need the custom deleter
        std::shared_ptr< A /*can't give A[] type*/ > sp( new A[2], []( A* a ) { delete[] a; } );
    }
}

BOOST_AUTO_TEST_CASE( WeakPtr )
{
    {
        std::shared_ptr< A > shared = std::make_shared< A >();
        {
            std::weak_ptr< A > weak = shared;
            std::shared_ptr< A > other_shared = weak.lock();
            BOOST_CHECK( other_shared );
        }
        BOOST_CHECK( shared );
    }
    {
        std::weak_ptr< A > weak;
        {
            std::shared_ptr< A > scoped_shared = std::make_shared< A >();
            weak = scoped_shared;

            BOOST_CHECK( weak.lock() );
        }
        BOOST_CHECK( ! weak.lock() );
    }
}

BOOST_AUTO_TEST_SUITE_END() // SmartPointer
