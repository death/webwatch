// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpGetProxyHttpSocket.h $ - HTTP GET Proxy HTTP socket
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_HTTPGETPROXYHTTPSOCKET_H
#define WEBWATCH_INC_HTTPGETPROXYHTTPSOCKET_H

#include "ProxyHttpSocket.h"

namespace Core_DE050401
{
    class HttpGetProxyHttpSocket : public ProxyHttpSocket
    {
    public:
        // Transfers ownership of SocketImpl
        HttpGetProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy);

    protected:
        virtual std::string CreateRequestImpl(const char *address, RequestMethod method, bool finalize);
        virtual int ConnectImpl(const char *address);
    };
}

#endif // WEBWATCH_INC_HTTPGETPROXYHTTPSOCKET_H