// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsSiteListPage.h $ - Settings Site List property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 10/09/04 1:50 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"
#include "Colors.h"
#include "SiteList.h"

class CSettingsDlg;

class CSettingsSiteListPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsSiteListPage)

    enum ColorElement { text, background };

public:
    explicit CSettingsSiteListPage(CSiteList & siteList, CSettingsDlg & owner);
	virtual ~CSettingsSiteListPage();

	enum { IDD = IDD_SETTINGS_SITE_LIST_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnItemActivationActionChanged();
    afx_msg void OnItemColorNameChanged();
    afx_msg HBRUSH OnCtlColor(CDC *dc, CWnd *wnd, UINT type);
    afx_msg void OnSetItemColor();
    afx_msg void OnSetItemTextColor();
    afx_msg void OnSetItemBackgroundColor();
    afx_msg void OnSetItemDefaultColors();

	DECLARE_MESSAGE_MAP()

private:
    void SetNewColor(ColorElement el);
    std::string GetCurrentColorName() const;
    const WebWatch::ColorPair & GetCurrentColorPair() const;

private:
    CSettingsDlg *m_owner;
    CSiteList *m_siteList;
    CComboBox m_action;
    CComboBox m_color;
    CBrush m_brush;
    CMenu m_colorMenu;
    CSiteList::Colors m_colors;
};
