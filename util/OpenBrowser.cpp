// WebWatch Util - by DEATH, 2004
//
// $Workfile: OpenBrowser.cpp $ - OpenBrowser functor
//
// $Author: Death $
// $Revision: 3 $
// $Date: 5/07/05 20:03 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "OpenBrowser.h"

namespace 
{
    const char RegKey[] = "http\\shell\\open\\command";

    std::string BuildCommand(const std::string & cmd, 
                             const std::string & address);
}

namespace Util_DE050401
{
    OpenBrowser::OpenBrowser(HWND hwndParent)
    : m_hwndParent(hwndParent)
    {
        HKEY hkey;
        if (RegOpenKey(HKEY_CLASSES_ROOT, RegKey, &hkey) == ERROR_SUCCESS) {
            char path[_MAX_PATH + 1];
            LONG pathLen = _MAX_PATH;
            if (RegQueryValue(hkey, 0, path, &pathLen) == ERROR_SUCCESS)
                m_browser = path;
            RegCloseKey(hkey);
        }
    }

    bool OpenBrowser::operator() (const std::string & address) const
    {
        if (address.find("://") == std::string::npos) {
            std::string fullAddress("http://");
            fullAddress += address;
            return Open(fullAddress);
        } else {
            return Open(address);
        }
    }

    bool OpenBrowser::Open(const std::string & address) const
    {
        bool noBrowser = m_browser.empty();
        return noBrowser ? ShellMethod(address) : ProcessMethod(address);
    }

    bool OpenBrowser::ShellMethod(const std::string & address) const
    {
        // ShellExecute() has an utterly stupid return value convention.
        const int FailureUBound = 32;
        #pragma warning(push)
        #pragma warning(disable:4311)
        int result = reinterpret_cast<int>(ShellExecute(
            m_hwndParent, "open", address.c_str(), 0, 0, SW_SHOWNORMAL));
        #pragma warning(pop)
        return result > FailureUBound;
    }

    bool OpenBrowser::ProcessMethod(const std::string & address) const
    {
        bool result(false);
        
        const std::string & command = BuildCommand(m_browser, address);

        std::vector<char> cmd(command.begin(), command.end());
        cmd.push_back('\0');

        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        GetStartupInfo(&si);
        BOOL ok = CreateProcess(0, &cmd[0], 0, 0, FALSE, 0, 0, 0, &si, &pi);

        if (ok) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            result = true;
        }

        return result;
    }
}

namespace 
{
    // Firefox has %1 that needs to be replaced by the URL, IE wants the
    // string appended (Firefox has strange behaviour?)
    std::string BuildCommand(const std::string & cmd, 
                             const std::string & address)
    {
        std::string result(cmd);
        
        std::string::size_type i = cmd.find("%1");
        if (i == std::string::npos) {
            result += ' ';
            result += address;
        } else {
            result.replace(i, 2, address);
        }
        
        return result;
    }
}
