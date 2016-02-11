//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#pragma warning( push )
#pragma warning( disable : 4005 )
#include <boost/asio.hpp>
#pragma warning( pop )
#include <boost/utility/string_ref.hpp>

#include <thread>
#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>

namespace ba = boost::asio;

BOOST_AUTO_TEST_SUITE( BasicNetworkingTestSuite )

BOOST_AUTO_TEST_CASE( SynchronousTimerTest )
{
    ba::io_service io;
    ba::deadline_timer t( io, boost::posix_time::seconds( 2 ) );

    std::function< void ( const boost::system::error_code& ) > timerTicker = [ &timerTicker, &t ] ( const boost::system::error_code& /*e*/ )
        {
            static int count = 5;
            if ( count-- > 0 )
            {
                t.expires_at( t.expires_at() + boost::posix_time::seconds( 1 ) );
                t.async_wait( timerTicker );
            }
        };


    t.async_wait( timerTicker );

    io.run();
}

// OLD STYLE RAW SOCKET C-style
namespace
{
    void    print_addrinfo( addrinfo* res )
    {
        boost::system::error_code ec;
        char ipstr[ INET6_ADDRSTRLEN ];
        for ( auto p = res; p != 0; p = p->ai_next )
        {
            void* addr;
            char* ipver;
            int port;

            // get the pointer to the address itself,
            // different fields in IPv4 and IPv6:
            if ( p->ai_family == AF_INET ) // IPv4
            {
                struct sockaddr_in *ipv4 = ( struct sockaddr_in * )p->ai_addr;
                addr = &( ipv4->sin_addr );
                port = ipv4->sin_port;
                ipver = "IPv4";
            }
            else // IPv6
            {
                struct sockaddr_in6 *ipv6 = ( struct sockaddr_in6 * )p->ai_addr;
                addr = &( ipv6->sin6_addr );
                port = ipv6->sin6_port;
                ipver = "IPv6";
            }

            // convert the IP to a string and print it:
            boost::asio::detail::socket_ops::inet_ntop( p->ai_family, addr, ipstr, sizeof ipstr, 0, ec );
            printf( "  %s: %s@%d\n", ipver, ipstr, port );
        }
    }

    static constexpr const char* END_MESSAGE = "END_MESSAGE";
    static constexpr const char* PORT_TEST = "20453";

    struct RAIIRawSocketInfos
    {
        RAIIRawSocketInfos()
            : res( nullptr )
            , socketFd( -1 )
            , clientFd( -1 )
        {}

        ~RAIIRawSocketInfos()
        {
            if ( clientFd != -1 )
                closesocket( clientFd );

            if ( socketFd != -1 )
                closesocket( socketFd );

            if ( res )
                freeaddrinfo( res );
        }

        addrinfo*   res;
        SOCKET      socketFd;
        SOCKET      clientFd;
    };

