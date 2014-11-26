#include <boost/test/unit_test.hpp>

#pragma warning( push )
#pragma warning( disable: 4250 )

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

BOOST_AUTO_TEST_SUITE( Twitch )

namespace
{
    class ITwitchCallback
    {
    public:
        virtual ~ITwitchCallback() = 0;

        virtual void onPing(); // usefull for gui status connected or disc, implicit ping reply by twitchclient
        virtual void onDiconnection() = 0; // redundant with onPing ?
        virtual void onNewMessage( const std::string& newMessage ) = 0;
    };
}

BOOST_AUTO_TEST_CASE( ConnectTestSuite )
{
    //boost::asio::io_service io_service;
    //boost::asio::ip::tcp::resolver resolver(io_service);

    // check and take one on http://twitchstatus.com/
    // se baser sur https://code.google.com/p/ntest/source/browse/Client.cpp
    boost::asio::ip::tcp::iostream stream("199.9.249.252", "80");
    BOOST_CHECK( stream );
    /*boost::asio::ip::tcp::resolver::query query( "199.9.253.199", "6667" );

    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;

    boost::asio::ip::tcp::socket socket(io_service);
    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
      socket.close();
      socket.connect(*endpoint_iterator++, error);
    }
    if (error)
      throw boost::system::system_error(error);*/
    std::cout << "ee" << std::endl;
}

#pragma warning( pop )

BOOST_AUTO_TEST_SUITE_END() // ! Twitch
