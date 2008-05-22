// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteMUPage.cpp $ - Site MU property page
//
// $Author: Death $
// $Revision: 5 $
// $Date: 28/05/04 19:09 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SiteMUPage.h"
#include "SitePropertiesDlg.h"
#include "SiteItem.h"
#include "SetMUDlg.h"
#include "util/TimeUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSiteMUPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSiteMUPage, CPropertyPage)
    ON_BN_CLICKED(IDC_MU_SESSION_BUTTON, OnSessionButtonClicked)
END_MESSAGE_MAP()

CSiteMUPage::CSiteMUPage(CSitePropertiesDlg & owner, WebWatch::SiteItem & site)
: CPropertyPage(CSiteMUPage::IDD)
, m_owner(&owner)
, m_site(site)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "MU";
}

CSiteMUPage::~CSiteMUPage()
{
}

void CSiteMUPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSiteMUPage::OnSetActive()
{
    ShowInfo();
    return CPropertyPage::OnSetActive();
}

void CSiteMUPage::OnOK()
{
    CPropertyPage::OnOK();
}

void CSiteMUPage::ShowInfo()
{
    if (m_site.IsMUInfoAvailable() == false) {
        SetDlgItemText(IDC_MU_INFO, "No Minor Update session");
        SetDlgItemText(IDC_MU_SESSION_BUTTON, "Begin session...");
        return;
    }

    time_t now = time(0);
    int days = m_site.GetMUDayCount(now);
    time_t expiry = now + days * 60 * 60 * 24;
    std::ostringstream ss("MU expiry: ");
    struct tm *gmt = gmtime(&expiry);
    if (gmt == 0) {
        ss << "Cannot determine time";
    } else {
        char str[64];
        strftime(str, 64, "%x", gmt);

        ss << str << " at midnight (GMT) - ";
        if (days == 0) {
            ss << "today";
        } else if (days > 0) {
            if (days == 1)
                ss << "tomorrow";
            else
                ss << "in " << days << " days";
        } else {
            // days < 0
            if (days == -1)
                ss << "yesterday";
            else
                ss << -days << " days ago";
        }
    }

    SetDlgItemText(IDC_MU_INFO, ss.str().c_str());
    SetDlgItemText(IDC_MU_SESSION_BUTTON, "End session");
}

void CSiteMUPage::OnSessionButtonClicked()
{
    if (m_site.IsMUInfoAvailable() == false) {
        // Start MU session
        time_t now = time(0);
        CSetMUDlg dlg(now);
        if ((dlg.DoModal() == IDCANCEL) || (dlg.IsExpirySet() == false))
            return;

        m_site.SetMU(dlg.GetExpiry());
    } else {
        // End MU session
        m_site.ResetMU();
    }

    ShowInfo();
}
