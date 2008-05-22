// WebWatch UI - by DEATH, 2004
//
// $Workfile: CheckNotification.cpp $ - Check notifications
//
// $Author: Death $
// $Revision: 3 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/EmptyString.h"

#include "CheckNotification.h"
#include "Resource.h"

namespace 
{
    class SoundSettingsDlg : public CDialog
    {
        DECLARE_MESSAGE_MAP()

    public:
        enum { IDD = IDD_NOTIFICATION_SOUND_SETTINGS };

        explicit SoundSettingsDlg(CWnd & parent, const std::string & filename);
        std::string GetFileName() const;

    protected:
        virtual void DoDataExchange(CDataExchange *dx);

        afx_msg void OnBeep();
        afx_msg void OnBrowse();
        afx_msg void OnPlay();

    private:
        CString m_filename;
    };

    BEGIN_MESSAGE_MAP(SoundSettingsDlg, CDialog)
        ON_COMMAND(IDC_BEEP, OnBeep)
        ON_COMMAND(IDC_BROWSE, OnBrowse)
        ON_COMMAND(IDC_PLAY, OnPlay)
    END_MESSAGE_MAP()

    class SoundNotificationImpl : public WebWatch::CheckNotification
    {
    public:
        virtual ~SoundNotificationImpl();
        virtual void SetParams(const XML::CXML & xml);
        virtual std::auto_ptr<XML::CXML> GetParams() const;
        virtual void Activate(CWnd & wnd);
        virtual std::string GetName() const;
        virtual void Configure(CWnd & parent, XML::CXML & conf) const;

    private:
        std::string m_filename;
    };

    void SoundPlay(const std::string & filename);

    typedef Loki::SingletonHolder<SoundNotificationImpl> SoundNotification;

    class MessageNotificationImpl : public WebWatch::CheckNotification
    {
    public:
        virtual void Activate(CWnd & wnd);
        virtual std::string GetName() const;
        virtual void Configure(CWnd & parent, XML::CXML & conf) const;
    };

    typedef Loki::SingletonHolder<MessageNotificationImpl> MessageNotification;

    class ShowNotificationImpl : public WebWatch::CheckNotification
    {
    public:
        virtual void Activate(CWnd & wnd);
        virtual std::string GetName() const;
        virtual void Configure(CWnd & parent, XML::CXML & conf) const;
    };

    typedef Loki::SingletonHolder<ShowNotificationImpl> ShowNotification;

    class NameComparison
    {
        const std::string *m_name;
    public:
        explicit NameComparison(const std::string & name) : m_name(&name) {}
        bool operator() (const WebWatch::CheckNotification *notification) const
        {
            return notification->GetName() == *m_name;
        }
    };
}

namespace WebWatch
{

CheckNotificationChooserImpl::CheckNotificationChooserImpl()
{
    m_notifications.push_back(&SoundNotification::Instance());
    m_notifications.push_back(&MessageNotification::Instance());
    m_notifications.push_back(&ShowNotification::Instance());
}

CheckNotification *CheckNotificationChooserImpl::GetCheckNotification(const std::string & name)
{
    Notifications::iterator i = std::find_if(m_notifications.begin(),
        m_notifications.end(), NameComparison(name));
    return i == m_notifications.end() ? 0 : *i;
}

CheckNotification *CheckNotificationChooserImpl::GetCheckNotification(const XML::CXML & params)
{
    ASSERT(GetXMLName() == params.GetTagName());
    const char *type = params.GetParamText("Type");
    return GetCheckNotification(type);
}

CheckNotificationChooserImpl::Notifications CheckNotificationChooserImpl::GetAllNotifications() const
{
    return m_notifications;
}

std::string CheckNotificationChooserImpl::GetXMLName()
{
    return "Notification";
}

// CheckNotification default implementation

CheckNotification::CheckNotification()
: m_enabled(false)
{
}

void CheckNotification::SetParams(const XML::CXML & xml)
{
    ASSERT(CheckNotificationChooserImpl::GetXMLName() == xml.GetTagName());
    ASSERT(GetName() == xml.GetParamText("Type"));
    m_enabled = xml.GetParamBool("Enabled");
}

std::auto_ptr<XML::CXML> CheckNotification::GetParams() const
{
    const std::string & tag = CheckNotificationChooserImpl::GetXMLName();
    std::auto_ptr<XML::CXML> xml(new XML::CXML(tag.c_str()));
    const std::string & name = GetName();
    xml->SetParam("Type", name.c_str());
    xml->SetParam("Enabled", m_enabled);
    return xml;
}

void CheckNotification::Enable(bool enabled)
{
    m_enabled = enabled;
}

bool CheckNotification::IsEnabled() const
{
    return m_enabled;
}

}

