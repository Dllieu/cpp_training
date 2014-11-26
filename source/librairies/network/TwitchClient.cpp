//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/array.hpp>

#include "TwitchClient.h"

using namespace network;

TwitchClient::TwitchClient( const std::shared_ptr< ITwitchCallback >& twitchCallback,
                            const std::string& login,
                            const std::string& password )
    : twitchCallback_( twitchCallback )
    , login_( login )
    , password_( password )
{
    // NOTHING
}

TwitchClient::~TwitchClient()
{
    // NOTHING
}

bool    TwitchClient::connect()
{
    // already connected
    if ( tcpStream_ )
        return true;

    // TODO : check and take one ip/port from http://twitchstatus.com/
    try
    {
        tcpStream_.reset( new boost::asio::ip::tcp::iostream( "199.9.249.252", "80" ) );
        if ( ! *tcpStream_.get() )
            tcpStream_.reset();
    }
    catch (...)
    {
        tcpStream_.reset();
    }
    return tcpStream_;
}
