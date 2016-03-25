//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

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

// TODO:
// - What's the difference between Flow Control and Congestion Control in TCP? (http://stackoverflow.com/questions/16473038/whats-the-difference-between-flow-control-and-congestion-control-in-tcp)
// - how tcp implement guarantee in order data transmission
//   * (http://stackoverflow.com/questions/9936496/how-does-tcp-implement-guarantee-in-order-data-transmission)
//   * what are retransmission rules (http://stackoverflow.com/questions/12956685/what-are-the-retransmission-rules-for-tcp)
//   * http://stackoverflow.com/questions/16259774/what-if-a-tcp-handshake-segment-is-lost
// - can tcp/udp split packet into pieces (http://serverfault.com/questions/534063/can-tcp-and-udp-packets-be-split-into-pieces)
// - difference udp/tcp (http://serverfault.com/questions/8981/what-is-the-difference-between-udp-and-tcp?rq=1)
// - why 3 way handshake tcp connection (http://serverfault.com/questions/35997/tcp-connection-handshake?lq=1)
// - UDP flow
// - real life example udp / tcp : quake 3(http://fabiensanglard.net/quake3/network.php) + http://stackoverflow.com/questions/6187456/tcp-vs-udp-on-video-stream?rq=1 + http://trac.bookofhook.com/bookofhook/trac.cgi/wiki/IntroductionToMultiplayerGameProgramming
// - blocking vs non blocking socket http://www.beej.us/guide/bgnet/output/html/multipage/advanced.html http://stackoverflow.com/questions/10654286/why-should-i-use-non-blocking-or-blocking-sockets
// - select / epoll
BOOST_AUTO_TEST_SUITE( BasicNetworkingTestSuite )

BOOST_AUTO_TEST_CASE( BoostTcpTest )
{
    // TODO
}

BOOST_AUTO_TEST_CASE( BoostUdpTest )
{
    // TODO
}

