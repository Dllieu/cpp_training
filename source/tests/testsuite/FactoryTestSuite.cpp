//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE( FactoryTestSuite )

namespace
{
    template <typename T> struct factory_tag { };

    template <typename T, typename... Args>
    typename std::enable_if< std::is_constructible< T, Args... >::value, T* >::type     create( factory_tag<T>, Args&&... args )
    {
        return new T( std::forward<Args>(args)... );
    }

    template <typename T, typename... Args>
    typename std::enable_if< ! std::is_constructible< T, Args... >::value, T* >::type   create( factory_tag<T>, Args&&... )
    {
        return nullptr;
    }
}

namespace
{
    struct A
    {
        A( int i )
        {
            BOOST_CHECK( true );
        }
    };

    struct B : public A
    {
        B( int i, char c )
            : A( i )
        {
            std::cout << "B" << std::endl;
            BOOST_CHECK( true );
        }
    };

    struct C : public A
    {
        C( const std::string& s )
            : A( 0 )
        {
            std::cout << "C" << std::endl;
            BOOST_CHECK( true );
        }
    };

    enum class TypeToCreate
    {
        A,
        B,
        C
    };

    template <class... Args>
    static A* create( TypeToCreate id, Args&&... args )
    {
        switch ( id )
        {
            case TypeToCreate::A:
                return create( factory_tag< A >{}, std::forward< Args >( args )... );

            case TypeToCreate::B:
                return create( factory_tag< B >{}, std::forward< Args >( args )... );

            case TypeToCreate::C:
                return create( factory_tag< C >{}, std::forward< Args >( args )... );

            default:
                return nullptr;
        }
    }
}

BOOST_AUTO_TEST_CASE( CallTestSuite )
{
    std::unique_ptr< A > a( create( TypeToCreate::A, 5 ) );
    BOOST_CHECK( a.get() );

    BOOST_CHECK( create( TypeToCreate::A, "b" ) == nullptr );

    std::unique_ptr< A > c( create( TypeToCreate::C, "b" ) );
    BOOST_CHECK( c.get() );
}

BOOST_AUTO_TEST_SUITE_END() // FactoryTestSuite
