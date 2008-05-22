// WebWatch Core - by DEATH, 2004
//
// $Workfile: SecureSocketImpl.cpp $ - Secure socket implementation (SSL)
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/SocketUtil.h"
#include "util/ThreadUtil.h"

#include "SecureSocketImpl.h"
#include "SocketExceptions.h"

namespace 
{
    SSL_CTX *s_sslContext;
    unsigned int s_sslRefs;
    Util::CriticalSection s_cs;
}

namespace Core_DE050401
{
    struct SecureSocketImpl::Impl
    {
        SSL *ssl;
    };

    SecureSocketImpl::SecureSocketImpl(const AbortIndicator & abortIndicator)
    : PlainSocketImpl(abortIndicator)
    , m_pimpl(new Impl)
    {
        m_pimpl->ssl = 0;

        Util::ScopedLock lock(s_cs);
        if (s_sslRefs == 0) {
            s_sslContext = SSL_CTX_new(SSLv23_client_method());
            if (s_sslContext == 0)
                throw SSLError("Could not initialize SSL context");
        }
        s_sslRefs++;
    }

    SecureSocketImpl::~SecureSocketImpl()
    {
        if (m_pimpl->ssl) {
            SSL_shutdown(m_pimpl->ssl);
            SSL_free(m_pimpl->ssl);
        }

        Util::ScopedLock lock(s_cs);
        if (--s_sslRefs == 0 && s_sslContext) {
            SSL_CTX_free(s_sslContext);
            s_sslContext = 0;
        }
    }

    int SecureSocketImpl::Connect(const char *host, unsigned short port)
    {
        int ret = PlainSocketImpl::Connect(host, port);
        if (ret != 0)
            return ret;

        m_pimpl->ssl = SSL_new(s_sslContext);
        if (m_pimpl->ssl == 0)
            throw SSLError("Could not initialize socket SSL context");

        SSL_set_fd(m_pimpl->ssl, static_cast<int>(GetDescriptor()));
        ret = SSL_connect(m_pimpl->ssl);
        if (ret < 1)
            throw SSLError("Could not establish a secure connection");

        return 0;
    }

    std::size_t SecureSocketImpl::Send(const void *data, std::size_t size)
    {
        size_t pos = 0;
        const char *p = static_cast<const char *>(data);
        while (pos < size) {
            int sent = SSL_write(m_pimpl->ssl, p + pos, static_cast<int>(size - pos));
            if (sent == SOCKET_ERROR)
                throw SocketError("Error while sending data");
            pos += static_cast<size_t>(sent);
        }
        return pos;
    }

    std::size_t SecureSocketImpl::Receive(void *data, std::size_t size)
    {
        int received = SSL_read(m_pimpl->ssl, static_cast<char *>(data), static_cast<int>(size));
        if (received == SOCKET_ERROR)
            throw SocketError("Error while receiving data");
        return static_cast<size_t>(received);
    }
}
