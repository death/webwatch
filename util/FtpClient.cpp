// WebWatch Util - by DEATH, 2004
//
// $Workfile: FtpClient.cpp $ - Very simple FTP client implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "FtpClient.h"
#include "AddressParser.h"
#include "TimeUtil.h"

namespace Util_DE050401
{
    FtpClient::FtpClient(const char *agent)
    : m_hSession(0)
    , m_hConnect(0)
    , m_hFind(0)
    {
        m_hSession = InternetOpen(agent, INTERNET_OPEN_TYPE_DIRECT, 0, 0, 0);
    }

    FtpClient::~FtpClient()
    {
        CloseFind();

        if (m_hConnect)
            InternetCloseHandle(m_hConnect);
        
        if (m_hSession)
            InternetCloseHandle(m_hSession);
    }

    int FtpClient::Connect(const char *address)
    {
        AddressParser parser(address);
        m_hConnect = InternetConnect(m_hSession, parser.GetHost(), parser.GetPort(), parser.GetUser(), parser.GetPassword(), INTERNET_SERVICE_FTP, 0, 0);
        if (m_hConnect == 0)
            return -1;

        return 0;
    }

    bool FtpClient::FirstFile(const char *filename, FtpFileInformation & info)
    {
        if (m_hConnect == 0)
            return false;

        CloseFind();

        m_hFind = (HINTERNET )FtpFindFirstFile(m_hConnect, filename, &m_findData, INTERNET_FLAG_DONT_CACHE, 0);
        if (m_hFind == NULL)
            return false;

        UpdateFileInformation(info);

        return true;
    }

    bool FtpClient::NextFile(FtpFileInformation & info)
    {
        if (m_hFind == 0)
            return false;

        if (InternetFindNextFile(m_hFind, &m_findData) == FALSE) {
            CloseFind();
            return false;
        }

        UpdateFileInformation(info);

        return true;
    }

    void FtpClient::CloseFind()
    {
        if (m_hFind) {
            InternetCloseHandle(m_hFind);
            m_hFind = 0;
        }
    }

    void FtpClient::UpdateFileInformation(FtpFileInformation & info)
    {
        info.name = m_findData.cFileName;
        info.size = m_findData.nFileSizeLow;
        info.writeTime = 0;

        SYSTEMTIME st;
        if (FileTimeToSystemTime(&m_findData.ftLastWriteTime, &st) == TRUE)
            info.writeTime = SystemTimeToTime(st, true);
    }
}