    // getaddrinfo();
    // socket();
    // bind();
    // listen();
    // accept();
    void    setup_server()
    {
        // Prepare the socket address structures for subsequent use. It's also used in host name lookups, and service name lookups.
        addrinfo hints;
        ZeroMemory( &hints, sizeof( hints ) );

        hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // tcp
        hints.ai_protocol = IPPROTO_TCP; // or 0 for any
        //hints.ai_flags = AI_PASSIVE; // fill my ip

        int status;
        RAIIRawSocketInfos socketInfos;
        if ( ( status = getaddrinfo( 0 /*localhost*/, PORT_TEST /*port*/, &hints, &socketInfos.res ) ) != 0 )
            throw std::runtime_error( "getaddrinfo error" );

        print_addrinfo( socketInfos.res );

        if ( ( socketInfos.socketFd = socket( socketInfos.res->ai_family, socketInfos.res->ai_socktype, socketInfos.res->ai_protocol ) ) == -1 ) // just use the first sock available
            throw std::runtime_error( "socket error" );

        // If you want to bind to a specific local IP address, drop the AI_PASSIVE and put an IP address in for the first argument to getaddrinfo()
        if ( bind( socketInfos.socketFd, socketInfos.res->ai_addr, static_cast< int >( socketInfos.res->ai_addrlen ) ) == -1 )
            throw std::runtime_error( "bind error" );

        // Incoming connections are going to wait in this queue until you accept() them (see below) and this is the limit on how many can queue up
        auto backlog = 2; // how many pending connections queue will hold
        if ( listen( socketInfos.socketFd, backlog ) == -1 )
            throw std::runtime_error( "listen error" );

        sockaddr_storage clientAddr;
        int addrSize = sizeof( sockaddr_storage );
        if ( ( socketInfos.clientFd = accept( socketInfos.socketFd, reinterpret_cast< sockaddr* >( &clientAddr ), &addrSize ) ) == -1 )
            throw std::runtime_error( "client error" );

        std::array< char, 64 > buffer;
        int bytesRead;
        while ( ( bytesRead = recv( socketInfos.clientFd, buffer.data(), static_cast< int >( buffer.size() ) - 1, 0 /*flags*/ ) ) > 0 )
        {
            buffer[ bytesRead ] = 0;
            std::cout << "server received: " << buffer.data() << std::endl;
            if ( boost::string_ref( buffer.data() ) == END_MESSAGE )
                break;

            if ( send( socketInfos.clientFd, "OK", 2, 0 /*flags*/ ) != 2 )
                throw std::runtime_error( "send error" );
        }

        if ( bytesRead == -1 )
            throw std::runtime_error( "recv error" );
    }

    // getaddrinfo();
    // socket();
    // connect();
    void    setup_client()
    {
        // Prepare the socket address structures for subsequent use. It's also used in host name lookups, and service name lookups.
        addrinfo hints;
        ZeroMemory( &hints, sizeof( hints ) );

        hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // tcp
        hints.ai_protocol = IPPROTO_TCP; // or 0 for any

        int status;
        RAIIRawSocketInfos socketInfos;
        if ( ( status = getaddrinfo( 0 /*localhost*/, PORT_TEST /*port*/, &hints, &socketInfos.res ) ) != 0 )
            throw std::runtime_error( "getaddrinfo error" );

        print_addrinfo( socketInfos.res );

        if ( ( socketInfos.socketFd = socket( socketInfos.res->ai_family, socketInfos.res->ai_socktype, socketInfos.res->ai_protocol ) ) == -1 )
            throw std::runtime_error( "socket error" );

        if ( connect( socketInfos.socketFd, socketInfos.res->ai_addr, static_cast< int >( socketInfos.res->ai_addrlen ) ) == -1 )
            throw std::runtime_error( "connect error" );

        // send() returns the number of bytes actually sent out-this might be less than the number you told it to send!
        // if the value returned by send() doesn't match the value in len, it's up to you to send the rest of the string
        const char* m1 = "hello";
        auto sizeM = static_cast< int >( strlen( m1 ) );
        if ( send( socketInfos.socketFd, m1, sizeM, 0 /*flags*/ ) != sizeM )
            throw std::runtime_error( "send error" );

        std::array< char, 64 > buffer;
        int bytesRead;
        // A return value of 0 means the host has closed the connection
        if ( ( bytesRead = recv( socketInfos.socketFd, buffer.data(), static_cast< int >( buffer.size() ) - 1, 0 /*flags*/ ) ) < 0 )
            throw std::runtime_error( "recv error" );

        buffer[ bytesRead ] = 0;
        std::cout << "client received: " << buffer.data() << std::endl;

        sizeM = static_cast< int >( strlen( END_MESSAGE ) );
        if ( send( socketInfos.socketFd, END_MESSAGE, sizeM, 0 /*flags*/ ) != sizeM )
            throw std::runtime_error( "send error" );
    }
}

BOOST_AUTO_TEST_CASE( ClientServerRawSocketTest )
{
    std::thread ts( []
    {
        try
        {
            setup_server();
        }
        catch ( ... )
        {
            BOOST_REQUIRE( false );
        }
    } );

    std::thread tc( []
    {
        try
        {
            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
            setup_client();
        }
        catch ( ... )
        {
            BOOST_REQUIRE( false );
        }
    } );

    ts.join();
    tc.join();
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // ! BasicNetworkingTestSuite
