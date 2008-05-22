// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpConnectProxyHttpSocket.h $ - HTTP CONNECT Proxy HTTP socket
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_HTTPCONNECTPROXYHTTPSOCKET_H
#define WEBWATCH_INC_HTTPCONNECTPROXYHTTPSOCKET_H

#include "ProxyHttpSocket.h"

namespace Core_DE050401
{
    class HttpConnectProxyHttpSocket : public ProxyHttpSocket
    {
    public:
        // Transfers ownership of SocketImpl
        HttpConnectProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy);

    protected:
        virtual int ConnectImpl(const char *address);
    };
}

#endif // WEBWATCH_INC_HTTPCONNECTPROXYHTTPSOCKET_H