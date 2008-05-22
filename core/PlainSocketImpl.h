// WebWatch Core - by DEATH, 2004
//
// $Workfile: PlainSocketImpl.h $ - Plain socket implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_PLAINSOCKETIMPL_H
#define WEBWATCH_INC_PLAINSOCKETIMPL_H

#include "SocketImpl.h"

namespace Core_DE050401
{
    class PlainSocketImpl : public SocketImpl
    {
    public:
        PlainSocketImpl(const AbortIndicator & abortIndicator);
        virtual ~PlainSocketImpl();

        virtual int Connect(const char *host, unsigned short port);
        virtual std::size_t Send(const void *data, std::size_t size);
        virtual std::size_t Receive(void *data, std::size_t size);
    };
}

#endif // WEBWATCH_INC_PLAINSOCKETIMPL_H