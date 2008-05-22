// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckFtp.cpp $ - FTP check method implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
// NOTE
// This module is supposed to make use of an IdentServer if an ident
// string is specified in the parameters (<Ident>DEATH</Ident>), but since we
// use lame WinInet, we don't know what client port we channel through, or can
// even start the IdentServer in a reasonable point of time.
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/AddressParser.h"
#include "util/HttpParser.h"
#include "util/FtpClient.h"
#include "util/TimeUtil.h"

#include "CheckMethodFactory.h"
#include "CheckFtp.h"
#include "HttpSocket.h"

namespace Core_DE050401
{
    void CheckFtp::CheckImpl(const char *address, const XML::CXML & parameters, XML::CXML & results, StatusNotifyClient *statusNotifyClient)
    {
        Util::FtpClient client(HttpSocket::GetAgent());

        if (ShouldAbort())
            return;

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Connecting");

        if (client.Connect(address) != 0)
            throw CheckMethodError("Cannot connect");

        if (ShouldAbort())
            return;

        Util::AddressParser parser(address);

        if (statusNotifyClient)
            statusNotifyClient->SetStatus("Scanning files");

        Util::FtpFileInformation fileInfo;
        bool found = client.FirstFile(parser.GetFile(), fileInfo);
        if (found == false)
            throw CheckMethodError("File not found");

        if (ShouldAbort())
            return;

        size_t count = 0;
        time_t chosenWriteTime = 0;
        size_t chosenSize = 0;

        while (found) {
            if (count == 0 || (difftime(fileInfo.writeTime, chosenWriteTime) > 0.0)) {
                chosenWriteTime = fileInfo.writeTime;
                chosenSize = fileInfo.size;
            }

            found = client.NextFile(fileInfo);
            count++;

            if (ShouldAbort())
                return;
        }

        results.SetParam("Type", "ftp");

        std::ostringstream os;
        os << static_cast<unsigned int>(chosenSize);
        results.CreateChild("Size", os.str().c_str());

        struct tm *gmt = gmtime(&chosenWriteTime);
        if (gmt)
            Util::CreateXMLTimeNode(results, "Time", *gmt);
    }
}