namespace {

// Sound notification

SoundSettingsDlg::SoundSettingsDlg(CWnd & parent, const std::string & filename)
: CDialog(SoundSettingsDlg::IDD, &parent)
, m_filename(filename.c_str())
{
}

void SoundSettingsDlg::DoDataExchange(CDataExchange *dx)
{
    CDialog::DoDataExchange(dx);
    DDX_Text(dx, IDC_SOUND_FILE, m_filename); 
}

std::string SoundSettingsDlg::GetFileName() const
{
    return static_cast<const char *>(m_filename);
}

void SoundSettingsDlg::OnBeep()
{
    m_filename.Empty();
    UpdateData(FALSE);
}

void SoundSettingsDlg::OnBrowse()
{
    CFileDialog dlg(TRUE, "wav", 0, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST, "Wave files|*.wav|All files|*.*||", this);
    if (dlg.DoModal() == IDOK) {
        m_filename = dlg.GetPathName();
        UpdateData(FALSE);
    }
}

void SoundSettingsDlg::OnPlay()
{
    UpdateData();
    SoundPlay(static_cast<const char *>(m_filename));
}

SoundNotificationImpl::~SoundNotificationImpl()
{
}

void SoundNotificationImpl::SetParams(const XML::CXML & xml)
{
    CheckNotification::SetParams(xml);
    m_filename = xml.GetChildText("FileName");
}

std::auto_ptr<XML::CXML> SoundNotificationImpl::GetParams() const
{
    std::auto_ptr<XML::CXML> xml(CheckNotification::GetParams());
    if (xml.get()) {
        if (m_filename.empty() == false)
            xml->CreateChild("FileName", m_filename.c_str());
    }

    return xml;
}

void SoundNotificationImpl::Activate(CWnd &)
{
    SoundPlay(m_filename);
}

std::string SoundNotificationImpl::GetName() const
{
    return "Sound";
}

void SoundNotificationImpl::Configure(CWnd & parent, XML::CXML & conf) const
{
    XML::CXML *filename = conf.FindFirstChild("FileName");
    SoundSettingsDlg dlg(parent, filename ? filename->GetText() : Util::EmptyString());
    if (dlg.DoModal() == IDOK) {
        const std::string & newFileName = dlg.GetFileName();
        if (newFileName.empty()) {
            if (filename) {
                conf.RemoveChild(filename);
                delete filename;
            }
        } else {
            if (filename == 0)
                conf.CreateChild("FileName", newFileName.c_str());
            else
                filename->SetText(newFileName.c_str());
        }
    }
}

void SoundPlay(const std::string & filename)
{
    if (filename.empty() == false) {
        PlaySound(filename.c_str(), 0, SND_FILENAME | SND_ASYNC);
    } else {
        MessageBeep(-1);
    }
}

// Message notification

void MessageNotificationImpl::Activate(CWnd & wnd)
{
    wnd.MessageBox("Site check completed", "Notification", MB_OK | MB_ICONINFORMATION);
}

std::string MessageNotificationImpl::GetName() const
{
    return "Message";
}

void MessageNotificationImpl::Configure(CWnd & parent, XML::CXML & conf) const
{
    parent.MessageBox("Nothing to configure for this notification type", "Configuration", MB_ICONINFORMATION | MB_OK);
}

// Show notification

void ShowNotificationImpl::Activate(CWnd &)
{
    CWnd *mainWnd = AfxGetMainWnd();
    ASSERT(mainWnd);
    if (mainWnd->IsWindowVisible() == FALSE) {
        mainWnd->ShowWindow(SW_SHOW);
        if (mainWnd->IsIconic())
            mainWnd->ShowWindow(SW_RESTORE);
        mainWnd->SetForegroundWindow();
    } else {
        mainWnd->FlashWindowEx(FLASHW_ALL, 3, 0);
    }
}

std::string ShowNotificationImpl::GetName() const
{
    return "Show/Flash";
}

void ShowNotificationImpl::Configure(CWnd & parent, XML::CXML & conf) const
{
    parent.MessageBox("Nothing to configure for this notification type", "Configuration", MB_ICONINFORMATION | MB_OK);
}

}
