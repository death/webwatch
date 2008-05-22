// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpMethod.h $ - HTTP check method
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKHTTPMETHOD_H
#define WEBWATCH_INC_CHECKHTTPMETHOD_H

#include "util/Util.h"
#include "util/HttpParser.h"

#include "CheckMethod.h"

#define WEBWATCH_CHECKHTTPMETHOD_PERFORM_HEADER_CHECK(parser)   \
        switch (CheckHeader(parser)) {                          \
            case ContinueCheck:                                 \
                break;                                          \
            case NewLocation:                                   \
                {                                               \
                    const std::string & location = GetNewLocation();    \
                    sock.reset();                               \
                    return CheckImpl(location.c_str(), parameters, results, statusNotifyClient);    \
                }                                               \
            case StopCheck:                                     \
            default:                                            \
                return;                                         \
        }

namespace Core_DE050401
{
    class CheckHttpMethod : public CheckMethod
    {
    protected:
        CheckHttpMethod();

        enum CheckHeaderReturn
        {
            ContinueCheck, StopCheck, NewLocation
        };

        // If returns false, return from the check method
        CheckHeaderReturn CheckHeader(const Util::HttpParser & parser);
        std::string GetNewLocation() const;

    private:
        std::string m_newLocation;
        int m_tries;
    };
}

#endif // WEBWATCH_INC_CHECKHTTPMETHOD_H