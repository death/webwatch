// WebWatch Core - by DEATH, 2004
//
// $Workfile: Socks4ProxyHttpSocket.cpp $ - HTTP SOCKS4 Proxy HTTP socket implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/AddressParser.h"

#include "Socks4ProxyHttpSocket.h"
#include "SocketExceptions.h"

namespace Core_DE050401
{
    Socks4ProxyHttpSocket::Socks4ProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy)
    : ProxyHttpSocket(socketImpl, proxy)
    {
    }

    int Socks4ProxyHttpSocket::ConnectImpl(const char *address)
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
        std::string request("\04\01ppaaaa");
        request += proxyParser.GetUser();

        const struct sockaddr_in *sin = addr.GetSockAddrIn();
        memmove(&request[2], &sin->sin_port, 2);
        memmove(&request[4], &sin->sin_addr.s_addr, 4);
        
        Send(request.c_str(), request.length() + 1);

        timeval tv;
	    tv.tv_sec = 10;
	    tv.tv_usec = 0;

	    fd_set readfds;
        FD_ZERO(&readfds);
	    FD_SET(GetDescriptor(), &readfds);

	    int res = select(0, &readfds, NULL, NULL, &tv);
	    if (res == SOCKET_ERROR)
		    throw ProxyError("Error on select()");
	    if (res == 0)
		    throw ProxyError("Timed out");

        std::string response = Receive();

	    // Process proxy reply
	    if (response.length() < 8) 
            throw ProxyError("Bad response (less than 8 bytes received)");

	    if (response[0] != '\0') 
            throw ProxyError("Bad response (first byte not 0)");

	    switch (response[1]) {
		    case 90:
                break;
		    case 91:
		    case 92:
		    case 93:
		    default:
                std::ostringstream msg;
                msg << "Bad response (code " << static_cast<int>(response[1]) << ")";
                throw ProxyError(msg.str());
	    }

        return 0;
    }
}
