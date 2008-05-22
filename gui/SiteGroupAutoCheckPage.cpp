// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupAutoCheckPage.cpp $ - Site group Auto Check property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 27/09/04 3:48 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SiteGroupAutoCheckPage.h"
#include "SiteGroupPropertiesDlg.h"
#include "SiteItemGroup.h"
#include "Store.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSiteGroupAutoCheckPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSiteGroupAutoCheckPage, CPropertyPage)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_INCLUDED_IN_PARENT_AUTOCHECK, OnIncludedInParentAutoCheckClicked)
END_MESSAGE_MAP()

CSiteGroupAutoCheckPage::CSiteGroupAutoCheckPage(CSiteGroupPropertiesDlg & owner, WebWatch::SiteItemGroup & group, bool isRootGroup)
: CPropertyPage(CSiteGroupAutoCheckPage::IDD)
, m_owner(&owner)
, m_group(group)
, m_isRootGroup(isRootGroup)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "Auto-Check";
}

CSiteGroupAutoCheckPage::~CSiteGroupAutoCheckPage()
{
}

void CSiteGroupAutoCheckPage::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_AUTO_CHECK_SLIDER, m_slider);
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSiteGroupAutoCheckPage::OnSetActive()
{
    m_slider.SetRange(0, WebWatch::SiteItemGroup::GetAutoCheckMaxPeriods());
    m_slider.SetPos(m_group.GetAutoCheckPeriod());

    CWnd *wnd = GetDlgItem(IDC_INCLUDED_IN_PARENT_AUTOCHECK);
    ASSERT(wnd);
    if (m_isRootGroup)
        wnd->EnableWindow(FALSE);
    else
        wnd->SendMessage(BM_SETCHECK, m_group.IsIncludedInAutoCheck() ? BST_CHECKED : BST_UNCHECKED);

    UpdateText();

    return CPropertyPage::OnSetActive();
}

void CSiteGroupAutoCheckPage::OnOK()
{
    CPropertyPage::OnOK();
}

void CSiteGroupAutoCheckPage::OnHScroll(UINT sbCode, UINT pos, CScrollBar *scrollBar)
{
    if (scrollBar->m_hWnd == m_slider.m_hWnd) {
        UpdateText();
        UpdateProperties();
    }

    CPropertyPage::OnHScroll(sbCode, pos, scrollBar);
}

void CSiteGroupAutoCheckPage::OnIncludedInParentAutoCheckClicked()
{
    UpdateProperties();
}

void CSiteGroupAutoCheckPage::UpdateText()
{
    int pos = m_slider.GetPos();
    CString text;
    if (pos == 0) {
		text = "Auto Check (Disabled)";
    } else if (pos == 1) {
		text = "Auto Check (Each half an hour)";
    } else if (pos < 4) {
        text = "Auto Check (Each hour";
        if (pos & 1)
            text += " and a half";
        text += ')';
    } else {
		text.Format("Auto Check (Each %d hours", pos / 2);
		if (pos & 1)
			text += " and a half";
		text += ')';
    }

    SetDlgItemText(IDC_AUTO_CHECK_GROUP, text);
}

void CSiteGroupAutoCheckPage::UpdateProperties()
{
    int pos = m_slider.GetPos();
    m_group.SetAutoCheckPeriod(pos);
    m_group.SetIncludedInAutoCheck(IsDlgButtonChecked(IDC_INCLUDED_IN_PARENT_AUTOCHECK) == BST_CHECKED);
}
