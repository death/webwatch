// WebWatch Core - by DEATH, 2004
//
// $Workfile: SocketExceptions.h $ - Socket exceptions
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SOCKETEXCEPTIONS_H
#define WEBWATCH_INC_SOCKETEXCEPTIONS_H

#include "CoreException.h"

namespace Core_DE050401
{
    class SocketError : public Exception
    {
    public:
        SocketError(const std::string & error)
        : Exception(error)
        {
        }
    };

    class SSLError : public SocketError
    {
    public:
        SSLError(const std::string & error)
        : SocketError("(SSL) " + error)
        {
        }
    };

    class ProxyError : public SocketError
    {
    public:
        ProxyError(const std::string & error)
        : SocketError("(Proxy) " + error)
        {
        }
    };

    class SocketCreationError : public SocketError
    {
    public:
        SocketCreationError(const std::string & error)
        : SocketError(error)
        {
        }
    };

    class HttpError : public SocketError
    {
    public:
        HttpError(const std::string & error)
        : SocketError(error)
        {
        }
    };
}

#endif // WEBWATCH_INC_SOCKETEXCEPTIONS_H