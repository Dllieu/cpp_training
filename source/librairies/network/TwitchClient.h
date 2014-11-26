//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __NETWORK_TWITCHCLIENT_H__
#define __NETWORK_TWITCHCLIENT_H__

#pragma warning( push )
#pragma warning( disable: 4250 )
#include <boost/asio.hpp>
#pragma warning( pop )
#include <boost/noncopyable.hpp>

#include "ITwitchCallback.h"

namespace network
{
    // todo : attache un worker tcp, qui fait le listen dans un thread a part
    class TwitchClient : boost::noncopyable
    {
    public:
        TwitchClient( const std::shared_ptr< ITwitchCallback >& twitchCallback,
                      const std::string& login,
                      const std::string& password );

        ~TwitchClient();

        // can throw
        bool    connect(); // try intern list of possible host
        void    sendMessage();


    private:
        std::shared_ptr< ITwitchCallback >                  twitchCallback_; // does unique make more sense or give more freedom for the user ?
        std::unique_ptr< boost::asio::ip::tcp::iostream >   tcpStream_;
        std::string                                         login_;
        std::string                                         password_;
    };
}

#endif /* ! __NETWORK_TWITCHCLIENT_H__ */
