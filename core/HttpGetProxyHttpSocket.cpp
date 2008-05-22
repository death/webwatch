// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpGetProxyHttpSocket.cpp $ - HTTP GET Proxy HTTP socket implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/SocketUtil.h"
#include "util/AddressParser.h"
#include "util/TimeUtil.h"

#include "HttpGetProxyHttpSocket.h"

namespace Core_DE050401
{
    HttpGetProxyHttpSocket::HttpGetProxyHttpSocket(std::auto_ptr<SocketImpl> socketImpl, const char *proxy)
    : ProxyHttpSocket(socketImpl, proxy)
    {
    }

    std::string HttpGetProxyHttpSocket::CreateRequestImpl(const char *address, RequestMethod method, bool finalize)
    {
        std::ostringstream request;
        Util::AddressParser parser(address);

        request << (method == head ? "HEAD" : "GET");
        request << " ";
        // Full address
        request << parser.GetType() 
                << "://" 
                << parser.GetHost() 
                << ":" 
                << parser.GetPort()
                << parser.GetFile();
        request << " HTTP/1.0\r\n";

        request <<  "Host: ";
        request << parser.GetHost();
        request << "\r\n";

        request << "User-Agent: " << GetAgent() << "\r\n";
        request << "Connection: Close\r\n";

        time_t now = time(0);
        struct tm *gmt = gmtime(&now);
        if (gmt) {
            char timeStr[256];
            strftime(timeStr, 256, "%a, %d %b %Y %H:%M:%S GMT", gmt);
            request << "Expires: ";
            request << timeStr;
            request << "\r\n";
        }

        const char *user = parser.GetUser();
        if (user[0]) {
            // Authorization needed
            const char *password = parser.GetPassword();
            std::string credentials = user;
            credentials += ':';
            credentials += password;

            request << "Authorization: Basic ";
            request << Util::BinToBase64(credentials.c_str(), credentials.length());
            request << "\r\n";
        }

	    request << "Proxy-Connection: Close\r\n";

        if (finalize)
            request << "\r\n";

        return request.str();
    }

    int HttpGetProxyHttpSocket::ConnectImpl(const char *)
    {
        return ProxyHttpSocket::ConnectImpl(GetProxy());
    }
}
