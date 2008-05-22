// WebWatch UI - by DEATH, 2004
//
// $Workfile: SitePropertiesDlg.cpp $ - Site properties dialog (property sheet)
//
// $Author: Death $
// $Revision: 3 $
// $Date: 25/05/04 19:38 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SitePropertiesDlg.h"
#include "SiteItem.h"

#pragma warning(disable:4355) // 'this' : used in base member initializer list

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSitePropertiesDlg, CPropertySheet)

BEGIN_MESSAGE_MAP(CSitePropertiesDlg, CPropertySheet)
END_MESSAGE_MAP()

CSitePropertiesDlg::CSitePropertiesDlg(WebWatch::SiteItem & site, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, m_general(*this, site)
, m_check(*this, site)
, m_mu(*this, site)
{
    Initialize();
}

CSitePropertiesDlg::CSitePropertiesDlg(WebWatch::SiteItem & site, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, m_general(*this, site)
, m_check(*this, site)
, m_mu(*this, site)
{
    Initialize();
}

CSitePropertiesDlg::~CSitePropertiesDlg()
{
}

void CSitePropertiesDlg::Initialize()
{
    AddPage(&m_general);
    AddPage(&m_check);
    AddPage(&m_mu);

    // No Apply Now button, no Help button
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_psh.dwFlags &= ~PSH_HASHELP;
}

void CSitePropertiesDlg::EnableOK(bool enable)
{
    CWnd *ok = GetDlgItem(IDOK);
    ASSERT(ok);

    if (ok->IsWindowEnabled()) {
        if (enable == false)
            ok->EnableWindow(FALSE);
    } else {
        if (enable)
            ok->EnableWindow(TRUE);
    }
}
