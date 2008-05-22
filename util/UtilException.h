// WebWatch Util - by DEATH, 2004
//
// $Workfile: UtilException.h $ - Util exceptions base class
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_UTILEXCEPTION_H
#define WEBWATCH_INC_UTILEXCEPTION_H

namespace Util_DE050401
{
    class Exception : public std::runtime_error
    {
    public:
        Exception(const std::string & error)
        : std::runtime_error(error)
        {
        }
    };
}

#endif // WEBWATCH_INC_UTILEXCEPTION_H