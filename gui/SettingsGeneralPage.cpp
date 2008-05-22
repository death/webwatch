// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsGeneralPage.cpp $ - Settings General property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 26/08/04 11:25 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SettingsGeneralPage.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSettingsGeneralPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSettingsGeneralPage, CPropertyPage)
    ON_BN_CLICKED(IDC_ENABLE_AUTO_SAVE, OnChangedAutoSave)
END_MESSAGE_MAP()

CSettingsGeneralPage::CSettingsGeneralPage(const WebWatch::GeneralSettings & settings, CSettingsDlg & owner)
: CPropertyPage(CSettingsGeneralPage::IDD)
, m_owner(&owner)
, m_settings(settings)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "General";
}

CSettingsGeneralPage::~CSettingsGeneralPage()
{
}

void CSettingsGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CSettingsGeneralPage::OnSetActive()
{
    CheckDlgButton(IDC_ENABLE_AUTO_SAVE, m_settings.autoSave ? BST_CHECKED : BST_UNCHECKED);
    return CPropertyPage::OnSetActive();
}

void CSettingsGeneralPage::OnOK()
{
    CPropertyPage::OnOK();
}

void CSettingsGeneralPage::SetSettings(const WebWatch::GeneralSettings & settings)
{
    m_settings = settings;
}

WebWatch::GeneralSettings CSettingsGeneralPage::GetSettings() const
{
    return m_settings;
}

void CSettingsGeneralPage::OnChangedAutoSave()
{
    m_settings.autoSave = IsDlgButtonChecked(IDC_ENABLE_AUTO_SAVE) == BST_CHECKED;
}