// Big picture about layering:
// ______________________________________________________________________________
// | Ethernet                     | IP                  | UDP     | TFTP  | Data |
// |______________________________|_____________________|_________|_______|______|
//
// When another computers receives the packet, the hardware strips the Ethernet header, the kernel strips the IP and UDP headers, the TFTP program strips the TFTP header, and it finally has the data
//
// Max MTU (Maximum Transmission Unit) (64K bytes)
//   - size of the largest protocol data unit that the layer can pass onwards
//   - In practicality, this is far larger than the size of the lower layers (e.g. ethernet)
// Max size Ethernet packet 1522 bytes
// Headers (big picture):
//   - Ethernet frame (24 bytes)
//   - IPv4 (min 20 bytes) / IPv6 (min 40 bytes)
//   - TCP (min 20 bytes <-> 60 bytes (40 bytes can be used for options))
//   - UDP (usually 8 bytes)
//
// TCP packet Min size (i.e. empty data) = 24 + 20 + 20 = 64 bytes
// UDP packet Min size (i.e. empty data) = 24 + 20 +  8 = 52 bytes
//
// TCP packet Max data size (i.e. no options) = 1522 - 64 ~= 1458 bytes
// UDP packet Max data size = 1522 - 8 ~= 1514 bytes
// Respecting the max size avoid having the packets fragmented by routers while traveling over the internet since most network MTU are around 1500 bytes
// Avoiding fragmentation is very important:
//    - Application layer loss is increased
//    - Not all network handle fragmented packet
//    - Fragmentation can cause re-ordering
//    - Every part of the datagram have to be waited on and then time-costly re-assembled
//
//
//
// About TCP and small packet:
//   - Nagle's algorithm is a means of improving the efficiency of TCP/IP networks by reducing the number of packets that need to be sent over the network
//      * if data is smaller than a limit (usually maximum segment size (MSS)), wait till receiving ack for previously sent packets and in the mean time accumulate data from user. Then send the accumulated data
//      * since you are sending only one packet instead of ten, you will not be subject to the problem of having the packets routed differently (not frequent but possible)
//         and this might even reduce latency because nothing guarantees you that all the packets will arrive in the right order
//      * Altough, while sending streaming data, wait for the ACK may increase latency
//      * if the receiver also implements 'delayed ACK policy', it will cause a temporary deadlock situation
//      * You should disable the Nagle algorithm if your application writes its messages in a way that makes buffering irrelevant.
//        Such cases includes: application that implements its own buffering mechanism or applications that send their messages in a single system call.
//        If your program writes its message in a single call, the Nagle algorithm will not bring anything useful.
//        It will at best do nothing and at worst add a delay by waiting for either the packet to be large enough or the remote host to acknowledge packets in flight, if any.
//        This can severely impact performance by adding a delay equal to the roundtrip between the two hosts, while not improving the bandwidth since your message is complete and no further data may be sent.
//   - TCP_NODELAY: disable Nagle's Algorithm. (must run benchmarking if it's for latency reason)
//   - TCP_CORK: aggressively accumulates data. If TCP_CORK is enabled in a socket, it will not send data until the buffer fills to a fixed limit. (also deactivate Nagle's algo)
//     
// Quick notes:
// - POSIX defines send/recv as atomic operations, in the case of multiple sends (in parallel), the second will likely block until the first completes
// - About send/recv completion
//      * send() will return after the memory has been pushed to the kernel buffer (might not been immediate as that could involve page fault -> additional delay from reloading from main memory)
//      * recv() shall block until a message arrives if no messages are available at the socket and O_NONBLOCK is not set on the socket's file descriptor. if O_NONBLOCK is set on the socket's file descriptor, recv() shall fail and set errno to [EAGAIN] or [EWOULDBLOCK] if no message available
// - The TCP/UDP stack (both on the sender and receiver side) will be able to buffer some data for you, and this is usually done in the OS kernel. So if calling recv too late, the data will likely be buffered by the OS
//   can modify the size with SO_RCVBUF / SO_SNDBUF
// - To setup specific option on your socket (e.g. TCP_NODELAY / SO_SNDBUF / ...), you can use setsockopt
//
// OLD STYLE TCP RAW SOCKET C-style
//
// Usual flow (big picture)
// TCP CLIENT           TCP SERVER
//
//                      socket()
//                      bind()
//                      listen()
//                      accept()
// socket()
// connect() <--------> (connection established 3way handshake) (client -> SYN | server -> SYN+ACK | client -> ACK)
// send()/recv() <----> send()/recv()
// close()
//                      close()
namespace
{
    void    print_addrinfo( addrinfo* res )
    {
        boost::system::error_code ec;
        std::array< char, INET6_ADDRSTRLEN > ipstr;
        for ( auto p = res; p != 0; p = p->ai_next )
        {
            void* addr;
            char* ipver;
            int port;

            // get the pointer to the address itself,
            // different fields in IPv4 and IPv6:
            if ( p->ai_family == AF_INET ) // IPv4
            {
                auto ipv4 = reinterpret_cast< sockaddr_in* >( p->ai_addr );
                addr = &( ipv4->sin_addr );
                port = ipv4->sin_port;
                ipver = "IPv4";
            }
            else // IPv6
            {
                auto ipv6 = reinterpret_cast< sockaddr_in6* >( p->ai_addr );
                addr = &( ipv6->sin6_addr );
                port = ipv6->sin6_port;
                ipver = "IPv6";
            }

            // convert the IP to a string and print it:
            boost::asio::detail::socket_ops::inet_ntop( p->ai_family, addr, ipstr.data(), ipstr.size(), 0, ec );
            printf( "  %s: %s@%d\n", ipver, ipstr.data(), port );
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

        // When a socket is created with socket, it exists in a name space (address family) but has no address assigned to it.
        if ( ( socketInfos.socketFd = socket( socketInfos.res->ai_family, socketInfos.res->ai_socktype, socketInfos.res->ai_protocol ) ) == -1 ) // just use the first sock available
            throw std::runtime_error( "socket error" );

        // If you want to bind to a specific local IP address, drop the AI_PASSIVE and put an IP address in for the first argument to getaddrinfo()
        // bind the socket to a physical address:port (e.g. all packet incoming to this host on this giving port will be forwarded to socketFd)
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

        // When a socket is created with socket, it exists in a name space (address family) but has no address assigned to it.
        if ( ( socketInfos.socketFd = socket( socketInfos.res->ai_family, socketInfos.res->ai_socktype, socketInfos.res->ai_protocol ) ) == -1 )
            throw std::runtime_error( "socket error" );

        // system call connects the socket referred to by the file descriptor sockfd to the address specified by addr
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
    std::thread ts( [] { BOOST_CHECK_NO_THROW( setup_server() ); } );
    std::thread tc( [] { std::this_thread::sleep_for( std::chrono::seconds( 1 ) ); BOOST_CHECK_NO_THROW( setup_client() ); } );

    ts.join();
    tc.join();

    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // ! BasicNetworkingTestSuite
