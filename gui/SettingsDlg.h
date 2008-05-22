// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsDlg.h $ - Settings dialog (property sheet)
//
// $Author: Death $
// $Revision: 5 $
// $Date: 13/09/04 17:38 $
// $NoKeywords: $
//
#pragma once

class CSettingsGeneralPage;
class CSettingsSiteListPage;
class CSettingsSiteGroupsTreePage;
class CSettingsMUPage;
class CSettingsCheckPage;
class CSiteList;
class CSiteGroupsTree;

class CSettingsDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
    CSettingsDlg(CSiteList & siteList, CSiteGroupsTree & groupsTree, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CSettingsDlg();

    void EnableOK(bool enable);
    void StoreSettings();

protected:
    DECLARE_MESSAGE_MAP()

private:
    void Initialize();
    void CreatePages();

private:
    std::auto_ptr<CSettingsGeneralPage> m_generalPage;
    std::auto_ptr<CSettingsMUPage> m_muPage;
    std::auto_ptr<CSettingsSiteListPage> m_siteListPage;
    std::auto_ptr<CSettingsSiteGroupsTreePage> m_groupsTreePage;
    std::auto_ptr<CSettingsCheckPage> m_checkPage;
    CSiteList *m_siteList;
    CSiteGroupsTree *m_groupsTree;
};

