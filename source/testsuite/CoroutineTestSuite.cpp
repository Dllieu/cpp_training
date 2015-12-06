//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <experimental/generator>
#include <iostream>
#include <array>

BOOST_AUTO_TEST_SUITE( CoroutineTestSuite )

namespace
{
    // If a coroutine has a declared return type that contains a placeholder type, then the return type of the coroutine is deduced as follows :
    //  - If a yield statement and either an await expression or an await-for statement are present, then the return type is std::experimental::async_stream<T >, where T is deduced from the yield statements.
    //  - Otherwise, if an await expression or an await-for statement are present in a function, then the return type is std::experimental::task<T> where type T is deduced from return statements
    //  - Otherwise, if a yield statement is present in a function, then the return type is std::experimental::generator<T>
    /*std::experimental::generator<int>*/auto   generate_index( size_t to )
    {
        auto i = 0;
        while ( i < to )
            __yield_value i++;
    }

    auto   generate_fibonacci( size_t to )
    {
        auto a = 0;
        auto b = 1;
        for ( auto& i: generate_index( to ) )
        {
            __yield_value a;
            auto next = a + b;
            a = b;
            b = next;
        }
    }
}

BOOST_AUTO_TEST_CASE( GeneratorTest )
{
    std::array< int, 7 > fibo_result{ 0, 1, 1, 2, 3, 5, 8 };
    auto i = 0;

    // std::experimental::generator<int>::iterator::operator* will call promise()._CurrentValue
    for ( auto& n : generate_fibonacci( fibo_result.size() ) )
    {
        static_assert( std::is_same< std::decay_t< decltype( n ) >, int >::value, "" );
        BOOST_CHECK( fibo_result[ i++ ] == n );
    }
}

BOOST_AUTO_TEST_SUITE_END() // CoroutineTestSuite
