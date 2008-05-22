// WebWatch Util - by DEATH, 2004
//
// $Workfile: AddressParser.h $ - URL address parser
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SITEADDRESS_H
#define WEBWATCH_INC_SITEADDRESS_H

namespace Util_DE050401
{
    class AddressParser
    {
    public:
        explicit AddressParser(const char *address = 0);

        void SetAddress(const char *address);
        const char *GetAddress() const;

        void SetUser(const char *user);
        const char *GetUser() const;

        void SetPassword(const char *password);
        const char *GetPassword() const;

        void SetHost(const char *host);
        const char *GetHost() const;

        void SetPort(unsigned short port);
        unsigned short GetPort() const;

        void SetFile(const char *file);
        const char *GetFile() const;

        void SetType(const char *type);
        const char *GetType() const;

    private:
        void MakeAddress() const;
        void SplitAddress();

    private:
        mutable std::string m_address; // http://flesh:beef@mysite.blah.org:666/blergh/moo.htm
        std::string m_type;         // http
        std::string m_user;         // flesh
        std::string m_password;     // beef
        std::string m_host;         // mysite.blah.org
        unsigned short m_port;      // 666
        std::string m_file;         // /blergh/moo.htm

        bool m_dirty;
    };
}

#endif // WEBWATCH_INC_SITEADDRESS_H