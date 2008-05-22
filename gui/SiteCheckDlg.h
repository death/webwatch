// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteCheckDlg.h $ - Site check dialog
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#pragma once

#include "core/Core.h"
#include "core/CoreException.h"

#include "Resource.h"

namespace WebWatch
{
    class SiteItem;
}

class CSiteCheckDlg : public CDialog
{
public:
    typedef std::vector<WebWatch::SiteItem *> SiteItems;

    class Hook
    {
    protected:
        Hook() {}

    public:
        virtual ~Hook() {}
        virtual void OnSiteChecked(WebWatch::SiteItem & site, const XML::CXML & results) {};
        virtual void OnException(WebWatch::SiteItem & site, const Core::Exception & ex) {};
        virtual void OnAllSitesChecked(CWnd & wnd) {}
    };

public:
    CSiteCheckDlg(const SiteItems & sites, Hook & hook, CWnd *parent = 0);

    enum { IDD = IDD_SITE_CHECK };

private:
    virtual void DoDataExchange(CDataExchange *pDX);
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
    afx_msg void OnAbort();
    afx_msg void OnMinimize();

    DECLARE_MESSAGE_MAP()

private:
    UINT Check();
    static UINT CheckThread(LPVOID param);

    UINT CheckSite(WebWatch::SiteItem & site, int threadNumber);
    static UINT CheckSiteThread(LPVOID param);

private:
    SiteItems m_sites;
    Hook & m_hook;
    volatile bool m_abort;
    CListCtrl m_checkerList;
};
