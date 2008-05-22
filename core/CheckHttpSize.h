// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpSize.h $ - HTTP size check method
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKHTTPSIZE_H
#define WEBWATCH_INC_CHECKHTTPSIZE_H

#include "CheckHttpMethod.h"

namespace Core_DE050401
{
    class CheckHttpSize : public CheckHttpMethod
    {
    private:
        virtual void CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient);
    };
}

#endif // WEBWATCH_INC_CHECKHTTPSIZE_H