// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckMethod.cpp $ - Check method implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/TimeUtil.h"

#include "CheckMethod.h"

// FUCKIN UGLY CODE
#include "CheckMethodFactory.h"
#include "CheckHttpSize.h"
#include "CheckFtp.h"
#include "CheckHttpTime.h"
#include "CheckHttpContent.h"

namespace Core_DE050401
{
    StatusNotifyClient::~StatusNotifyClient()
    {
    }

    CheckMethod::CheckMethod()
    : m_abort(false)
    {
    }

    CheckMethod::~CheckMethod()
    {
    }

    void CheckMethod::Check(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient)
    {
        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Checking");

        if (strcmp(parameters.GetTagName(), "CheckParameters") != 0)
            throw CheckMethodError("XML passed is not check parameters");

        results.SetTagName("CheckResults");
        return CheckImpl(address, parameters, results, statusNotifyClient);
    }

    void CheckMethod::Abort()
    {
        m_abort = true;
    }

    bool CheckMethod::ShouldAbort() const
    {
        return m_abort;
    }

    // FUCKIN UGLY CODE

    template<class T>
    CheckMethod *CreateNew()
    {
        return new T;
    }

    void CheckMethod::RegisterAllTypes()
    {
        CheckMethodFactory::Instance().Register("HttpSize", CreateNew<CheckHttpSize>);
        CheckMethodFactory::Instance().Register("Ftp", CreateNew<CheckFtp>);
        CheckMethodFactory::Instance().Register("HttpTime", CreateNew<CheckHttpTime>);
        CheckMethodFactory::Instance().Register("HttpContent", CreateNew<CheckHttpContent>);
    }
}
