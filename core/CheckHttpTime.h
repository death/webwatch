// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpTime.h $ - HTTP time check method
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKHTTPTIME_H
#define WEBWATCH_INC_CHECKHTTPTIME_H

#include "CheckHttpMethod.h"

namespace Core_DE050401
{
    class CheckHttpTime : public CheckHttpMethod
    {
    private:
        virtual void CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient);
    };
}

#endif // WEBWATCH_INC_CHECKHTTPTIME_H