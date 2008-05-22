// WebWatch Util - by DEATH, 2004
//
// $Workfile: TimeUtil.cpp $ - WebWatch time utilities implementation
//
// $Author: Death $
// $Revision: 4 $
// $Date: 10/24/05 18:28 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "xml/Xml.h"

#include "TimeUtil.h"
#include "UtilException.h"

namespace Util_DE050401
{
    namespace
    {
        //
        // This is evil. According to Plauger, the only safe way to compute
        // the difference between two time_ts is to use difftime; he also
        // says one should never perform arithmetic with time_ts (although
        // they are arithmetic values). Needs a better solution.
        //
        time_t Difference()
        {
            static time_t result = -1;
            if (result == -1) {
                time_t l = time(0);
                struct tm *p = gmtime(&l);
                if (!p)
                    throw Exception("gmtime() failed in difference()");
                time_t g = mktime(p);
                result = l - g;
            }
            return result;
        }
    }

    void GmtToLocal(struct tm & tm)
    {
        time_t t = mktime(&tm) + Difference();
        struct tm *p = localtime(&t);
        if (!p)
            throw Exception("localtime() failed in gmt_to_local()");
        tm = *p;
    }

    time_t SystemTimeToTime(const SYSTEMTIME & st, bool gmt)
    {
        struct tm tm;

        tm.tm_isdst = 0;
        tm.tm_year = st.wYear - 1900;
        tm.tm_mon = st.wMonth - 1;
        tm.tm_mday = st.wDay;
        tm.tm_hour = st.wHour;
        tm.tm_min = st.wMinute;
        tm.tm_sec = st.wSecond;
        
        if (gmt)
            GmtToLocal(tm);

        return mktime(&tm);
    }

    XML::CXML *CreateXMLTimeNode(XML::CXML & xml, const char *name, const struct tm & tm)
    {
        XML::CXML *xmlTime = xml.CreateChild(name);
        if (xmlTime) {
            xmlTime->SetParam("Year", tm.tm_year + 1900);
            xmlTime->SetParam("Month", tm.tm_mon + 1);
            xmlTime->SetParam("Day", tm.tm_mday);
            xmlTime->SetParam("Hour", tm.tm_hour);
            xmlTime->SetParam("Minute", tm.tm_min);
            xmlTime->SetParam("Second", tm.tm_sec);
        }
        return xmlTime;
    }

    // If name == 0, the XML is a time node itself
    void ParseXMLTimeNode(const XML::CXML & xml, const char *name, struct tm & tm)
    {
        const XML::CXML *xmlTime = name ? xml.FindFirstChild(name) : &xml;
        if (xmlTime) {
            tm.tm_year = xmlTime->GetParamInt("Year", 1900) - 1900;
            tm.tm_mon = xmlTime->GetParamInt("Month", 1) - 1;
            tm.tm_mday = xmlTime->GetParamInt("Day", 0);
            tm.tm_hour = xmlTime->GetParamInt("Hour");
            tm.tm_min = xmlTime->GetParamInt("Minute");
            tm.tm_sec = xmlTime->GetParamInt("Second");
        } else {
            throw Exception("Unable to find XML time node");
        }
    }
}
