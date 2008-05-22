// WebWatch Core - by DEATH, 2004
//
// $Workfile: Site.cpp $ - WebWatch Site implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/TimeUtil.h"
#include "util/AddressParser.h"
#include "util/ThreadUtil.h"

#include "Site.h"
#include "CheckMethod.h"
#include "CheckMethodFactory.h"

namespace 
{
    const char *XmlName = "Site";
}

namespace Core_DE050401
{
    struct Site::Impl
    {
        explicit Impl(const XML::CXML & xml);

        explicit Impl(const char *address_)
        : address(address_)
        , checkTime(0)
        , checkMethod(0)
        {
            UpdateCheckParameters();
        }

        Impl(const Impl & other)
        : address(other.address)
        , checkParameters(other.checkParameters)
        , checkResults(other.checkResults)
        , checkTime(other.checkTime)
        , checkMethod(0)
        {
        }

        void UpdateCheckParameters();

        std::string address;
        XML::CXML checkParameters;
        XML::CXML checkResults;
        time_t checkTime;
        CheckMethod *checkMethod;
        Util::CriticalSection csCheckMethod;
    };

    Site::Site(const char *address)
    : m_pimpl(new Impl(address))
    {
    }

    Site::Site(const Site & other)
    : m_pimpl(new Impl(*other.m_pimpl))
    {
    }

    Site::Site(const XML::CXML & xml)
    : m_pimpl(new Impl(xml))
    {
    }

    Site::~Site()
    {
    }

    Site & Site::operator= (const Site & rhs)
    {
        if (this != &rhs) {
            Site temp(rhs);
            swap(m_pimpl, temp.m_pimpl);
        }
        return *this;
    }

    void Site::SetAddress(const char *address)
    {
        m_pimpl->address = address;
        m_pimpl->UpdateCheckParameters();
    }

    std::string Site::GetAddress() const
    {
        return m_pimpl->address;
    }

    std::string Site::GetType() const
    {
        Util::AddressParser parser(m_pimpl->address.c_str());
        return parser.GetType();
    }

    void Site::SetCheckParameters(const XML::CXML & parameters)
    {
        m_pimpl->checkParameters = parameters;
        m_pimpl->UpdateCheckParameters();
    }

    void Site::GetCheckParameters(XML::CXML & parameters) const
    {
        parameters = m_pimpl->checkParameters;
    }

    void Site::SetCheckResults(const XML::CXML & results)
    {
        m_pimpl->checkTime = time(0);
        m_pimpl->checkResults = results;
    }

    void Site::GetCheckResults(XML::CXML & results) const
    {
        results = m_pimpl->checkResults;
    }

    bool Site::AreCheckResultsAvailable() const
    {
        return m_pimpl->checkResults.IsEmpty() == false;
    }

    bool Site::IsUpdated(const XML::CXML & results) const
    {
        assert(AreCheckResultsAvailable() == true);
        return m_pimpl->checkResults != results;
    }

    void Site::GetCheckTime(XML::CXML & checkTime) const
    {
        if (m_pimpl->checkTime == 0)
            return;

        struct tm *local = localtime(&m_pimpl->checkTime);
        if (local == 0)
            return;

        XML::CXML *xmlTime = Util::CreateXMLTimeNode(checkTime, "CheckTime", *local);
        checkTime = *xmlTime;
    }

