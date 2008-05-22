// WebWatch Util - by DEATH, 2004
//
// $Workfile: IdentServer.cpp $ - A simple Ident server implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "IdentServer.h"
#include "ThreadUtil.h"
#include "EmptyString.h"

namespace
{
    SOCKADDR *Convert(SOCKADDR_IN *sin);
}

namespace Util_DE050401
{
    struct IdentServerImpl::Impl
    {
        std::string GetUser(unsigned short port);
        std::string GetDefaultUser();

        typedef std::map<unsigned short, std::string> ConnectionMap;

        ConnectionMap connections;
        std::string defUser;
        CriticalSection cs;
        volatile LONG stopFlag;
        HANDLE hThread;
        unsigned short port;
        SOCKET lsock;
    };

    IdentServerImpl::IdentServerImpl()
    : m_pimpl(new Impl)
    {
        m_pimpl->hThread = 0;
    }

    IdentServerImpl::~IdentServerImpl()
    {
        Stop();
    }

    bool IdentServerImpl::Start(unsigned short port)
    {
        Stop();

        m_pimpl->port = port;

        // Create listening socket
        m_pimpl->lsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_pimpl->lsock == INVALID_SOCKET)
            return false;

        // Bind listening socket
        SOCKADDR_IN sin;
        memset(&sin, 0, sizeof(SOCKADDR_IN));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(m_pimpl->port);
        sin.sin_addr.S_un.S_addr = INADDR_ANY;

        if (bind(m_pimpl->lsock, Convert(&sin), sizeof(SOCKADDR_IN)) != 0) {
            closesocket(m_pimpl->lsock);
            m_pimpl->lsock = INVALID_SOCKET;
            return false;
        }

        // Set socket mode to LISTENING
        if (listen(m_pimpl->lsock, 0) != 0) {
            closesocket(m_pimpl->lsock);
            m_pimpl->lsock = INVALID_SOCKET;
            return false;
        }

