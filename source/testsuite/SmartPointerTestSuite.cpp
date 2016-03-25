//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include <algorithm>

BOOST_AUTO_TEST_SUITE( SmartPointerTestSuite )

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

BOOST_AUTO_TEST_CASE( MakeSmartPointerTest )
{
    // unique_ptr hold tuple< raw_ptr, deleter >, in case deleter == default_deleter, deleter is an empty class, with tuple empty class arg optimization, it will have 0 overhead on the size (even if emptyclass >= sizeof(char))
    // if you give a custom deleter, there will be a small size overhead, and also a slight overhead when calling the destructor (possible cache miss during the indirection)
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

BOOST_AUTO_TEST_CASE( UniquePtrTest )
{
    std::unique_ptr< A > legacy( legacyMakeA( "legacy" ) );
    takeOwnership( std::move( legacy ) );
}

BOOST_AUTO_TEST_CASE( ArrayTestTest )
{
    {
        // can handle either delete / or delete []
        std::unique_ptr< A[] > arrayUnique( new A[2] );
    }
    {
        // shared need the custom deleter
        std::shared_ptr< A /*can't give A[] type*/ > sp( new A[2], []( A* a ) { delete[] a; } );
    }
    BOOST_CHECK( true );
}

// About shared_ptr
// always use make shared_ptr for the creation : http://herbsutter.com/2013/05/29/gotw-89-solution-smart-pointers/
// only pass shared_ptr by copy IF IT MAKE SENSE (i.e. it's not free! (it will at least modify the shared counter twice (scope in/out)))
// only case you can't : - shared ptr need to be made from a raw pointer (legacy code)
//                       - or if you need custom deleter
//
// all reference counter are atomic, but not the underlying ptr
// shared_ptr have overhead in size, it hold at least (underlying ptr + control block):
//    - underlying ptr
//    - ptr to the underlying_ptr (no overhead if use make_shared, there will be only the underlying_ptr,
//                                 and it will be in the same block as the control block, also it will be only one allocation/deletion instead of two)
//    - reference count
//    - weak reference count
//    - allocator (no overhead if default)
//    - deleter (no overhead if default)
// It also hold some overhead as it contains virtual methods for the type erasure, and overhead for the synchronisation of the reference counter
BOOST_AUTO_TEST_CASE( WeakPtrTest )
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
        referenceHolder.emplace_back( wrapper /*reference the shared_ptr alias*/, &wrapper->x/*value pointed, must be a pointer*/ );

        // If we used shared_ptr with no deletor : x would become a dangling pointer as soon as we go out of the scope of wrapper
        //referenceHolder.emplace_back( std::shared_ptr< X >( &wrapper->x, [] ( X* x ) { /* no deletion */ } ) ); // undefined behavior when out of scope of wrapper
    }

    std::for_each( std::begin( referenceHolder ), std::end( referenceHolder ), [] ( const std::shared_ptr< X >& x ) { BOOST_CHECK( x->i == 0 ); } );
}

// You can store any data in a shared_ptr< void >, and still have the correct destructor called at the end, because the shared_ptr constructor is a function template,
// and will use the type of the actual object passed for creating the deleter by default
BOOST_AUTO_TEST_CASE( SharedPtrTypeErasureTest )
{
    std::shared_ptr< void > pX( new X );

    BOOST_CHECK( true );
}

namespace
{
    struct Base
    {
        virtual ~Base() {}
    };

    struct Derived final : public Base
    {
        virtual ~Derived() override final = default;
        void g() { BOOST_CHECK( true ); }
    };
}


// std::static_pointer_cast / std::dynamic_pointer_cast / std::const_pointer_cast / std::reinterpret_pointer_cast (see SyntaxSpecificityTestSuite for cast behaviour)
BOOST_AUTO_TEST_CASE( SmartPtrCastTest )
{
    std::weak_ptr<Derived> wd;

    {
        std::shared_ptr<Base> b = std::make_shared<Derived>();
        wd = std::static_pointer_cast<Derived>( b );

        BOOST_CHECK( !wd.expired() );
        wd.lock()->g();
    }

    // when converting weak_ptr<D> to weak_ptr<B>, the object might be dead (that's the whole point of weak_ptr), so you have to internally lock to shared_ptr before performing the pointer conversion
    std::weak_ptr<Base> wb = wd;
    BOOST_CHECK( wb.expired() );
}

BOOST_AUTO_TEST_SUITE_END() // SmartPointerTestSuite
