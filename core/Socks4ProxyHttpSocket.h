// WebWatch Core - by DEATH, 2004
//
// $Workfile: Socks4ProxyHttpSocket.h $ - SOCKS4 Proxy HTTP socket
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SOCKS4PROXYHTTPSOCKET_H
#define WEBWATCH_INC_SOCKS4PROXYHTTPSOCKET_H

#include "ProxyHttpSocket.h"

namespace Core_DE050401
{
    class Socks4ProxyHttpSocket : public ProxyHttpSocket
    {
    public:
        // Transfers ownership of SocketImpl
        Socks4ProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy);

    protected:
        virtual int ConnectImpl(const char *address);
    };
}

#endif // WEBWATCH_INC_SOCKS4PROXYHTTPSOCKET_H