// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupGeneralPage.cpp $ - Site group General property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 25/05/04 23:11 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SiteGroupGeneralPage.h"
#include "SiteGroupPropertiesDlg.h"
#include "SiteItemGroup.h"
#include "Store.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSiteGroupGeneralPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSiteGroupGeneralPage, CPropertyPage)
    ON_EN_CHANGE(IDC_NAME, OnChangedName)
END_MESSAGE_MAP()

CSiteGroupGeneralPage::CSiteGroupGeneralPage(CSiteGroupPropertiesDlg & owner, WebWatch::SiteItemGroup & group, bool isRootGroup)
: CPropertyPage(CSiteGroupGeneralPage::IDD)
, m_owner(&owner)
, m_group(group)
, m_isRootGroup(isRootGroup)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "General";
}

CSiteGroupGeneralPage::~CSiteGroupGeneralPage()
{
}

void CSiteGroupGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSiteGroupGeneralPage::OnSetActive()
{
    CWnd *nameCtrl = GetDlgItem(IDC_NAME);
    nameCtrl->SetWindowText(m_group.GetName().c_str());
//    nameCtrl->EnableWindow(m_isRootGroup ? FALSE : TRUE);

    CheckValid();

    return CPropertyPage::OnSetActive();
}

void CSiteGroupGeneralPage::CheckValid()
{
    bool valid = !m_group.GetName().empty();
    m_owner->EnableOK(valid);
}

void CSiteGroupGeneralPage::OnChangedName()
{
    CString name;
    GetDlgItemText(IDC_NAME, name);
    m_group.SetName(name);
    CheckValid();
}

void CSiteGroupGeneralPage::OnOK()
{
    CPropertyPage::OnOK();
}
