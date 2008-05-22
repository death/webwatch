// WebWatch Core - by DEATH, 2004
//
// $Workfile: ProxyHttpSocket.h $ - Base class for HTTP socket proxies
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_PROXYHTTPSOCKET_H
#define WEBWATCH_INC_PROXYHTTPSOCKET_H

#include "HttpSocket.h"

namespace Core_DE050401
{
    class ProxyHttpSocket : public HttpSocket
    {
    public:
        // Transfers ownership of SocketImpl
        ProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy);

        const char *GetProxy() const;

    protected:
        virtual int ConnectImpl(const char *address);

    private:
        std::string m_proxy;
    };
}

#endif // WEBWATCH_INC_PROXYHTTPSOCKET_H