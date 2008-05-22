// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsCheckPage.h $ - Settings Check property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"
#include "CheckSettings.h"
#include "CheckNotification.h"

class CSettingsDlg;

class CSettingsCheckPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSettingsCheckPage)

    struct CheckNotificationParams
    {
        bool enabled;
        XML::CXML *params;
    };

    typedef std::map<std::string, CheckNotificationParams> CheckNotificationSettings;

    class AddNotification;

public:
    explicit CSettingsCheckPage(const WebWatch::CheckSettings & settings, CSettingsDlg & owner);
	virtual ~CSettingsCheckPage();

	enum { IDD = IDD_SETTINGS_CHECK_PAGE };

    void SetSettings(const WebWatch::CheckSettings & settings);
    WebWatch::CheckSettings GetSettings() const;

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();
    virtual BOOL OnInitDialog();

    afx_msg void OnChangeNotificationType();
    afx_msg void OnEnableNotification();
    afx_msg void OnConfigureNotification();

	DECLARE_MESSAGE_MAP()

private:
    CheckNotificationSettings::iterator GetCurrentNotification();

private:
    CSettingsDlg *m_owner;
    WebWatch::CheckSettings m_settings;
    CSpinButtonCtrl m_checkThreadsSpin;
    WebWatch::CheckNotificationChooserImpl::Notifications m_notifications;
    CComboBox m_notificationType;
    CheckNotificationSettings m_notificationSettings;
    CButton m_enableNotificationButton;
    CButton m_configureNotificationButton;
};
