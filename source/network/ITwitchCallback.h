//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __NETWORK_ITWITCHCALLBACK_H__
#define __NETWORK_ITWITCHCALLBACK_H__

namespace network
{
    class ITwitchCallback
    {
    public:
        virtual ~ITwitchCallback() {};

        virtual void onConnect() = 0;
        virtual void onDisconnect() = 0; // redundant with onPing ?
        virtual void onPing(); // usefull for gui status connected or disc, implicit ping reply by twitchclient
        virtual void onMessage( const std::string& newMessage ) = 0;
    };
}

#endif /* !__NETWORK_ITWITCHCALLBACK_H__ */
