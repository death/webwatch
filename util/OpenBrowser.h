// WebWatch Util - by DEATH, 2004
//
// $Workfile: OpenBrowser.h $ - OpenBrowser functor
//
// $Author: Death $
// $Revision: 3 $
// $Date: 5/07/05 20:03 $
// $NoKeywords: $
//
#ifndef WEBWATCH_UTIL_INC_OPENBROWSER_H
#define WEBWATCH_UTIL_INC_OPENBROWSER_H

namespace Util_DE050401
{
    class OpenBrowser
    {
    public:
        explicit OpenBrowser(HWND hwndParent);
        bool operator() (const std::string & address) const;

    private:
        bool Open(const std::string & address) const;
        bool ShellMethod(const std::string & address) const;
        bool ProcessMethod(const std::string & address) const;

    private:
        std::string m_browser;
        HWND m_hwndParent;
    };
}

#endif // WEBWATCH_UTIL_INC_OPENBROWSER_H
