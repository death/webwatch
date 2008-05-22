// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpSocket.cpp $ - WebWatch HTTP socket implementation
//
// $Author: Death $
// $Revision: 4 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/SocketUtil.h"
#include "util/TimeUtil.h"
#include "util/AddressParser.h"

#include "HttpSocket.h"
#include "SocketImpl.h"
#include "PlainSocketImpl.h"

namespace Core_DE050401
{
    struct HttpSocket::Impl
    {
        std::auto_ptr<SocketImpl> socketImpl;
    };

    HttpSocket::HttpSocket(std::auto_ptr<SocketImpl> socketImpl)
    : m_pimpl(new Impl)
    {
        m_pimpl->socketImpl = socketImpl;
    }

    HttpSocket::~HttpSocket()
    {
    }

    int HttpSocket::Connect(const char *address)
    {
        return ConnectImpl(address);
    }

    std::string HttpSocket::CreateRequest(const char *address, RequestMethod method, bool finalize)
    {
        return CreateRequestImpl(address, method, finalize);
    }

    int HttpSocket::ConnectImpl(const char *address)
    {
        Util::AddressParser parser(address);
        return m_pimpl->socketImpl->Connect(parser.GetHost(), parser.GetPort());
    }

    size_t HttpSocket::Send(std::istream & is)
    {
        return m_pimpl->socketImpl->Send(is);
    }

    size_t HttpSocket::Send(const void *data, size_t size)
    {
        return m_pimpl->socketImpl->Send(data, size);
    }

    size_t HttpSocket::Send(const std::string & str)
    {
        return m_pimpl->socketImpl->Send(str);
    }

    size_t HttpSocket::Receive(std::ostream & os)
    {
        return m_pimpl->socketImpl->Receive(os);
    }

    std::string HttpSocket::Receive()
    {
        return m_pimpl->socketImpl->Receive();
    }

    std::string HttpSocket::CreateRequestImpl(const char *address, RequestMethod method, bool finalize)
    {
        std::ostringstream request;
        Util::AddressParser parser(address);

        request << (method == head ? "HEAD" : "GET") 
                << " " 
                << parser.GetFile() 
                << " HTTP/1.0\r\n"  

                <<  "Host: " 
                << parser.GetHost() 
                << "\r\n" 

                << "User-Agent: " << GetAgent() << "\r\n" 
                << "Connection: Close\r\n";

        time_t now = time(0);
        if (struct tm *gmt = gmtime(&now)) {
            char timeStr[256];
            strftime(timeStr, 256, "%a, %d %b %Y %H:%M:%S GMT", gmt);
            request << "Expires: " 
                    << timeStr 
                    << "\r\n";
        }

        const char *user = parser.GetUser();
        if (user[0]) {
            // Authorization needed
            const char *password = parser.GetPassword();
            std::string credentials = user;
            credentials += ':';
            credentials += password;

            request << "Authorization: Basic " 
                    << Util::BinToBase64(credentials.c_str(), credentials.length()) 
                    << "\r\n";
        }

        if (finalize)
            request << "\r\n";

        return request.str();
    }

    AddrInfo HttpSocket::Resolve(const char *host, unsigned short port, int af)
    {
        return SocketImpl::Resolve(host, port, af);
    }

    SOCKET HttpSocket::GetDescriptor()
    {
        return m_pimpl->socketImpl->GetDescriptor();
    }

    const char *HttpSocket::GetAgent()
    {
        return SocketImpl::GetAgent();
    }
}
