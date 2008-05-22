// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsMUPage.h $ - Settings MU property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 13/09/04 2:40 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"
#include "MU.h"

class CSettingsDlg;

class CSettingsMUPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsMUPage)

public:
    explicit CSettingsMUPage(const WebWatch::MUSettings & settings, CSettingsDlg & owner);
	virtual ~CSettingsMUPage();

	enum { IDD = IDD_SETTINGS_MU_PAGE };

    void SetSettings(const WebWatch::MUSettings & settings);
    WebWatch::MUSettings GetSettings() const;

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
    CSettingsDlg *m_owner;
    WebWatch::MUSettings m_settings;
    CSpinButtonCtrl m_muDaysSpin;
};