    void Site::Check(XML::CXML & results, StatusNotifyClient *statusNotifyClient) const
    {
        if (m_pimpl->checkParameters.IsEmpty())
            throw SiteError("No check parameters");

        const char *method = m_pimpl->checkParameters.GetParamText("Method");
        if (!*method)
            throw SiteError("No method supplied in check parameters");

        std::auto_ptr<CheckMethod> checkMethod;
        try {
            checkMethod.reset(CheckMethodFactory::Instance().CreateObject(method));
        }
        catch(...) {
            throw SiteError("Could not create check method object");
        }

        if (checkMethod.get() == 0)
            throw SiteError("Could not create check method object");

        {
        Util::ScopedLock lock(m_pimpl->csCheckMethod);
        m_pimpl->checkMethod = checkMethod.get();
        }

        try {
            checkMethod->Check(m_pimpl->address.c_str(), m_pimpl->checkParameters, results, statusNotifyClient);
        }
        catch (...) {
            {
            Util::ScopedLock lock(m_pimpl->csCheckMethod);
            m_pimpl->checkMethod = 0;
            }
            throw;
        }

        {
        Util::ScopedLock lock(m_pimpl->csCheckMethod);
        m_pimpl->checkMethod = 0;
        }
    }

    void Site::Abort() const
    {
        Util::ScopedLock lock(m_pimpl->csCheckMethod);
        if (m_pimpl->checkMethod == 0)
            return;

        m_pimpl->checkMethod->Abort();
    }

    XML::CXML *Site::Write(XML::CXML & xmlParent) const
    {
        XML::CXML *xml = xmlParent.CreateChild(XmlName);

        if (xml) {
            xml->CreateChild("Address", m_pimpl->address.c_str());
            if (m_pimpl->checkParameters.IsEmpty() == false)
                xml->CreateChild(m_pimpl->checkParameters);
            if (m_pimpl->checkResults.IsEmpty() == false)
                xml->CreateChild(m_pimpl->checkResults);
            if (m_pimpl->checkTime != 0) {
                struct tm *local = localtime(&m_pimpl->checkTime);
                if (local)
                    Util::CreateXMLTimeNode(*xml, "CheckTime", *local);
            }
        } else {
            assert(!"Failed to create Site XML");
        }

        return xml;
    }

    const char *Site::GetXMLName()
    {
        return XmlName;
    }

    Site::Impl::Impl(const XML::CXML & xml)
    {
        if (std::strcmp(xml.GetTagName(), XmlName) != 0) {
            std::string msg = "XML passed to Site constructor is not a Site XML, but a ";
            msg += xml.GetTagName();
            msg += " XML";
            throw SiteError(msg);
        }

        address = xml.GetChildText("Address");
        
        const XML::CXML *child = xml.FindFirstChild("CheckParameters");
        if (child)
            checkParameters = *child;

        child = xml.FindFirstChild("CheckResults");
        if (child)
            checkResults = *child;

        child = xml.FindFirstChild("CheckTime");
        if (child) {
            struct tm tm;
            Util::ParseXMLTimeNode(xml, "CheckTime", tm);
            checkTime = mktime(&tm);
        } else {
            checkTime = 0;
        }

        checkMethod = 0;
        UpdateCheckParameters();
    }

    // Socket mode is always dependent on the address, a default check method
    // is chosen if none was previously set.
    void Site::Impl::UpdateCheckParameters()
    {
        Util::AddressParser parser(address.c_str());
        std::string type = parser.GetType();
        const char *defMethod = 0;
        const char *socketType = 0;

        if (type == "http") {
            defMethod = "HttpTime";
            socketType = "Plain";
        } else if (type == "https") {
            defMethod = "HttpTime";
            socketType = "Secure";
        } else if (type == "ftp") {
            defMethod = "Ftp";
            socketType = "Plain";
        }

        if (checkParameters.IsEmpty()) {
            checkParameters.SetTagName("CheckParameters");
            if (defMethod)
                checkParameters.SetParam("Method", defMethod);
        }

        if (socketType) {
            XML::CXML *socketParams = checkParameters.FindFirstChild("Socket", true);
            assert(socketParams);
            socketParams->SetParam("Mode", socketType);
        }
    }

    // Namespace-level functions
    bool operator== (const Site & site1, const Site & site2)
    {
        if (&site1 == &site2)
            return true;

        if (site1.m_pimpl->address != site2.m_pimpl->address)
            return false;

        return true;
    }

    bool operator!= (const Site & site1, const Site & site2)
    {
        return !operator== (site1, site2);
    }
}
