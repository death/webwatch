// WebWatch Util - by DEATH, 2004
//
// $Workfile: IdentServer.h $ - A simple Ident server
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_IDENTSERVER_H
#define WEBWATCH_INC_IDENTSERVER_H

#include "loki/Singleton.h"

namespace Util_DE050401
{
    class IdentServerImpl
    {
        friend struct Loki::CreateUsingNew<IdentServerImpl>;
        IdentServerImpl();
        ~IdentServerImpl();

    public:
        bool Start(unsigned short port = 113);
        void Stop();
        void AddConnection(unsigned short port, const char *user);
        void RemoveConnection(unsigned short port);
        void SetDefaultUser(const char *user);
        bool IsRunning() const;
        unsigned short GetPort() const;

    private:
        static unsigned int __stdcall ThreadHelper(void *p);
        void ThreadRoutine();

    private:
        struct Impl;
        std::auto_ptr<Impl> m_pimpl;

    private: // Preventive methods
        IdentServerImpl(const IdentServerImpl &);
        IdentServerImpl & operator= (const IdentServerImpl &);
    };

    typedef Loki::SingletonHolder<IdentServerImpl> IdentServer;
}

#endif // WEBWATCH_INC_IDENTSERVER_H