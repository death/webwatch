// WebWatch Core - by DEATH, 2004
//
// $Workfile: Socks5ProxyHttpSocket.cpp $ - HTTP SOCKS5 Proxy HTTP socket implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/AddressParser.h"
#include "util/Util.h"

#include "Socks5ProxyHttpSocket.h"
#include "SocketExceptions.h"

namespace Core_DE050401
{
    Socks5ProxyHttpSocket::Socks5ProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy)
    : ProxyHttpSocket(socketImpl, proxy)
    {
    }

    int Socks5ProxyHttpSocket::ConnectImpl(const char *address)
    {
        Util::AddressParser parser(address);
        AddrInfo addr = Resolve(parser.GetHost(), parser.GetPort(), AF_INET);
        if (addr.Get() == 0) {
            std::string msg = "Could not resolve host ";
            msg += parser.GetHost();
            throw SocketError(msg);
        }

        ProxyHttpSocket::ConnectImpl(GetProxy());
        Util::AddressParser proxyParser(GetProxy());
        
        char request[1024];
        request[0] = 5; // version
        request[1] = 1; // number of methods
        request[2] = 0; // basic (no) auth method
        size_t requestSize = 3;

        if (*proxyParser.GetUser() || *proxyParser.GetPassword()) {
            request[1]++;
            request[3] = 2; // username/password auth method
            requestSize++;
        }

        size_t sent = Send(request, requestSize);
        if (sent != requestSize)
            throw ProxyError("Couldn't send request completely");

        std::string response = Receive();
        if (response.length() < 2)
            throw ProxyError("Bad or no response");

        if (response[0] != 5 || (response[1] != 0 && response[1] != 2))
            throw ProxyError("Bad response");

        if (response[1] == 2) {
            // Authentication
            char *p = request;
            *p++ = 1; // version
            // User
            *p++ = static_cast<char>(strlen(parser.GetUser()));
            strncpy(p, parser.GetUser(), p[-1]);
            p += p[-1];
            // Password
            *p++ = static_cast<char>(strlen(parser.GetPassword()));
            strncpy(p, parser.GetPassword(), p[-1]);
            p += p[-1];
            Send(request, p - request);

            response = Receive();
            if (response.length() < 2)
                throw ProxyError("Bad response (authentication)");
            
            if (response[0] != 1 || response[0] != 0)
                throw ProxyError("Bad packet data from socks5 proxy");
        }

	    // Send connect command
	    request[0] = 5; // version
	    request[1] = 1; // one method
	    request[2] = 0; // reserved
	    request[3] = 1; // ipv4

        const struct sockaddr_in *sin = addr.GetSockAddrIn();
        memmove(&request[4], &sin->sin_addr.s_addr, 4);
        memmove(&request[8], &sin->sin_port, 2);

        Send(request, 10);

        response = Receive();
        if (response.length() < 4)
            throw ProxyError("Bad response (final)");

        if (response[0] != 5 || response[1] != 0)
            throw ProxyError("Bad data");

	    size_t len;
	    switch (response[3]) {
		    case 1:
			    len = 4;
			    break;
		    case 3:
			    if (response.length() < 5) 
                    throw ProxyError("Bad data");
			    len = response[4];
			    break;
		    case 4:
			    len = 16;
			    break;
		    default:
			    throw ProxyError("Bad data");
	    }

	    if ((response.length() - 4) != len + 2) 
            throw ProxyError("Bad packet length");

	    return 0;
    }
}
