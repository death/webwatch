// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupPropertiesDlg.cpp $ - Site group properties dialog (property sheet)
//
// $Author: Death $
// $Revision: 2 $
// $Date: 11/10/04 23:01 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SiteGroupPropertiesDlg.h"
#include "SiteItemGroup.h"

#pragma warning(disable:4355) // 'this' : used in base member initializer list

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSiteGroupPropertiesDlg, CPropertySheet)

BEGIN_MESSAGE_MAP(CSiteGroupPropertiesDlg, CPropertySheet)
END_MESSAGE_MAP()

CSiteGroupPropertiesDlg::CSiteGroupPropertiesDlg(WebWatch::SiteItemGroup & group, bool isRootGroup, UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
, m_general(*this, group, isRootGroup)
, m_autoCheck(*this, group, isRootGroup)
, m_proxy(*this, group, isRootGroup)
{
    Initialize();
}

CSiteGroupPropertiesDlg::CSiteGroupPropertiesDlg(WebWatch::SiteItemGroup & group, bool isRootGroup, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, m_general(*this, group, isRootGroup)
, m_autoCheck(*this, group, isRootGroup)
, m_proxy(*this, group, isRootGroup)
{
    Initialize();
}

CSiteGroupPropertiesDlg::~CSiteGroupPropertiesDlg()
{
}

void CSiteGroupPropertiesDlg::Initialize()
{
    AddPage(&m_general);
    AddPage(&m_autoCheck);
    AddPage(&m_proxy);

    // No Apply Now button, no Help button
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_psh.dwFlags &= ~PSH_HASHELP;
}

void CSiteGroupPropertiesDlg::EnableOK(bool enable)
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
