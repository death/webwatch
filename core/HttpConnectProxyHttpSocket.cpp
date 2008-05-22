// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpConnectProxyHttpSocket.cpp $ - HTTP CONNECT Proxy HTTP socket implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/AddressParser.h"
#include "util/SocketUtil.h"

#include "HttpConnectProxyHttpSocket.h"
#include "SocketExceptions.h"

namespace Core_DE050401
{
    HttpConnectProxyHttpSocket::HttpConnectProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy)
    : ProxyHttpSocket(socketImpl, proxy)
    {
    }

    int HttpConnectProxyHttpSocket::ConnectImpl(const char *address)
    {
        ProxyHttpSocket::ConnectImpl(GetProxy());
        Util::AddressParser parser(address);
        Util::AddressParser proxyParser(GetProxy());

        std::ostringstream request;
        request << "CONNECT " << parser.GetHost() << ":" << parser.GetPort() << " HTTP/1.0\r\n";
        request << "Host: " << proxyParser.GetHost() << ":" << proxyParser.GetPort() << "\r\n";
        request << "User-Agent: " << GetAgent() << "\r\n";

        const char *user = proxyParser.GetUser();
        const char *password = proxyParser.GetPassword();

        if (*user || *password) {
		    // Encode user:password
            std::string auth = user;
            auth += ':';
            auth += password;
            auth = Util::BinToBase64(auth.c_str(), auth.length());

            request << "Proxy-Authorization: Basic " << auth << "\r\n";
	    }
    	
        request << "\r\n";

        Send(request.str().c_str());

        std::string response = Receive();

	    if (response.length() < 12) 
            throw ProxyError("Invalid response");
        
        if (response[9] != '2' || response[10] != '0' || response[11] != '0') {
		    // Proxy responded with error
            std::ostringstream msg;
            int err = (response[9] - '0') * 100 + (response[10] - '0') * 10 + (response[11] - '0');
            msg << "Responded with error " << err;
		    throw ProxyError(msg.str().c_str());
	    }

        return 0;
    }
}
