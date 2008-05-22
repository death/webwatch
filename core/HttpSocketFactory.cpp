// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpSocketFactory.cpp $ - HTTP socket factory implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "HttpSocketFactory.h"
#include "PlainSocketImpl.h"
#include "SecureSocketImpl.h"
#include "SocketExceptions.h"
#include "Socks4ProxyHttpSocket.h"
#include "Socks5ProxyHttpSocket.h"
#include "HttpConnectProxyHttpSocket.h"
#include "HttpGetProxyHttpSocket.h"

namespace Core_DE050401
{
    HttpSocketFactoryImpl::HttpSocketFactoryImpl()
    {
    }

    HttpSocketFactoryImpl::~HttpSocketFactoryImpl()
    {
    }

    std::auto_ptr<HttpSocket> HttpSocketFactoryImpl::CreateObject(const AbortIndicator & abortIndicator, const XML::CXML & parameters)
    {
        if (strcmp(parameters.GetTagName(), "Socket") != 0)
            throw SocketCreationError("Invalid main tag name in parameters");

        const char *mode = parameters.GetParamText("Mode", "Plain");
        std::auto_ptr<SocketImpl> socketImpl;
        if (strcmp(mode, "Plain") == 0) {
            socketImpl.reset(new PlainSocketImpl(abortIndicator));
        } else if (strcmp(mode, "Secure") == 0) {
            socketImpl.reset(new SecureSocketImpl(abortIndicator));
        } else {
            throw SocketCreationError("Invalid socket mode specified");
        }

        std::auto_ptr<HttpSocket> sock;
        const XML::CXML *xmlProxy = parameters.FindFirstChild("Proxy");
        if (xmlProxy == 0) {
            // Create a direct connection socket
            sock.reset(new HttpSocket(socketImpl));
            return sock;
        }

        const char *proxyType = xmlProxy->GetParamText("Type");
        const char *proxyAddress = xmlProxy->GetText();
        if (*proxyType && !*proxyAddress)
            throw SocketCreationError("A proxy type is specified, but no proxy address");
        
        if (strcmp(proxyType, "Socks4") == 0) {
            sock.reset(new Socks4ProxyHttpSocket(socketImpl, proxyAddress));
        } else if (strcmp(proxyType, "Socks5") == 0) {
            sock.reset(new Socks5ProxyHttpSocket(socketImpl, proxyAddress));
        } else if (strcmp(proxyType, "HttpConnect") == 0) {
            sock.reset(new HttpConnectProxyHttpSocket(socketImpl, proxyAddress));
        } else if (strcmp(proxyType, "HttpGet") == 0) {
            sock.reset(new HttpGetProxyHttpSocket(socketImpl, proxyAddress));
        } else {
            throw SocketCreationError("Unknown proxy type");
        }

        return sock;
    }

    HttpSocketFactoryImpl::ProxyTypes HttpSocketFactoryImpl::GetProxyTypes() const
    {
        ProxyTypes types;
        types.push_back("Socks4");
        types.push_back("Socks5");
        types.push_back("HttpConnect");
        types.push_back("HttpGet");
        return types;
    }
}
