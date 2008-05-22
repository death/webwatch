// WebWatch UI - by DEATH, 2004
//
// $Workfile: CheckSettings.cpp $ - Site check settings
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "CheckSettings.h"
#include "CheckNotification.h"

namespace WebWatch
{
    namespace
    {
        const int DefaultThreadCount = 8;
        const int MinThreadCount = 1;
        const int MaxThreadCount = 8;
    }

    CheckSettings::CheckSettings()
    : m_threadCount(DefaultThreadCount)
    {
    }

    CheckSettings::CheckSettings(const XML::CXML & xml)
    {
        SetThreadCount(xml.GetParamInt("ThreadCount", DefaultThreadCount));

        const std::string & tag = CheckNotificationChooser::Instance().GetXMLName();
        const XML::CXML *xmlNotification = xml.FindFirstChild(tag.c_str());
        while (xmlNotification) {
            CheckNotification *notification = CheckNotificationChooser::Instance().GetCheckNotification(*xmlNotification);
            if (notification)
                notification->SetParams(*xmlNotification);
            xmlNotification = xml.FindNextChild(tag.c_str());
        }
    }

    const char *CheckSettings::GetXMLName()
    {
        return "CheckSettings";
    }

    void CheckSettings::SetThreadCount(int threadCount)
    {
        if (threadCount < MinThreadCount)
            m_threadCount = MinThreadCount;
        else if (threadCount > MaxThreadCount)
            m_threadCount = MaxThreadCount;
        else
            m_threadCount = threadCount;
    }

    int CheckSettings::GetThreadCount() const
    {
        return m_threadCount;
    }

    int CheckSettings::GetDefaultThreadCount()
    {
        return DefaultThreadCount;
    }

    int CheckSettings::GetMinThreadCount()
    {
        return MinThreadCount;
    }

    int CheckSettings::GetMaxThreadCount()
    {
        return MaxThreadCount;
    }

    namespace 
    {
        class CreateNotificationXML : public std::unary_function<const CheckNotification *, void>
        {
        public:
            explicit CreateNotificationXML(XML::CXML & parent)
            : m_parent(parent)
            {
            }

            void operator() (const CheckNotification *notification)
            {
                std::auto_ptr<XML::CXML> xml(notification->GetParams());
                if (xml.get())
                    m_parent.AddChild(xml.release());
            }

        private:
            XML::CXML & m_parent;
        };
    }

    XML::CXML & operator<< (XML::CXML & xmlParent, const CheckSettings & settings)
    {
        int threadCount = settings.GetThreadCount();
        XML::CXML *xml = xmlParent.CreateChild(CheckSettings::GetXMLName());
        if (xml)
            xml->SetParam("ThreadCount", threadCount);

        const CheckNotificationChooserImpl::Notifications & notifications = CheckNotificationChooser::Instance().GetAllNotifications();
        std::for_each(notifications.begin(), notifications.end(), CreateNotificationXML(*xml));

        return xmlParent;
    }
}
