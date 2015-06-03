//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "tools/Timer.h"

// Inheritance in C++ has served two distinct purposes:
// 
// - Mixins (adding new, drop-in behavior to a class, without duplicating code).
//      In this scenario, the base class has little meaning of its own -- its purpose is to support the new behavior, and not to be used as a common base class among all the subclasses.
// 
// - Polymorphism (extending already-declared behavior in the base class).
//      In this scenario, the base class provides a common interface for all subclasses, yada yada.
// 
// CRTP is generally used for the first purpose, and virtual is used for the second.
// 
// Sometimes, you can achieve the same thing with both -- and the difference is only in whether the "polymorphism" is static (at compile-time) or dynamic (at run-time).
// If you don't need run-time polymorphism then you generally go with CRTP because it's usually faster, as the compiler can see what's going on at compile time.

BOOST_AUTO_TEST_SUITE( CRTP )

namespace
{
    struct VirtualBase
    {
        virtual void Foo() = 0;
    };

    struct VirtualDerived : public VirtualBase
    {
        void Foo() final {}
    };

    // https://isocpp.org/wiki/faq/virtual-functions
    // Avoid the cost of virtual function while retaining the hierarchical benefit
    // Two levels of runtime indirection saved (virtual function pointer + virtual function table)
    // static_cast calculations can be performed at compile time - no runtime cost
    // Possibly saves on having any virtual function pointer and vtable - saving space (albeit minimal savings)
    // The base class doesn't actually need to define the method - like a pure virtual function
    // You can call static methods and public members (both static and non-static)

    // Must always be inherited
    template < typename Derived >
    struct Base
    {
        void    Foo() {}

        void    callDerivedImplementation()
        {
            // static_cast can be performed at compile time
            // compile assert if this method is called and if the subclass doesn't implement this method (pseudo virtual pure)
            static_cast< Derived& >( *this ).toBeImplemented();
        }

        static void     Bar() {}
        static void     callBar()
        {
            Derived::Bar();
        }
    };

    struct Derived : public Base< Derived >
    {
        // This class uses base variant of Foo
        //void    Foo() {}

        void    toBeImplemented() {}

        static void    Bar() {}
    };
}

BOOST_AUTO_TEST_CASE( PerfTest )
{
    auto testRange = 5000000;
    {
        tools::Timer    t( "Virtual" );
        VirtualDerived  d;

        for ( auto i = 0; i < testRange; ++i )
            d.Foo();
    }
    {
        tools::Timer    t( "CRTP" );
        Derived         d;

        for ( auto i = 0; i < testRange; ++i )
            d.Foo();
    }

    Derived         d;
    d.callDerivedImplementation();
}

namespace
{
    template < typename T >
    class EqualComparable
    {
        friend bool     operator==( const T& a, const T& b ) { return a.equal( b ); }
        friend bool     operator!=( const T& a, const T& b ) { return ! a.equal( b ); }
    };

    class ValueType : public EqualComparable< ValueType >
    {
    public:
        ValueType( int id )
            : id_( id )
        {}

        bool    equal( const ValueType& v ) const
        {
            return id_ == v.id_;
        }

    private:
        int     id_;
    };
}

BOOST_AUTO_TEST_CASE( EqualityTest )
{
    ValueType v1(5), v2(6), v3(5);

    BOOST_CHECK( v1 != v2 );
    BOOST_CHECK( v1 == v3 );
}

namespace
{
    class Shape
    {
    public:
        virtual ~Shape() {}
        virtual Shape*  clone() const = 0;
    };

    template < typename Derived >
    class ShapeCRTP : public Shape
    {
    public:
        virtual Shape*  clone() const override
        {
            // default copy constructor
            return new Derived( static_cast< const Derived& >( *this ) );
        }
    };
}

#define DERIVE_SHAPE_CRTP( TYPE ) class TYPE: public ShapeCRTP< TYPE > {};

DERIVE_SHAPE_CRTP( Square )
DERIVE_SHAPE_CRTP( Circle )

#undef DERIVE_SHAPE_CRTP

BOOST_AUTO_TEST_CASE( PolymorphicCopyConstructionTest )
{
    Square b;
    delete b.clone();
}

BOOST_AUTO_TEST_SUITE_END() // CRTP
