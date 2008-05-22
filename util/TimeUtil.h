// WebWatch Util - by DEATH, 2004
//
// $Workfile: TimeUtil.h $ - WebWatch time utilities
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_TIMEUTIL_H
#define WEBWATCH_INC_TIMEUTIL_H

namespace Util_DE050401
{
    XML::CXML *CreateXMLTimeNode(XML::CXML & xml, const char *name, const struct std::tm & tm);
    void GmtToLocal(struct std::tm & tm);
    std::time_t SystemTimeToTime(const SYSTEMTIME & st, bool gmt);
    void ParseXMLTimeNode(const XML::CXML & xml, const char *name, struct std::tm & tm);
}

#endif // WEBWATCH_INC_TIMEUTIL_H