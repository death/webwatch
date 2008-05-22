// WebWatch Core - by DEATH, 2004
//
// $Workfile: SocketImpl.h $ - Socket implementation abstract base class
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SOCKETIMPL_H
#define WEBWATCH_INC_SOCKETIMPL_H

namespace Core_DE050401
{
    class AddrInfo
    {
    public:
        explicit AddrInfo(struct addrinfo *info)
        : m_info(info)
        , m_refs(new int(1))
        {
        }

        AddrInfo(const AddrInfo & other)
        : m_info(other.m_info)
        , m_refs(other.m_refs)
        {
            ++*m_refs;
        }

        AddrInfo & operator= (const AddrInfo & rhs)
        {
            Destroy();
            m_info = rhs.m_info;
            m_refs = rhs.m_refs;
            ++*m_refs;
            return *this;
        }

        ~AddrInfo()
        {
            Destroy();
        }

        struct addrinfo *Get()
        {
            return m_info;
        }

        const struct sockaddr_in *GetSockAddrIn() const
        {
            struct sockaddr *addr = m_info->ai_addr;
            return reinterpret_cast<struct sockaddr_in *>(addr);
        }

    private:
        void Destroy()
        {
            if (--*m_refs != 0)
                return;
            delete m_refs;
            if (m_info) {
                freeaddrinfo(m_info);
                m_info = 0;
            }
        }

    private:
        int *m_refs;
        struct addrinfo *m_info;
    };

    class AbortIndicator
    {
    public:
        virtual ~AbortIndicator();
        virtual bool ShouldAbort() const = 0;
    };

    class SocketImpl
    {
    public:
        SocketImpl(const AbortIndicator & abortIndicator);
        virtual ~SocketImpl();

        virtual int Connect(const char *host, unsigned short port) = 0;
        virtual std::size_t Send(const void *data, std::size_t size) = 0;
        virtual std::size_t Receive(void *data, std::size_t size) = 0;

        SOCKET GetDescriptor();

        std::size_t Send(std::istream & is);
        std::size_t Send(const std::string & str);
        std::string Receive();
        std::size_t Receive(std::ostream & os);

        static AddrInfo Resolve(const char *host, unsigned short port, int af = AF_UNSPEC);
        static const char *GetAgent();

    private: // Preventive declarations
        SocketImpl(const SocketImpl &);
        SocketImpl & operator= (const SocketImpl &);

    private:
        struct Impl;
        std::auto_ptr<Impl> m_pimpl;
    };
}

#endif // WEBWATCH_INC_SOCKETIMPL_H