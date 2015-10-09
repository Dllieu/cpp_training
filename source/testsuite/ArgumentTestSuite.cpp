//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE( Argument )

// http://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers
namespace
{
    struct A {};

    template <typename T>
    bool f( T&& t ) // universal reference, accept either T& or T&&
    {
        return true;
    }

    template<typename T>
    bool f( const T&& T ) // means only rvalue reference (worst match than only T&&)
    {
        return false;
    }
}

BOOST_AUTO_TEST_CASE( UniversalReference )
{
    BOOST_CHECK( f( A() ) ); // rvalue

    A a;
    BOOST_CHECK( f( a ) ); // lvalue
}

namespace
{
    // A declared variable can be Zero Initialized, Value Initialized or Default Initialized.
    // The C++03 Standard 8.5/5 aptly defines each:
    // To zero-initialize an object of type T means:
    // - if T is a scalar type (3.9), the object is set to the value of 0 (zero) converted to T;
    // - if T is a non-union class type, each nonstatic data member and each base-class subobject
    // is zero-initialized;
    // - if T is a union type, the object's first named data member is zero-initialized;
    // - if T is an array type, each element is zero-initialized;
    // - if T is a reference type, no initialization is performed.
    // 
    // To default-initialize an object of type T means:
    // - if T is a non-POD class type (clause 9), the default constructor for T is called (and the initialization is ill-formed if T has no accessible default constructor);
    // - if T is an array type, each element is default-initialized;
    // - otherwise, the object is zero-initialized.
    // 
    // To value-initialize an object of type T means:
    // - if T is a class type (clause 9) with a user-declared constructor (12.1), then the default constructor for T is called (and the initialization is ill-formed if T has no accessible default constructor);
    // - if T is a non-union class type without a user-declared constructor, then every non-static data member and base-class component of T is value-initialized;
    // - if T is an array type, then each element is value-initialized;
    // - otherwise, the object is zero-initialized

    // A POD type is a C++ type that has an equivalent in C, and that uses the same rules as C uses for initialization, copying, layout, and addressing
    // To make sure the other rules match, the C++ version must not have virtual functions, base classes, non-static members that are private or protected, or a destructor.
    // It can, however, have static data members, static member functions, and non-static non-virtual member functions
    // references are not allowed. In addition, a POD type can’t have constructors, virtual functions, base classes, or an overloaded assignment operator
    struct PodExample
    {
      char          c;
      static double sd;
      double        d;
      int           i;
    };

    double PodExample::sd = 0;
}

BOOST_AUTO_TEST_CASE( PodTest )
{
    // a.c = 72
    // a::sd skipped because static
    // a.d = 1.2
    // a.i = 0 (If there are fewer initializers in the list than there are members in the pod, then each member not explicitly initialized shall be value-initialized)
    PodExample p = { 72, 1.2 };
    BOOST_CHECK( p.c == 72 && PodExample::sd == 0 && p.d == 1.2 && p.i == 0 );
}

namespace
{
    // Diff pointer / reference / copy / rvalue reference
    // todo : http://www.cplusplus.com/articles/z6vU7k9E/
    // stack faster than heap for allocation as it only need to use a single to move the stack pointer (at least on x86) (stack is a (big) buffer with static size)
    // also stack data are more cache friendly as they are aligned
    // heap datas might be cold, meaning they could not be used for a long time, making them more apt to cache miss compared to stack datas (can still happen for both)
}

BOOST_AUTO_TEST_SUITE_END() // Argument
