#ifndef __NETWORK_TCPCLIENT_H__
#define __NETWORK_TCPCLIENT_H__

namespace network
{
    class TcpClient
    {
    public:
        // can throw (ensure you can connect at least once)
        TcpClient( const std::string& address, const std::string& port, std::function<void (const std::string&)> onMessageFunctor );
        void    sendMessage( const std::string& message );

    private:
        struct pimpl;
        std::unique_ptr< pimpl > pimpl_;
    };
}

#endif /*! __NETWORK_TCPCLIENT_H__ */
