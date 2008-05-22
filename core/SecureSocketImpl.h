// WebWatch Core - by DEATH, 2004
//
// $Workfile: SecureSocketImpl.h $ - Secure socket implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SECURESOCKETIMPL_H
#define WEBWATCH_INC_SECURESOCKETIMPL_H

#include "PlainSocketImpl.h"

namespace Core_DE050401
{
    class SecureSocketImpl : public PlainSocketImpl
    {
    public:
        SecureSocketImpl(const AbortIndicator & abortIndicator);
        virtual ~SecureSocketImpl();

        virtual int Connect(const char *host, unsigned short port);
        virtual std::size_t Send(const void *data, std::size_t size);
        virtual std::size_t Receive(void *data, std::size_t size);

    private:
        struct Impl;
        std::auto_ptr<Impl> m_pimpl;
    };
}

#endif // WEBWATCH_INC_SECURESOCKETIMPL_H