// WebWatch Core - by DEATH, 2004
//
// $Workfile: PlainSocketImpl.cpp $ - Plain socket implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "PlainSocketImpl.h"
#include "SocketExceptions.h"

namespace Core_DE050401
{
    PlainSocketImpl::PlainSocketImpl(const AbortIndicator & abortIndicator)
    : SocketImpl(abortIndicator)
    {
    }

    PlainSocketImpl::~PlainSocketImpl()
    {
    }

    int PlainSocketImpl::Connect(const char *host, unsigned short port)
    {
        AddrInfo addr = Resolve(host, port);
        if (addr.Get() == 0) {
            std::string msg = "Could not resolve host ";
            msg += host;
            throw SocketError(msg);
        }

        return connect(GetDescriptor(), addr.Get()->ai_addr, static_cast<int>(addr.Get()->ai_addrlen));
    }

    std::size_t PlainSocketImpl::Send(const void *data, std::size_t size)
    {
        size_t pos = 0;
        SOCKET sock = GetDescriptor();
        const char *p = static_cast<const char *>(data);
        while (pos < size) {
            int sent = send(sock, p + pos, static_cast<int>(size - pos), 0);
            if (sent == SOCKET_ERROR)
                throw SocketError("Error while sending data");
            pos += static_cast<size_t>(sent);
        }
        return pos;
    }

    std::size_t PlainSocketImpl::Receive(void *data, std::size_t size)
    {
        int received = recv(GetDescriptor(), static_cast<char *>(data), static_cast<int>(size), 0);
        if (received == SOCKET_ERROR)
            throw SocketError("Error while receiving data");
        return static_cast<size_t>(received);
    }
}
