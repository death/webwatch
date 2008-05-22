// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGeneralPage.cpp $ - Site General property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 16/04/04 14:14 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SiteGeneralPage.h"
#include "SitePropertiesDlg.h"
#include "SiteItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSiteGeneralPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSiteGeneralPage, CPropertyPage)
    ON_EN_CHANGE(IDC_NAME, OnChangedName)
    ON_EN_CHANGE(IDC_ADDRESS, OnChangedAddress)
    ON_EN_CHANGE(IDC_NOTES, OnChangedNotes)
END_MESSAGE_MAP()

CSiteGeneralPage::CSiteGeneralPage(CSitePropertiesDlg & owner, WebWatch::SiteItem & site)
: CPropertyPage(CSiteGeneralPage::IDD)
, m_owner(&owner)
, m_site(site)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "General";
}

CSiteGeneralPage::~CSiteGeneralPage()
{
}

void CSiteGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSiteGeneralPage::OnSetActive()
{
    SetDlgItemText(IDC_NAME, m_site.GetName().c_str());
    SetDlgItemText(IDC_ADDRESS, m_site.GetAddress().c_str());
    SetDlgItemText(IDC_NOTES, m_site.GetNotes().c_str());

    CheckValid();

    return CPropertyPage::OnSetActive();
}

void CSiteGeneralPage::CheckValid()
{
    bool valid = !m_site.GetName().empty() && !m_site.GetAddress().empty();
    m_owner->EnableOK(valid);
}

void CSiteGeneralPage::OnChangedName()
{
    CString name;
    GetDlgItemText(IDC_NAME, name);
    m_site.SetName(name);
    CheckValid();
}

void CSiteGeneralPage::OnChangedAddress()
{
    CString address;
    GetDlgItemText(IDC_ADDRESS, address);
    m_site.SetAddress(address);
    CheckValid();
}

void CSiteGeneralPage::OnChangedNotes()
{
    CString notes;
    GetDlgItemText(IDC_NOTES, notes);
    m_site.SetNotes(notes);
    CheckValid(); // needed by design?
}

void CSiteGeneralPage::OnOK()
{
    CPropertyPage::OnOK();
}
