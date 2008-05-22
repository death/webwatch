// WebWatch Util - by DEATH, 2004
//
// $Workfile: FtpClient.h $ - A very simple FTP client interface
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_FTPCLIENT_H
#define WEBWATCH_INC_FTPCLIENT_H

namespace Util_DE050401
{
    struct FtpFileInformation
    {
        std::string name;
        std::time_t writeTime;
        std::size_t size;
    };

    class FtpClient
    {
    public:
        FtpClient(const char *agent);
        ~FtpClient();

        int Connect(const char *address);

        bool FirstFile(const char *filename, FtpFileInformation & info);
        bool NextFile(FtpFileInformation & info);
        void CloseFind();

    private:
        FtpClient(const FtpClient &);
        FtpClient & operator= (const FtpClient &);

    private:
        void UpdateFileInformation(FtpFileInformation & info);

    private:
        HINTERNET m_hSession;
        HINTERNET m_hConnect;
        WIN32_FIND_DATA m_findData;
        HINTERNET m_hFind;
    };
}

#endif // WEBWATCH_INC_FTPCLIENT_H
