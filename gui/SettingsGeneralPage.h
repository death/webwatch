// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsGeneralPage.h $ - Settings General property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 26/08/04 11:25 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"
#include "GeneralSettings.h"

class CSettingsDlg;

class CSettingsGeneralPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsGeneralPage)

public:
    explicit CSettingsGeneralPage(const WebWatch::GeneralSettings & settings, CSettingsDlg & owner);
	virtual ~CSettingsGeneralPage();

	enum { IDD = IDD_SETTINGS_GENERAL_PAGE };

    void SetSettings(const WebWatch::GeneralSettings & settings);
    WebWatch::GeneralSettings GetSettings() const;

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnChangedAutoSave();

	DECLARE_MESSAGE_MAP()

private:
    CSettingsDlg *m_owner;
    WebWatch::GeneralSettings m_settings;
};
