// WebWatch Core - by DEATH, 2004
//
// $Workfile: ProxyHttpSocket.cpp $ - Proxy HTTP socket implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "ProxyHttpSocket.h"
#include "SocketExceptions.h"

namespace Core_DE050401
{
    ProxyHttpSocket::ProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy)
    : HttpSocket(socketImpl)
    , m_proxy(proxy)
    {
    }

    const char *ProxyHttpSocket::GetProxy() const
    {
        return m_proxy.c_str();
    }

    int ProxyHttpSocket::ConnectImpl(const char *address)
    {
        try {
            int ret = HttpSocket::ConnectImpl(address);
            if (ret != 0)
                throw ProxyError("Cannot connect");
        }
        catch(const SocketError & error) {
            throw ProxyError(error.what());
        }
        return 0;
    }
}
