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
    typename std::enable_if_t< std::is_constructible< T, Args... >::value, T* >     create( factory_tag<T>, Args&&... args )
    {
        return new T( std::forward<Args>(args)... );
    }

    template <typename T, typename... Args>
    typename std::enable_if_t< ! std::is_constructible< T, Args... >::value, T* >   create( factory_tag<T>, Args&&... )
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
    static std::unique_ptr< A > create( TypeToCreate id, Args&&... args )
    {
        std::unique_ptr< A > result;
        switch ( id )
        {
            case TypeToCreate::A:
                result.reset( create( factory_tag< A >{}, std::forward< Args >( args )... ) );
                break;

            case TypeToCreate::B:
                result.reset( create( factory_tag< B >{}, std::forward< Args >( args )... ) );
                break;

            case TypeToCreate::C:
                result.reset( create( factory_tag< C >{}, std::forward< Args >( args )... ) );
                break;
        }

        return result;
    }
}

BOOST_AUTO_TEST_CASE( CallTest )
{
    std::unique_ptr< A > a( create( TypeToCreate::A, 5 ) );
    BOOST_CHECK( a.get() );

    BOOST_CHECK( create( TypeToCreate::A, "b" ) == nullptr );

    std::unique_ptr< A > c( create( TypeToCreate::C, "b" ) );
    BOOST_CHECK( c.get() );
}

BOOST_AUTO_TEST_SUITE_END() // FactoryTestSuite
