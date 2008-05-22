// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpMethod.cpp $ - HTTP time check method implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/HttpParser.h"
#include "util/ThreadUtil.h"

#include "CheckHttpMethod.h"

namespace 
{
    const int MaxTries = 5;
    
    typedef std::map<int, std::string> ErrorMap;
    ErrorMap errorMap;
    Util::CriticalSection errorMapCS;
}

namespace Core_DE050401
{
    CheckHttpMethod::CheckHttpMethod()
    : m_tries(0)
    {
    }

    CheckHttpMethod::CheckHeaderReturn CheckHttpMethod::CheckHeader(const Util::HttpParser & parser)
    {
        if (parser.IsSuccess() == false)
            throw CheckMethodError("Could not parse HTTP header");

        if (m_tries >= MaxTries)
            throw CheckMethodError("Maximal check tries limit exceeded");

        std::ostringstream msg;
        if (parser.GetResultCode() < 200) {
            msg << "Bad HTTP code: " << parser.GetResultCode();
            throw CheckMethodError(msg.str().c_str());
        }

        if (ShouldAbort())
            return StopCheck;

        {
            // DCLP - C++ rules don't guarantee correct functioning
            if (errorMap.empty()) {
                Util::ScopedLock lock(errorMapCS);
                if (errorMap.empty()) {
                    errorMap.insert(std::make_pair(400, "Bad request"));
                    errorMap.insert(std::make_pair(401, "Unauthorized"));
                    errorMap.insert(std::make_pair(402, "Payment required"));
                    errorMap.insert(std::make_pair(403, "Forbidden"));
                    errorMap.insert(std::make_pair(404, "Not found"));
                    errorMap.insert(std::make_pair(405, "Method not allowed"));
                    errorMap.insert(std::make_pair(406, "Not acceptable"));
                    errorMap.insert(std::make_pair(407, "Proxy authentication required"));
                    errorMap.insert(std::make_pair(408, "Request timeout"));
                    errorMap.insert(std::make_pair(409, "Conflict"));
                    errorMap.insert(std::make_pair(410, "Gone"));
                    errorMap.insert(std::make_pair(411, "Length required"));
                    errorMap.insert(std::make_pair(412, "Precondition failed"));
                    errorMap.insert(std::make_pair(413, "Request entity too large"));
                    errorMap.insert(std::make_pair(414, "Request URI too long"));
                    errorMap.insert(std::make_pair(415, "Unsupported media type"));
                    errorMap.insert(std::make_pair(416, "Requested range not satisfiable"));
                    errorMap.insert(std::make_pair(417, "Expectation failed"));
                    errorMap.insert(std::make_pair(500, "Internal server error"));
                    errorMap.insert(std::make_pair(501, "Not implemented"));
                    errorMap.insert(std::make_pair(502, "Bad gateway"));
                    errorMap.insert(std::make_pair(503, "Service unavailable"));
                    errorMap.insert(std::make_pair(504, "Gateway timeout"));
                    errorMap.insert(std::make_pair(505, "HTTP version not supported"));
                }
            }
        }

        int resultCode = parser.GetResultCode();
        
        ErrorMap::const_iterator err = errorMap.find(resultCode);
        if (err != errorMap.end()) {
            msg << err->second << " (" << resultCode << ")";
            throw CheckMethodError(msg.str());
        }

        switch (resultCode) {
            case 301:
                // Moved permanently
                msg << "Website moved permanently to " << parser.GetValue("Location");
                throw CheckMethodError(msg.str());
            case 302:
                // Moved temporarily
                {
                    const std::string & location = parser.GetValue("Location");
                    if (location.empty()) {
                        msg << "Website temporarily moved to somewhere but won't specify where";
                        throw CheckMethodError(msg.str());
                    }

                    m_tries++;

                    m_newLocation = location;
                    return NewLocation;
                }
            case 304:
                // Not modified
                return StopCheck;
            default:
                if (resultCode < 200 || resultCode > 299) {
                    msg << "Weird result code: " << parser.GetResultCode();
                    throw CheckMethodError(msg.str());
                }
                break;
        }

        return ContinueCheck;
    }

    std::string CheckHttpMethod::GetNewLocation() const
    {
        return m_newLocation;
    }
}
