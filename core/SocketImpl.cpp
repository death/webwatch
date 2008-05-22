// WebWatch Core - by DEATH, 2004
//
// $Workfile: SocketImpl.cpp $ - WebWatch socket implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SocketImpl.h"
#include "SocketExceptions.h"

namespace 
{
    const char * const Agent = "WebWatch/1.0";
    const size_t SendBufferSize = 2048;
    const size_t ReceiveBufferSize = 8192;
}

namespace Core_DE050401
{
    AbortIndicator::~AbortIndicator()
    {
    }

    struct SocketImpl::Impl
    {
        Impl(const AbortIndicator & abortIndicator_)
        : abortIndicator(abortIndicator_)
        {
        }

        SOCKET sock;
        const AbortIndicator & abortIndicator;
    };

    SocketImpl::SocketImpl(const AbortIndicator & abortIndicator)
    : m_pimpl(new Impl(abortIndicator))
    {
        m_pimpl->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_pimpl->sock == INVALID_SOCKET)
            throw SocketCreationError("Could not create windows socket");
    }

    SocketImpl::~SocketImpl()
    {
        closesocket(m_pimpl->sock);
    }

    std::size_t SocketImpl::Send(std::istream & is)
    {
        std::vector<char> buffer(SendBufferSize);
        size_t total = 0;

        while (1) {
            is.read(&buffer[0], SendBufferSize);
            size_t bytes = static_cast<size_t>(is.gcount());
            if (bytes == 0 || m_pimpl->abortIndicator.ShouldAbort())
                break;

            total += Send(&buffer[0], bytes);
        }

        return total;
    }

    std::size_t SocketImpl::Send(const std::string & str)
    {
        return Send(str.c_str(), str.length());
    }

    std::string SocketImpl::Receive()
    {
        std::ostringstream os;
        Receive(os);
        return os.str();
    }

    std::size_t SocketImpl::Receive(std::ostream & os)
    {
        std::vector<char> bufferHolder(ReceiveBufferSize);
        char *buffer = &bufferHolder[0];
        size_t total = 0;
        size_t received;
        timeval tv;
        FD_SET readfds;

        const int MaxTries = 10;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        do {
            int tries = 0;
            int result;
            while (tries < MaxTries) {
                FD_ZERO(&readfds);
                FD_SET(m_pimpl->sock, &readfds);
                result = select(0, &readfds, 0, 0, &tv);
                if (result == SOCKET_ERROR) {
                    int error = WSAGetLastError();
                    std::ostringstream msg;
                    msg << "Socket error on receiving data (select) : 0x" << std::hex << error;
                    throw SocketError(msg.str());
                }

                if (result != 0)
                    break;

                // Timeout
                if (m_pimpl->abortIndicator.ShouldAbort())
                    return total;

                tries++;
            }

            if (tries == MaxTries)
                break;

            received = Receive(buffer, ReceiveBufferSize);
            os.write(buffer, static_cast<std::streamsize>(received));
            total += received;
        } while (received);

        return total;
    }

    AddrInfo SocketImpl::Resolve(const char *host, unsigned short port, int af)
    {
        struct addrinfo *result(0);

        struct addrinfo hints = {0};
        hints.ai_family = af;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        const std::string & portString(boost::lexical_cast<std::string>(port));

        if (getaddrinfo(host, portString.c_str(), &hints, &result) != 0)
            result = 0;

        return AddrInfo(result);
    }

    const char *SocketImpl::GetAgent()
    {
        return Agent;
    }

    SOCKET SocketImpl::GetDescriptor()
    {
        return m_pimpl->sock;
    }
}
