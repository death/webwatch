// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckHttpSize.cpp $ - HTTP size method implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/HttpParser.h"

#include "CheckHttpSize.h"
#include "HttpSocketFactory.h"
#include "HttpSocket.h"

namespace Core_DE050401
{
    void CheckHttpSize::CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient)
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

        if (httpParser.IsSizeFound() == false)
            throw CheckMethodError("Size field unspecified");

        size_t size = httpParser.GetSize();
        std::ostringstream os;
        os << static_cast<unsigned int>(size);
        results.CreateChild("Size", os.str().c_str());
    }
}
