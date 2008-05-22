// WebWatch Util - by DEATH, 2004
//
// $Workfile: HttpParser.h $ - HTTP header parser
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_HTTPPARSER_H
#define WEBWATCH_INC_HTTPPARSER_H

namespace Util_DE050401
{
    class HttpParser
    {
    public:
        explicit HttpParser(const std::string & packet);

        bool IsSuccess() const;
        bool IsSizeFound() const;
        bool IsTimeFound() const;
        int GetResultCode() const;

        std::size_t GetSize() const;
        std::time_t GetTime() const;

        std::string GetValue(const std::string & key) const;
        std::string GetContent() const;

    private:
        HttpParser(const HttpParser &);
        HttpParser & operator= (const HttpParser &);

    private:
        void ParseSize(const std::string & s);
        void ParseDate(const std::string & s);

    private:
        bool m_success;
        bool m_sizeFound;
        bool m_timeFound;
        std::size_t m_size;
        std::time_t m_time;
        int m_resultCode;
        std::string m_content;

        typedef std::map<std::string, std::string> ValueMap;
        ValueMap m_values;
    };
}

#endif // WEBWATCH_INC_HTTPPARSER_H
