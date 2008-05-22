// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpTime.cpp $ - HTTP time check method implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/HttpParser.h"
#include "util/TimeUtil.h"

#include "CheckHttpTime.h"
#include "HttpSocket.h"
#include "HttpSocketFactory.h"

namespace Core_DE050401
{
    void CheckHttpTime::CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient)
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
            throw CheckMethodError("Can't connect");

        if (ShouldAbort())
            return;

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Sending request");

        std::string request = sock->CreateRequest(address, HttpSocket::head, true);
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

        if (httpParser.IsTimeFound() == false)
            throw CheckMethodError("Time field unspecified");

        time_t siteTime = httpParser.GetTime();

        struct tm *gmt = gmtime(&siteTime);
        if (gmt == 0)
            throw CheckMethodError("Bad time for GMT conversion");

        Util::CreateXMLTimeNode(results, "Time", *gmt);
    }
}
