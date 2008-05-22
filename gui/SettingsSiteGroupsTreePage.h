// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsSiteGroupsTreePage.h $ - Settings Site Groups Tree property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 11/09/04 20:36 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"
#include "Colors.h"
#include "SiteGroupsTree.h"

class CSettingsDlg;

class CSettingsSiteGroupsTreePage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsSiteGroupsTreePage)

public:
    explicit CSettingsSiteGroupsTreePage(CSiteGroupsTree & groupsTree, CSettingsDlg & owner);
	virtual ~CSettingsSiteGroupsTreePage();

	enum { IDD = IDD_SETTINGS_SITE_GROUPS_TREE_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnItemColorNameChanged();
    afx_msg HBRUSH OnCtlColor(CDC *dc, CWnd *wnd, UINT type);
    afx_msg void OnSetItemColor();
    afx_msg void OnSetItemTextColor();
    afx_msg void OnSetItemDefaultColors();

	DECLARE_MESSAGE_MAP()

private:
    WebWatch::Color & GetCurrentColor(CSiteGroupsTree::Colors *colors = 0);
    std::string GetCurrentColorName() const;

private:
    CSettingsDlg *m_owner;
    CSiteGroupsTree *m_groupsTree;
    CComboBox m_color;
    CMenu m_colorMenu;
    CSiteGroupsTree::Colors m_colors;
};
