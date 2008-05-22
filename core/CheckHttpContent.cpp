// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpContent.cpp $ - HTTP content check method implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "xml/XmlUtil.h"

#include "util/Util.h"
#include "util/HttpParser.h"
#include "util/Sha1.h"

#include "CheckHttpContent.h"
#include "HttpSocket.h"
#include "HttpSocketFactory.h"
#include "NumberStripper.h"
#include "HtmlStripper.h"

namespace Core_DE050401
{
    namespace 
    {
        void Filter(std::string & content, const XML::CXML & parameters);
        void SearchRegex(const std::string & content, const char *expression, XML::CXML & results);
    }

    CheckHttpContent::CheckHttpContent()
    {
    }
        
    void CheckHttpContent::CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient)
    {
        const XML::CXML *xmlSocket = parameters.FindFirstChild("Socket");
        if (xmlSocket == 0)
            throw CheckMethodError("Must supply socket information");
        std::auto_ptr<HttpSocket> sock = HttpSocketFactory::Instance().CreateObject(*this, *xmlSocket);

        if (ShouldAbort())
            return;

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Connecting");

        int res = sock->Connect(address);
        if (res != 0)
            throw CheckMethodError("Cannot connect");

        if (ShouldAbort())
            return;

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Sending request");

        std::string request = sock->CreateRequest(address, HttpSocket::get);
        if (sock->Send(request) != request.length())
            throw CheckMethodError("Cannot send request completely");

        if (ShouldAbort())
            return;

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Receiving data");

        std::string data = sock->Receive();

        if (ShouldAbort())
            return;

        if (data.empty())
            throw CheckMethodError("Receive returned with no data");

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Processing data");

        Util::HttpParser httpParser(data);
        WEBWATCH_CHECKHTTPMETHOD_PERFORM_HEADER_CHECK(httpParser);

        std::string content = httpParser.GetContent();
        Filter(content, parameters);

        const XML::CXML *mode = parameters.FindFirstChild("Mode");
        if (mode) {
            const char *type = mode->GetParamText("Type");
            if (strcmp(type, "GetContent") == 0) {
                std::string xmlContent = XML::MultilineToXML(content);
                results.CreateChild("Content", xmlContent.c_str());
            }
        }

    //    cout << content << endl;

        if (ShouldAbort())
            return;

        const XML::CXML *xmlRegex = parameters.FindFirstChild("Regex");
        if (xmlRegex) {
            while (xmlRegex) {
                const char *expression = xmlRegex->GetText();
                if (!*expression)
                    throw CheckMethodError("No regular expression text specified in Regex field in parameters");
                SearchRegex(content, expression, results);
                xmlRegex = parameters.FindNextChild("Regex");
            }
        } else {
            unsigned char digest[SHA1_SIGNATURE_LENGTH];
            SHA1_CTX sha1;
            SHA1Init(&sha1);
            SHA1Update(&sha1, content.c_str(), static_cast<int>(content.length()));
            SHA1Final(digest, &sha1);

            XML::CXML *xmlHash = results.CreateChild("Hash");
            xmlHash->SetParam("Type", "SHA1");

            std::string hash;
            for (int i = 0; i < SHA1_SIGNATURE_LENGTH; i++) {
                char digit[8];
                std::sprintf(digit, "%02X", digest[i]);
                hash += digit;
            }

            xmlHash->SetText(hash.c_str());
        }
    }

    namespace 
    {
        void Filter(std::string & content, const XML::CXML & parameters)
        {
            const XML::CXML *xmlFilter = parameters.FindFirstChild("Filter");
            while (xmlFilter) {
                const char *method = xmlFilter->GetParamText("Method");
                if (!*method)
                    throw CheckMethodError("No filter method for a given filter in parameters");

                std::istringstream input(content);
                std::ostringstream output;

                if (strcmp(method, "StripHtml") == 0) {
                    bool skipScript = xmlFilter->GetParamBool("SkipScript");
                    bool skipWhiteSpace = xmlFilter->GetParamBool("SkipWhiteSpace");
                    StripHtml(input, output, skipScript, skipWhiteSpace);
                } else if (strcmp(method, "StripNumbers") == 0) {
                    StripNumbers(input, output);
                } else {
                    std::string msg = "Unknown filter method: ";
                    msg += method;
                    throw CheckMethodError(msg);
                }

                content = output.str();

                xmlFilter = parameters.FindNextChild("Filter");
            }
        }

        void SearchRegex(const std::string & content, const char *expression, XML::CXML & results)
        {
            boost::regex regex(expression);
            std::string::const_iterator start = content.begin();
            std::string::const_iterator end = content.end();
            boost::match_results<std::string::const_iterator> what;
            boost::match_flag_type flags = boost::match_default;
            
            XML::CXML *xmlResult = results.CreateChild("RegexSearchResult");
            xmlResult->CreateChild("Regex", expression);

            while (boost::regex_search(start, end, what, regex, flags)) {
                std::string result(what[0].first, what[0].second);
                result = XML::MultilineToXML(result);
                xmlResult->CreateChild("Result", result.c_str());
                // Update search position
                start = what[0].second;
                // Update flags
                flags |= boost::match_prev_avail;
                flags |= boost::match_not_bob;
            }
        }
    }
}
