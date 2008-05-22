// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsCheckPage.cpp $ - Settings Check property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "SettingsCheckPage.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

IMPLEMENT_DYNAMIC(CSettingsCheckPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSettingsCheckPage, CPropertyPage)
    ON_CBN_SELCHANGE(IDC_NOTIFICATION_TYPE, OnChangeNotificationType)
    ON_COMMAND(IDC_ENABLE_NOTIFICATION, OnEnableNotification)
    ON_COMMAND(IDC_CONFIGURE_NOTIFICATION, OnConfigureNotification)
END_MESSAGE_MAP()

CSettingsCheckPage::CSettingsCheckPage(const WebWatch::CheckSettings & settings, CSettingsDlg & owner)
: CPropertyPage(CSettingsCheckPage::IDD)
, m_owner(&owner)
, m_settings(settings)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "Check";
}

CSettingsCheckPage::~CSettingsCheckPage()
{
    CheckNotificationSettings::iterator i;
    for (i = m_notificationSettings.begin(); i != m_notificationSettings.end(); ++i) {
        CheckNotificationParams & params = i->second;
        delete params.params;
    }
}

void CSettingsCheckPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHECK_THREADS_SPIN, m_checkThreadsSpin);
    DDX_Control(pDX, IDC_NOTIFICATION_TYPE, m_notificationType);
    DDX_Control(pDX, IDC_ENABLE_NOTIFICATION, m_enableNotificationButton);
    DDX_Control(pDX, IDC_CONFIGURE_NOTIFICATION, m_configureNotificationButton);
}

BOOL CSettingsCheckPage::OnSetActive()
{
    m_checkThreadsSpin.SetRange(WebWatch::CheckSettings::GetMinThreadCount(), WebWatch::CheckSettings::GetMaxThreadCount());
    m_checkThreadsSpin.SetPos(m_settings.GetThreadCount());

    return CPropertyPage::OnSetActive();
}

void CSettingsCheckPage::OnOK()
{
    m_settings.SetThreadCount(m_checkThreadsSpin.GetPos());

    CheckNotificationSettings::iterator i;
    for (i = m_notificationSettings.begin(); i != m_notificationSettings.end(); ++i) {
        const std::string & name = i->first;
        CheckNotificationParams & params = i->second;
        WebWatch::CheckNotification *notification = WebWatch::CheckNotificationChooser::Instance().GetCheckNotification(name);
        if (notification) {
            notification->SetParams(*params.params);
            notification->Enable(params.enabled);
        }
    }

    CPropertyPage::OnOK();
}

class CSettingsCheckPage::AddNotification : public std::unary_function<const WebWatch::CheckNotification *, void>
{
public:
    AddNotification(CSettingsCheckPage & owner)
    : m_owner(owner)
    {
    }

    void operator() (const WebWatch::CheckNotification *notification)
    {
        if (notification) {
            const std::string & name = notification->GetName();
            m_owner.m_notificationType.AddString(name.c_str());
            CheckNotificationParams params;
            params.enabled = notification->IsEnabled();
            params.params = notification->GetParams().release();
            m_owner.m_notificationSettings.insert(std::make_pair(name, params));
        }
    }

private:
    CSettingsCheckPage & m_owner;
};

BOOL CSettingsCheckPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_notifications = WebWatch::CheckNotificationChooser::Instance().GetAllNotifications();
    std::for_each(m_notifications.begin(), m_notifications.end(), AddNotification(*this));
    m_notificationType.SetCurSel(0);
    OnChangeNotificationType();

    return FALSE;
}

void CSettingsCheckPage::SetSettings(const WebWatch::CheckSettings & settings)
{
    m_settings = settings;
}

WebWatch::CheckSettings CSettingsCheckPage::GetSettings() const
{
    return m_settings;
}

void CSettingsCheckPage::OnChangeNotificationType()
{
    CheckNotificationSettings::iterator i = GetCurrentNotification();
    const CheckNotificationParams & params = i->second;
    if (params.enabled) {
        m_enableNotificationButton.SetWindowText("Disable");
        m_configureNotificationButton.EnableWindow();
    } else {
        m_enableNotificationButton.SetWindowText("Enable");
        m_configureNotificationButton.EnableWindow(FALSE);
    }
}

void CSettingsCheckPage::OnEnableNotification()
{
    CheckNotificationSettings::iterator i = GetCurrentNotification();
    CheckNotificationParams & params = i->second;
    params.enabled = !params.enabled;
    OnChangeNotificationType();
}

void CSettingsCheckPage::OnConfigureNotification()
{
    CheckNotificationSettings::iterator i = GetCurrentNotification();
    const std::string & name = i->first;
    CheckNotificationParams & params = i->second;
    WebWatch::CheckNotification *notification = WebWatch::CheckNotificationChooser::Instance().GetCheckNotification(name);
    if (notification)
        notification->Configure(*this, *params.params);
}

CSettingsCheckPage::CheckNotificationSettings::iterator CSettingsCheckPage::GetCurrentNotification()
{
    int sel = m_notificationType.GetCurSel();
    CString name;
    m_notificationType.GetLBText(sel, name);
    CheckNotificationSettings::iterator i = m_notificationSettings.find(static_cast<const char *>(name));
    ASSERT(i != m_notificationSettings.end());
    return i;
}
