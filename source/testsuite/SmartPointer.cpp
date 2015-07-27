//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <algorithm>

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
    auto goodWay = std::make_unique< A >( "good way" );

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

BOOST_AUTO_TEST_CASE( ArrayTest )
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

BOOST_AUTO_TEST_CASE( WeakPtrTest )
{
    {
        // shared_ptr are thread safe
        // Ptr to A + Ptr to control block (containing reference count / weak count / other data such as allocator, custom deleter, ...)
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

namespace
{
    class Widget : public std::enable_shared_from_this< Widget >
    {
    public:
        std::shared_ptr< Widget >   getShared()
        {
            return shared_from_this();
        }
    };
}

BOOST_AUTO_TEST_CASE( SharedFromThisTest )
{
    std::weak_ptr< Widget > weak;
    {
        std::shared_ptr< Widget > w = std::make_shared< Widget >();

        weak = w->getShared();
        BOOST_CHECK( weak.lock() );
    }
    BOOST_CHECK( ! weak.lock() );
}

namespace
{
    struct X
    {
        X() : i( 0 )
        {}

        int i;
    };

    struct Wrapper
    {
        double  d;
        X       x;
    };
}

BOOST_AUTO_TEST_CASE( SharedAliasConstructorTest )
{
    std::vector< std::shared_ptr< X > >     referenceHolder;

    {
        auto wrapper = std::make_shared< Wrapper >();

        // Increment / decrement refcount of wrapper, although value pointed might not be related at all
        referenceHolder.emplace_back( std::shared_ptr< X >( wrapper /*reference the shared_ptr alias*/, &wrapper->x/*value pointed, must be a pointer*/ ) );

        // If we used shared_ptr with no deletor : x would become a dangling pointer as soon as we go out of the scope of wrapper
        //referenceHolder.emplace_back( std::shared_ptr< X >( &wrapper->x, [] ( X* x ) { /* no deletion */ } ) ); // undefined behavior when out of scope of wrapper
    }

    std::for_each( std::begin( referenceHolder ), std::end( referenceHolder ), [] ( const std::shared_ptr< X >& x ) { BOOST_CHECK( x->i == 0 ); } );
}

BOOST_AUTO_TEST_SUITE_END() // SmartPointer
