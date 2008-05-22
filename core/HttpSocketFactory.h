// WebWatch Core - by DEATH, 2004
//
// $Workfile: HttpSocketFactory.h $ - HTTP socket factory
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_HTTPSOCKETFACTORY_H
#define WEBWATCH_INC_HTTPSOCKETFACTORY_H

#include "loki/Singleton.h"

namespace Core_DE050401
{
    class HttpSocket;
    class AbortIndicator;

    class HttpSocketFactoryImpl
    {
    public:
        typedef std::vector<std::string> ProxyTypes;

        HttpSocketFactoryImpl();
        ~HttpSocketFactoryImpl();

        std::auto_ptr<HttpSocket> CreateObject(const AbortIndicator & abortIndicator, const XML::CXML & parameters);
        ProxyTypes GetProxyTypes() const;
    };

    typedef Loki::SingletonHolder<HttpSocketFactoryImpl> HttpSocketFactory;
}

#endif // WEBWATCH_INC_HTTPSOCKETFACTORY_H