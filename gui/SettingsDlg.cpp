// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsDlg.cpp $ - Settings dialog (property sheet)
//
// $Author: Death $
// $Revision: 5 $
// $Date: 13/09/04 17:38 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SettingsDlg.h"
#include "Store.h"
#include "SettingsGeneralPage.h"
#include "SettingsSiteListPage.h"
#include "SettingsSiteGroupsTreePage.h"
#include "SettingsMUPage.h"
#include "SettingsCheckPage.h"
#include "SiteList.h"
#include "SiteGroupsTree.h"

#pragma warning(disable:4355) // 'this' : used in base member initializer list

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSettingsDlg, CPropertySheet)

BEGIN_MESSAGE_MAP(CSettingsDlg, CPropertySheet)
END_MESSAGE_MAP()

CSettingsDlg::CSettingsDlg(CSiteList & siteList, CSiteGroupsTree & groupsTree, LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
: CPropertySheet(pszCaption, pParentWnd, iSelectPage)
, m_siteList(&siteList)
, m_groupsTree(&groupsTree)
{
    Initialize();
}

CSettingsDlg::~CSettingsDlg()
{
}

void CSettingsDlg::Initialize()
{
    CreatePages();

    AddPage(m_generalPage.get());
    AddPage(m_muPage.get());
    AddPage(m_checkPage.get());
    AddPage(m_siteListPage.get());
    AddPage(m_groupsTreePage.get());

    // No Apply Now button, no Help button
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
    m_psh.dwFlags &= ~PSH_HASHELP;
}

void CSettingsDlg::EnableOK(bool enable)
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

void CSettingsDlg::StoreSettings()
{
    ASSERT(m_generalPage.get());
    const WebWatch::GeneralSettings & generalSettings = m_generalPage->GetSettings();
    WebWatch::Store::Instance().SetGeneralSettings(generalSettings);

    ASSERT(m_muPage.get());
    const WebWatch::MUSettings & muSettings = m_muPage->GetSettings();
    WebWatch::Store::Instance().SetMUSettings(muSettings);

    ASSERT(m_checkPage.get());
    const WebWatch::CheckSettings & checkSettings = m_checkPage->GetSettings();
    WebWatch::Store::Instance().SetCheckSettings(checkSettings);
}

void CSettingsDlg::CreatePages()
{
    WebWatch::GeneralSettings generalSettings;
    WebWatch::Store::Instance().GetGeneralSettings(generalSettings);
    m_generalPage.reset(new CSettingsGeneralPage(generalSettings, *this));

    WebWatch::MUSettings muSettings;
    WebWatch::Store::Instance().GetMUSettings(muSettings);
    m_muPage.reset(new CSettingsMUPage(muSettings, *this));

    WebWatch::CheckSettings checkSettings;
    WebWatch::Store::Instance().GetCheckSettings(checkSettings);
    m_checkPage.reset(new CSettingsCheckPage(checkSettings, *this));

    m_siteListPage.reset(new CSettingsSiteListPage(*m_siteList, *this));

    m_groupsTreePage.reset(new CSettingsSiteGroupsTreePage(*m_groupsTree, *this));
}
