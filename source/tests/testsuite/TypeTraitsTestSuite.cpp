//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <type_traits>

BOOST_AUTO_TEST_SUITE( TypeTraits )

namespace
{
    // T&& is a universal , it can either mean lvalue (&) or rvalue (&&)
    template <typename T>
    T&& forward( T&& param )
    {
        if ( std::is_lvalue_reference< T >::value )
            return param; // if lvalue, return lvalue ( T& or const T& )
        return std::move( param );
    }
}

BOOST_AUTO_TEST_CASE( DecayTestSuite )
{
    BOOST_CHECK( ( std::is_same<int, std::decay<int&>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int, std::decay<int&&>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int, std::decay<const int&>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int*, std::decay<int[2]>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int(*)(int), std::decay<int (int)>::type>::value ) );

    int i = 5;
    BOOST_CHECK( ( std::is_same< decltype( forward(i) ), int& >::value ) );
    BOOST_CHECK( ( std::is_same< decltype( forward(5) ), int&& >::value ) );
}

namespace
{
    class A {};
    class B { virtual ~B() {} };
}

BOOST_AUTO_TEST_CASE( HasVirtualDestructorTestSuite )
{
    BOOST_CHECK( ! std::has_virtual_destructor< A >::value );
    BOOST_CHECK( std::has_virtual_destructor< B >::value );
}

namespace
{
    class RealClass
    {
    public:
        int     realA() { return 0; }
        double  realB() { return 1; }
    };

    class ForwardClass
    {
    public:
        int         forwardA();
        double      forwardB();

    private:
        RealClass   realClass_;
    };
}

#define IMPLEMFORWARDCLASS( methodName, methodFromRealClassToBeForwarded ) \
    std::decay< std::result_of< decltype ( &RealClass::##methodFromRealClassToBeForwarded )( RealClass ) >::type >::type ForwardClass::##methodName() \
    { \
        return realClass_.##methodFromRealClassToBeForwarded(); \
    }

IMPLEMFORWARDCLASS( forwardA, realA );
IMPLEMFORWARDCLASS( forwardB, realB );

#undef IMPLEMFORWARDCLASS

BOOST_AUTO_TEST_CASE( TraitsTestSuite )
{
    ForwardClass forwardClass;
    RealClass realClass;

    BOOST_CHECK( forwardClass.forwardA() == realClass.realA() );
    BOOST_CHECK( forwardClass.forwardB() == realClass.realB() );
}

namespace
{
    template < typename T >
    std::enable_if_t< std::is_pointer< T >::value >     callWithPointer( T, bool& isPointer ) { isPointer = true; }

    template < typename T, std::size_t sz >
    void callWithPointer( T(&)[sz], bool& isPointer ) { isPointer = false; }
}

BOOST_AUTO_TEST_CASE( ArrayParameterTestSuite )
{
    bool isPointer = false;
    int i[5];

    callWithPointer( &i, isPointer );
    BOOST_CHECK( isPointer );

    callWithPointer( i, isPointer );
    BOOST_CHECK( !isPointer );
}

BOOST_AUTO_TEST_SUITE_END() // TypeTraits
