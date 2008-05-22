// WebWatch Util - by DEATH, 2004
//
// $Workfile: AddressParser.cpp $ - URL address parser implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/20/05 3:12 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "AddressParser.h"

namespace Util_DE050401
{
    AddressParser::AddressParser(const char *address)
    {
        SetAddress(address);
    }

    void AddressParser::SetAddress(const char *address)
    {
        if (address == 0)
            m_address.clear();
        else
            m_address = address;

        SplitAddress();

        m_dirty = false;
    }

    const char *AddressParser::GetAddress() const
    {
        if (m_dirty)
            MakeAddress();
        return m_address.c_str();
    }

    void AddressParser::SetUser(const char *user)
    {
        if (user == 0)
            m_user.clear();
        else
            m_user = user;
        
        m_dirty = true;
    }

    const char *AddressParser::GetUser() const
    {
        return m_user.c_str();
    }

    void AddressParser::SetPassword(const char *password)
    {
        if (password == 0)
            m_password.clear();
        else
            m_password = password;
        
        m_dirty = true;
    }

    const char *AddressParser::GetPassword() const
    {
        return m_password.c_str();
    }

    void AddressParser::SetHost(const char *host)
    {
        if (host == 0)
            m_host.clear();
        else
            m_host = host;

        m_dirty = true;
    }

    const char *AddressParser::GetHost() const
    {
        return m_host.c_str();
    }

    void AddressParser::SetPort(unsigned short port)
    {
        m_port = port;
        m_dirty = true;
    }

    unsigned short AddressParser::GetPort() const
    {
        return m_port;
    }

    void AddressParser::SetFile(const char *file)
    {
        if (file == 0)
            m_file.clear();
        else
            m_file = file;

        m_dirty = true;
    }

    const char *AddressParser::GetFile() const
    {
        return m_file.c_str();
    }

    void AddressParser::SetType(const char *type)
    {
        if (type == 0)
            m_type.clear();
        else
            m_type = type;

        m_dirty = true;
    }

    const char *AddressParser::GetType() const
    {
        return m_type.c_str();
    }

    void AddressParser::SplitAddress()
    {
        m_user.clear();
        m_password.clear();
        m_host.clear();
        m_port = 80;
        m_file = "/";

        if (m_address.empty()) {
            m_type = "unknown";
            return;
        }

        std::string::const_iterator i = m_address.begin();
        std::string::const_iterator addressBegin = i;
        std::string::const_iterator addressEnd = m_address.end();

        // Check if a tag is supplied with the address, and what tag is it
        // Also set a default port
        std::string::size_type tagPos = m_address.find("://");
        if (tagPos != std::string::npos) {
            m_type.assign(addressBegin, addressBegin + tagPos);
            boost::to_lower(m_type);
            if (m_type == "ftp") {
                m_port = 21;
            } else if (m_type == "https") {
                m_port = 443;
            }

            i += tagPos + 3;
            
            if (i == addressEnd)
                return;
        } else {
            m_type = "http";
        }

        // Check for user/password
        std::string::size_type credEnd = m_address.find('@', i - addressBegin);
        if (credEnd != std::string::npos) {
            std::string::size_type credSep = m_address.find(':', i - addressBegin);
            if (credSep == std::string::npos || credSep > credEnd) {
                // Only user specified
                m_user.assign(i, addressBegin + credEnd);
            } else {
                // Both user and password specified
                m_user.assign(i, addressBegin + credSep);
                m_password.assign(addressBegin + credSep + 1, addressBegin + credEnd);
            }
            i = addressBegin + credEnd + 1;
        }

        // Assign host
        std::string::size_type slash = m_address.find('/', i - addressBegin);
        std::string::size_type portBegin = m_address.find(':', i - addressBegin);
        std::string::size_type hostEnd = std::min(slash, portBegin);
        if (hostEnd != std::string::npos) {
            m_host.assign(i, addressBegin + hostEnd);
            i = addressBegin + hostEnd;
        } else {
            // The rest of the address is the host
            m_host.assign(i, addressEnd);
            i = addressEnd;
        }

        if (i == addressEnd)
            return;

        // Assign port
        if (*i == ':') {
            std::string::const_iterator portStart = ++i;
            if (i != addressEnd) {
                do {
                    if (!std::isdigit(*i))
                        break;
                } while (++i != addressEnd);

                std::string port(portStart, i);
                std::istringstream is(port);
                is >> m_port;
            }
        }

        // Assign rest to file
        if (i != addressEnd)
            m_file.assign(i, addressEnd);
    }

    void AddressParser::MakeAddress() const
    {
        std::ostringstream os;

        if (m_type.empty() == false)
            os << m_type << "://"; // http://

        if (m_user.empty() == false) {
            os << m_user; // flesh
            if (m_password.empty() == false) {
                os << ':'; // :
                os << m_password; // beef
            }
            os << '@'; // @
        }

        if (m_host.empty() == false) {
            os << m_host; // mysite.blah.org
            os << ':'; // :
            os << m_port; // 666
        }

        os << m_file; // /blergh/moo.htm

        m_address = os.str();
    }
}
