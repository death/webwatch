// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckMethod.h $ - Check method abstract base class
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKMETHOD_H
#define WEBWATCH_INC_CHECKMETHOD_H

#include "SocketImpl.h"
#include "CoreException.h"

namespace Core_DE050401
{
    class StatusNotifyClient
    {
    public:
        virtual ~StatusNotifyClient();
        virtual void SetStatus(const char *status) = 0;
    };

    class CheckMethod : public AbortIndicator
    {
    public:
        CheckMethod();
        virtual ~CheckMethod();

        virtual bool ShouldAbort() const;

        void Check(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient);
        void Abort();

        static void RegisterAllTypes();

    private:
        virtual void CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient) = 0;

    private: // Preventive declarations
        CheckMethod(const CheckMethod &);
        CheckMethod & operator= (const CheckMethod &);

    private:
        bool m_abort;
    };

    class CheckMethodError : public Exception
    {
    public:
        CheckMethodError(const std::string & error)
        : Exception(error)
        {
        }
    };
}

#endif // WEBWATCH_INC_CHECKMETHOD_H