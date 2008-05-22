// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpSocket.h $ - HTTP socket
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_HTTPSOCKET_H
#define WEBWATCH_INC_HTTPSOCKET_H

#include "SocketImpl.h"

namespace Core_DE050401
{
    class HttpSocket
    {
    public: // Enumerations
        enum RequestMethod {
            get, head
        };

    public: // Methods
        // Transfers ownership
        HttpSocket(std::auto_ptr<SocketImpl> socketImpl);
        virtual ~HttpSocket();

        SOCKET GetDescriptor();

        int Connect(const char *address);

        std::size_t Send(std::istream & is);
        std::size_t Send(const void *data, std::size_t size);
        std::size_t Send(const std::string & str);

        std::size_t Receive(void *data, std::size_t size);
        std::string Receive();
        std::size_t Receive(std::ostream & os);

        std::string CreateRequest(const char *address, RequestMethod method, bool finalize = true);

        static const char *GetAgent();

    private: // Preventive declarations
        HttpSocket(const HttpSocket &);
        HttpSocket & operator= (const HttpSocket &);

    protected:
        static AddrInfo Resolve(const char *host, unsigned short port, int af = AF_UNSPEC);

        virtual int ConnectImpl(const char *address);
        virtual std::string CreateRequestImpl(const char *address, RequestMethod method, bool finalize);

    private:
        struct Impl;
        std::auto_ptr<Impl> m_pimpl;
    };
}

#endif // WEBWATCH_INC_HTTPSOCKET_H