        // Create server thread
        m_pimpl->stopFlag = 0;
        unsigned int tid;
        m_pimpl->hThread = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, ThreadHelper, this, 0, &tid));

        if (m_pimpl->hThread == 0) {
            closesocket(m_pimpl->lsock);
            m_pimpl->lsock = INVALID_SOCKET;
            return false;
        }

        return true;
    }

    void IdentServerImpl::Stop()
    {
        if (m_pimpl->hThread == 0)
            return;

        // No need for InterlockedIncrement, it's not critical
        m_pimpl->stopFlag++;
        WaitForSingleObject(m_pimpl->hThread, INFINITE);
        CloseHandle(m_pimpl->hThread);
        m_pimpl->hThread = 0;

        if (m_pimpl->lsock != INVALID_SOCKET) {
            closesocket(m_pimpl->lsock);
            m_pimpl->lsock = INVALID_SOCKET;
        }
    }

    void IdentServerImpl::AddConnection(unsigned short port, const char *user)
    {
        ScopedLock lock(m_pimpl->cs);
        m_pimpl->connections[port] = user;
    }

    void IdentServerImpl::RemoveConnection(unsigned short port)
    {
        ScopedLock lock(m_pimpl->cs);
        m_pimpl->connections.erase(port);
    }

    void IdentServerImpl::SetDefaultUser(const char *user)
    {
        ScopedLock lock(m_pimpl->cs);
        if (user)
            m_pimpl->defUser = user;
        else
            m_pimpl->defUser.clear();
    }

    std::string IdentServerImpl::Impl::GetUser(unsigned short port)
    {
        ScopedLock lock(cs);
        ConnectionMap::const_iterator i = connections.find(port);
        return i != connections.end() ? i->second : EmptyString();
    }

    std::string IdentServerImpl::Impl::GetDefaultUser()
    {
        ScopedLock lock(cs);
        return defUser;
    }

    bool IdentServerImpl::IsRunning() const
    {
        return m_pimpl->stopFlag == 0;
    }

    unsigned short IdentServerImpl::GetPort() const
    {
        return m_pimpl->port;
    }

    unsigned int __stdcall IdentServerImpl::ThreadHelper(void *p)
    {
        IdentServerImpl *obj = static_cast<IdentServerImpl *>(p);
        obj->ThreadRoutine();
        return 0;
    }

    void IdentServerImpl::ThreadRoutine()
    {
        FD_SET readfds;
        FD_SET writefds;
        FD_SET exceptfds;
        TIMEVAL tv;
        SOCKET csock = INVALID_SOCKET;
        SOCKADDR_IN sin;
        std::ostringstream reply;

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        while (IsRunning()) {
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            if (csock == INVALID_SOCKET) {
                FD_SET(m_pimpl->lsock, &readfds);
                FD_SET(m_pimpl->lsock, &exceptfds);
            } else {
                FD_SET(csock, &readfds);
                FD_SET(csock, &writefds);
                FD_SET(csock, &exceptfds);
            }

            int ret = select(0, &readfds, &writefds, &exceptfds, &tv);
            if (ret == 0) {
                Sleep(0);
                continue;
            }

            if (ret == SOCKET_ERROR)
                break;

            if (FD_ISSET(m_pimpl->lsock, &exceptfds))
                break;

            if (FD_ISSET(m_pimpl->lsock, &readfds)) {
                char b[sizeof(BOOL)] = {0};
                int len = sizeof(b);

                if (getsockopt(m_pimpl->lsock, SOL_SOCKET, SO_ACCEPTCONN, b, &len) == 0) {
                    // A client wants to connect, accept
                    if (b) {
                        len = sizeof(SOCKADDR_IN);
                        csock = accept(m_pimpl->lsock, Convert(&sin), &len);
                    }
                }
            }

            if (csock == INVALID_SOCKET)
                continue;

            // From here we assume a client is connected
            if (FD_ISSET(csock, &exceptfds)) {
                closesocket(csock);
                csock = INVALID_SOCKET;
                continue;
            }

            if (FD_ISSET(csock, &writefds)) {
                std::string str = reply.str();
                if (str.empty() == false) {
                    send(csock, str.c_str(), static_cast<int>(str.length()), 0);
                    closesocket(csock);
                    csock = INVALID_SOCKET;
                    reply.str("");
                    continue;
                }
            }

            if (FD_ISSET(csock, &readfds)) {
                char ports[512] = {0};
                int len = recv(csock, ports, 512, 0);
                if (len == SOCKET_ERROR || len > 256) {
                    // If the request is too big or erroneous, disconnect the client
                    reply << "0, 0 : ERROR : UNKNOWN-ERROR\r\n";
                    continue;
                }

                char *stop = 0;
                long serverPort = strtol(ports, &stop, 10);
                if (stop == 0) {
                    // Bad request, a single or no number
                    reply << "0, 0 : ERROR : INVALID-PORT\r\n";
                    continue;
                }

                while (*stop == ' ')
                    stop++;

                if (*stop != ',') {
                    // Bad request, no comma
                    reply << "0, 0 : ERROR : INVALID-PORT\r\n";
                    continue;
                }

                stop++;

                while (*stop == ' ')
                    stop++;

                long clientPort = strtol(stop, 0, 10);

                reply << serverPort << ", " << clientPort;

                if (serverPort <= 0 || serverPort > 65535 || clientPort <= 0 || clientPort > 65535) {
                    // Port number out of range
                    reply << " : ERROR : INVALID-PORT\r\n";
                    continue;
                }

                std::string user = m_pimpl->GetUser(static_cast<unsigned short>(serverPort));
                if (user.empty()) {
                    user = m_pimpl->GetDefaultUser();
                    if (user.empty()) {
                        // No user
                        reply << " : ERROR : NO-USER\r\n";
                        continue;
                    }
                }

                reply << " : USERID : UNIX : " << user << "\r\n";
            }
        }

        if (csock != INVALID_SOCKET)
            closesocket(csock);
    }
}

namespace
{
    SOCKADDR *Convert(SOCKADDR_IN *sin)
    {
        return reinterpret_cast<SOCKADDR *>(sin);
    }
}
