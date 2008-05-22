// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpContent.h $ - HTTP content check method
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKHTTPCONTENT_H
#define WEBWATCH_INC_CHECKHTTPCONTENT_H

#include "CheckHttpMethod.h"

namespace Core_DE050401
{
    class CheckHttpContent : public CheckHttpMethod
    {
    public:
        CheckHttpContent();

    private:
        virtual void CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient);
    };
}

#endif // WEBWATCH_INC_CHECKHTTPCONTENT_H