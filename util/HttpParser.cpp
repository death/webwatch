// WebWatch Util - by DEATH, 2004
//
// $Workfile: HttpParser.cpp $ - HTTP header parser implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "HttpParser.h"
#include "TimeUtil.h"
#include "EmptyString.h"

namespace algo = boost::algorithm;

namespace Util_DE050401
{
    HttpParser::HttpParser(const std::string & header)
    : m_time(0)
    , m_success(true)
    , m_sizeFound(false)
    , m_timeFound(false)
    , m_resultCode(0)
    {
        std::istringstream strm(header);

        // Get first line
        std::string line;
        getline(strm, line);

        // Check version
        std::string httpVersion = line.substr(0, 9);
        httpVersion[7] = '0';
        if (httpVersion != "HTTP/1.0 ") {
            m_success = false;
            return;
        }

        // Check result code
        m_resultCode = boost::lexical_cast<int>(line.substr(9, 3));
        if (m_resultCode == 0) {
            m_success = false;
            return;
        }

        while (strm.good()) {
            getline(strm, line);
            if (line.empty() || line[0] == '\r' || line[0] == '\n')
                break;
            
            std::string::size_type sep = line.find(':');
            if (sep != std::string::npos) {
                // insert key:value into map
                std::string key(line.begin(), line.begin() + sep);
                algo::trim(key);
                algo::to_lower(key);

                std::string value(line.begin() + sep + 1, line.end());
                algo::trim(value);

                m_values[key] = value;
                
                if (key == "last-modified") {
                    m_timeFound = true;
                    ParseDate(value.c_str());
                } else if (key == "content-length") {
                    m_sizeFound = true;
                    ParseSize(value.c_str());
                }
            }
        }

        if (strm.good()) {
            typedef std::istreambuf_iterator<char> Iterator;
            std::copy(Iterator(strm), Iterator(), std::back_inserter(m_content));
        }
    }

    bool HttpParser::IsSuccess() const
    {
        return m_success;
    }

    bool HttpParser::IsSizeFound() const
    {
        return m_sizeFound;
    }

    bool HttpParser::IsTimeFound() const
    {
        return m_timeFound;
    }

    size_t HttpParser::GetSize() const
    {
        return m_size;
    }

    time_t HttpParser::GetTime() const
    {
        return m_time;
    }

    void HttpParser::ParseSize(const std::string & s)
    {
        m_size = boost::lexical_cast<std::size_t>(s);
    }

    void HttpParser::ParseDate(const std::string & s)
    {
        static const char *month[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", 0
        };

        // month entries string lengths
        static const int monthLen = 3;

        static const char *wkday[] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", 0
        };

        // wkday entries string lengths
        static const int wkdayLen = 3;

        static const char *weekday[] = {
            "Sunday", "Monday", "Tuesday", "Wednesday", 
            "Thursday", "Friday", "Saturday", 0
        };

        // weekday entries string lengths
        static const size_t weekdayLen[] = {
            6, 6, 7, 9, 8, 6, 8
        };

        // Date format determination and decoding
        bool is_rfc1123 = false;
        bool is_rfc850 = false;
        bool is_asctime = false;
        const char *str = s.c_str();
        const char *next = 0;

        struct tm tm = {0};

        for (int i = 0; weekday[i]; i++) {
            if (strncmp(weekday[i], str, weekdayLen[i]) == 0) {
                is_rfc850 = true;
                tm.tm_wday = i;
                next = str + weekdayLen[i] + 2;
                break;
            }
        }

        if (is_rfc850 == false) {
            // Either asctime or rfc1123
            int i;
            for (i = 0; wkday[i]; i++) {
                if (strncmp(wkday[i], str, wkdayLen) == 0) {
                    tm.tm_wday = i;
                    next = str + wkdayLen + 1;
                    break;
                }
            }

            // Error decoding
            if (wkday[i] == 0) {
                m_timeFound = false;
                return;
            }

            if (next[0] == ' ') {
                is_rfc1123 = true;
                next++;
            } else {
                is_asctime = true;
            }
        }

        if (is_rfc1123 == true) {
            // Decode day
            tm.tm_mday = (next[0] - '0') * 10 + (next[1] - '0');
            next += 3;

            // Decode month
            int i;
            for (i = 0; month[i]; i++) {
                if (strncmp(month[i], next, monthLen) == 0) {
                    tm.tm_mon = i;
                    next += monthLen + 1;
                    break;
                }
            }

            if (month[i] == 0) {
                m_timeFound = false;
                return;
            }

            // Decode year
            if (strlen(next) < 4) {
                m_timeFound = false;
                return;
            }

            int year = (next[0] - '0') * 1000;
            year += (next[1] - '0') * 100;
            year += (next[2] - '0') * 10;
            year += (next[3] - '0');

            tm.tm_year = year - 1900;

            next += 5;
        } else if (is_asctime == true) {
            // Decode month
            for (int i = 0; month[i]; i++) {
                if (strncmp(month[i], next, monthLen) == 0) {
                    tm.tm_mon = i;
                    next += monthLen + 1;
                    break;
                }
            }

            // Decode day
            if (next[0] == ' ') {
                tm.tm_wday = next[1] - '0';
            } else {
                tm.tm_wday = (next[0] - '0') * 10 + (next[1] - '0');
            }

            // No year
		    time_t now = time(0);
            struct tm *gmt = gmtime(&now);
            tm.tm_year = gmt ? gmt->tm_year : (2000 - 1900); // Evil, evil code!
        } else if (is_rfc850 == true) {
            // Decode day
            tm.tm_wday = (next[0] - '0') * 10 + (next[1] - '0');
            next += 3;

            // Decode month
            int i;
            for (i = 0; month[i]; i++) {
                if (strncmp(month[i], next, monthLen) == 0) {
                    tm.tm_mon = i;
                    next += monthLen + 1;
                    break;
                }
            }

            if (month[i] == 0) {
                m_timeFound = false;
                return;
            }

            // Decode year
            if (strlen(next) < 2) {
                m_timeFound = false;
                return;
            }

            // Short year form
            int year = (next[0] - '0') * 10 + (next[1] - '0') + 2000;
            tm.tm_year = year - 1900;

            next += 3;
        }

        // Decode time
        int hh = 0;
        int mm = 0;
        int ss = 0;
        sscanf(next, "%d:%d:%d", &hh, &mm, &ss);
        tm.tm_hour = hh;
        tm.tm_min = mm;
        tm.tm_sec = ss;

        GmtToLocal(tm);
        m_time = mktime(&tm);
    }

    int HttpParser::GetResultCode() const
    {
        return m_resultCode;
    }

    std::string HttpParser::GetValue(const std::string & key) const 
    {
        ValueMap::const_iterator i = m_values.find(algo::to_lower_copy(key));
        return i != m_values.end() ? i->second : EmptyString();
    }

    std::string HttpParser::GetContent() const
    {
        return m_content;
    }
}
