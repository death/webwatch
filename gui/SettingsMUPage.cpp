// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsMUPage.cpp $ - Settings MU property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 13/09/04 2:40 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SettingsMUPage.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSettingsMUPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSettingsMUPage, CPropertyPage)
END_MESSAGE_MAP()

CSettingsMUPage::CSettingsMUPage(const WebWatch::MUSettings & settings, CSettingsDlg & owner)
: CPropertyPage(CSettingsMUPage::IDD)
, m_owner(&owner)
, m_settings(settings)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "MU";
}

CSettingsMUPage::~CSettingsMUPage()
{
}

void CSettingsMUPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MU_DAYS_SPIN, m_muDaysSpin);
}

BOOL CSettingsMUPage::OnSetActive()
{
    m_muDaysSpin.SetRange(WebWatch::MUSettings::GetMinSpan(), WebWatch::MUSettings::GetMaxSpan());
    m_muDaysSpin.SetPos(m_settings.GetSpan());

    return CPropertyPage::OnSetActive();
}

void CSettingsMUPage::OnOK()
{
    m_settings.SetSpan(m_muDaysSpin.GetPos());
    CPropertyPage::OnOK();
}

void CSettingsMUPage::SetSettings(const WebWatch::MUSettings & settings)
{
    m_settings = settings;
}

WebWatch::MUSettings CSettingsMUPage::GetSettings() const
{
    return m_settings;
}
