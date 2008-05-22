// WebWatch UI - by DEATH, 2004
//
// $Workfile: CheckNotification.h $ - Check notifications
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKNOTIFICATION_H
#define WEBWATCH_INC_CHECKNOTIFICATION_H

#include "loki/Singleton.h"

namespace WebWatch
{

class CheckNotification
{
public:
    CheckNotification();

    virtual ~CheckNotification() {}
    virtual void SetParams(const XML::CXML & xml);
    virtual std::auto_ptr<XML::CXML> GetParams() const;
    virtual std::string GetName() const = 0;
    virtual void Activate(CWnd & wnd) = 0;
    virtual void Configure(CWnd & parent, XML::CXML & conf) const = 0;

    void Enable(bool enabled);
    bool IsEnabled() const;

private:
    bool m_enabled;
};

class CheckNotificationChooserImpl
{
public:
    typedef std::vector<CheckNotification *> Notifications;

public:
    CheckNotificationChooserImpl();

    CheckNotification *GetCheckNotification(const std::string & name);
    CheckNotification *GetCheckNotification(const XML::CXML & params);
    Notifications GetAllNotifications() const;

    static std::string GetXMLName();

private:
    Notifications m_notifications;
};

typedef Loki::SingletonHolder<CheckNotificationChooserImpl> CheckNotificationChooser;

} 

#endif // WEBWATCH_INC_CHECKNOTIFICATION_H
