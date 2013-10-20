#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <future>

BOOST_AUTO_TEST_SUITE( Threading )

namespace
{
    unsigned    accumulate( unsigned from, unsigned to )
    {
        auto result = from;
        while ( from < to ) result += ++from;
        return result;
    }
}

BOOST_AUTO_TEST_CASE( FutureTestSuite )
{
    unsigned from = 0, to = 100000;
    std::future< unsigned > f = std::async( accumulate, from, to );
    BOOST_CHECK( f.get() /* join until result is not received */ == accumulate( from, to ) );
}

BOOST_AUTO_TEST_CASE( ThreadGroupTestSuite )
{
    boost::thread_group threadGroup;

    unsigned threadNumber = 10;
    for ( unsigned i = 0; i < threadNumber; ++i )
        threadGroup.add_thread( new boost::thread( [] ( unsigned threadId ) { std::cout << threadId << std::endl; },
                                                   i ) );
    threadGroup.join_all();
}

BOOST_AUTO_TEST_SUITE_END() // Threading
