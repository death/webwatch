// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteItemGroup.cpp $ - SiteItem group
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Common.h"

#include "SiteItemGroup.h"
#include "SiteItem.h"
#include "SiteOperationManager.h"
#include "Store.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace {
    struct OverallSiteCountAccumulator : public std::binary_function<int, const WebWatch::SiteItemGroup *, int>
    {
        int operator() (int result, const WebWatch::SiteItemGroup *group)
        {
            ASSERT(group);
            return result + GetOverallSiteCount(*group);
        }
    };

    struct IncludeDefaultCheckedSites : public std::unary_function<const WebWatch::SiteItem *, bool>
    {
        bool operator() (const WebWatch::SiteItem *site) const
        {
            return site->IsCheckedByDefault();
        }
    };

    struct IncludeAllGroups : public std::unary_function<const WebWatch::SiteItemGroup *, bool>
    {
        bool operator() (const WebWatch::SiteItemGroup *group) const
        {
            return true;
        }
    };

    struct IncludeAllSites : public std::unary_function<const WebWatch::SiteItem *, bool>
    {
        bool operator() (const WebWatch::SiteItem *site) const
        {
            return true;
        }
    };
}

namespace WebWatch
{

// SiteMover class

class SiteItemGroup::SiteMover : public std::unary_function<SiteItem *, void>
{
public:
    explicit SiteMover(SiteItemGroup & dest)
    : m_dest(dest)
    {
    }

    void operator() (SiteItem *site)
    {
        ASSERT(site);

        const SiteItemGroup *parent = site->GetParentGroup();
        ASSERT(parent);

        if (parent != &m_dest)
            m_dest.AddSite(SiteItemGroup::DetachSite(*site));
    }

private:
    SiteItemGroup & m_dest;
};

// GroupMover class

class SiteItemGroup::GroupMover : public std::unary_function<SiteItemGroup *, void>
{
public:
    explicit GroupMover(SiteItemGroup & dest)
    : m_dest(dest)
    {
    }

    void operator() (SiteItemGroup *group)
    {
        ASSERT(group);

        const SiteItemGroup *parent = group->GetParentGroup();
        ASSERT(parent);
    
        if (parent != &m_dest)
            m_dest.AddGroup(SiteItemGroup::DetachGroup(*group));
    }

private:
    SiteItemGroup & m_dest;
};

// SiteItemGroup class

SiteItemGroup::SiteItemGroup(const char *name)
: m_parent(0)
{
    m_prop.name = name;
    // Auto-checking is disabled by default, but also included in parent group's 
    // auto-check
    m_prop.autoCheckData.included = true;
    m_prop.autoCheckData.period = 0;
    SetNextCheck();
}

SiteItemGroup::SiteItemGroup(const XML::CXML & xml)
: m_parent(0)
{
    // Make sure the XML passed is a SiteItemGroup XML
    if (strcmp(xml.GetTagName(), GetXMLName()) != 0) {
        std::string msg = "XML passed to SiteItemGroup constructor is not a SiteItemGroup XML, but a ";
        msg += xml.GetTagName();
        msg += " XML";
        throw SiteItemGroupError(msg);
    }

    m_prop.name = xml.GetChildText("Name");

    // Load auto-check data (auto-checking is disabled by default)
    const XML::CXML *xmlAutoCheck = xml.FindFirstChild("AutoCheck");
    if (xmlAutoCheck) {
        SetAutoCheckPeriod(xmlAutoCheck->GetParamInt("Period"));
        SetIncludedInAutoCheck(xmlAutoCheck->GetParamBool("Included", true));
    } else {
        // Auto-checking is disabled by default, but also included in parent group's 
        // auto-check
        SetAutoCheckPeriod(0);
        SetIncludedInAutoCheck(true);
    }

    SetNextCheck();

    // Load proxy settigns
    const XML::CXML *xmlProxy = xml.FindFirstChild("Proxy");
    if (xmlProxy) {
        std::string usage(xmlProxy->GetParamText("Usage"));
        if (usage == "No") {
            m_prop.proxyData.usage = ProxyData::noProxy;
        } else if (usage == "Inherit") {
            m_prop.proxyData.usage = ProxyData::inheritProxy;
        } else if (usage == "Use") {
            m_prop.proxyData.usage = ProxyData::useProxy;
        }

        m_prop.proxyData.type = xmlProxy->GetParamText("Type");
        m_prop.proxyData.address = xmlProxy->GetChildText("Address");
    }

    // Load sub-groups
    const XML::CXML *xmlGroup = xml.FindFirstChild(GetXMLName());
    while (xmlGroup) {
        SiteItemGroup *group = new SiteItemGroup(*xmlGroup);
        AddGroup(*group);
        xmlGroup = xml.FindNextChild(GetXMLName());
    }

    // Load sites
    const XML::CXML *xmlSite = xml.FindFirstChild(SiteItem::GetXMLName());
    while (xmlSite) {
        SiteItem *site = new SiteItem(*xmlSite);
        AddSite(*site);
        xmlSite = xml.FindNextChild(SiteItem::GetXMLName());
    }
}

SiteItemGroup::SiteItemGroup(const SiteItemGroup::Properties & prop)
: m_prop(prop)
, m_parent(0)
{
    SetNextCheck();
}

SiteItemGroup::~SiteItemGroup()
{
    ASSERT(SanityCheck());
    DeleteAllSites();
    DeleteAllGroups();
}

void SiteItemGroup::SetName(const char *name)
{
    m_prop.name = name;
}

std::string SiteItemGroup::GetName() const
{
    return m_prop.name;
}

SiteItemGroup *SiteItemGroup::GetParentGroup()
{
    return m_parent;
}

const SiteItemGroup *SiteItemGroup::GetParentGroup() const
{
    return m_parent;
}

SiteItemGroup::Properties SiteItemGroup::GetProperties() const
{
    return m_prop;
}

void SiteItemGroup::SetProperties(const SiteItemGroup::Properties & prop)
{
    m_prop = prop;
    SetNextCheck();
}

// Group management

int SiteItemGroup::GetGroupCount() const
{
    return static_cast<int>(m_groups.size());
}

void SiteItemGroup::DeleteAllGroups()
{
    std::for_each(m_groups.begin(), m_groups.end(), Util::SafeDelete());
    m_groups.clear();
}

void SiteItemGroup::DeleteGroup(SiteItemGroup & group)
{
    ASSERT(group.m_parent == this);
    m_groups.erase(std::remove(m_groups.begin(), m_groups.end(), &group));
    delete &group;
}

SiteItemGroup::GroupIterator SiteItemGroup::GetGroupsBegin()
{
    return m_groups.begin();
}

SiteItemGroup::ConstGroupIterator SiteItemGroup::GetGroupsBegin() const
{
    return m_groups.begin();
}

SiteItemGroup::GroupIterator SiteItemGroup::GetGroupsEnd()
{
    return m_groups.end();
}

SiteItemGroup::ConstGroupIterator SiteItemGroup::GetGroupsEnd() const
{
    return m_groups.end();
}

void SiteItemGroup::AddGroup(SiteItemGroup & group)
{
    m_groups.push_back(&group);
    group.m_parent = this;
}

// Site management - Deals with direct sites

int SiteItemGroup::GetSiteCount() const
{
    return static_cast<int>(m_sites.size());
}

// This function adds a site to the group. It is important to notice that the
// site must exist on the free store, and that by adding it to the group, the
// ownership for the site is also transferred.
void SiteItemGroup::AddSite(SiteItem & site)
{
    ASSERT(site.GetParentGroup() == 0);
    m_sites.push_back(&site);
    site.SetParentGroup(this);
}

void SiteItemGroup::DeleteAllSites()
{
    std::for_each(m_sites.begin(), m_sites.end(), Util::SafeDelete());
    m_sites.clear();
}

void SiteItemGroup::DeleteSite(SiteItem & site)
{
    ASSERT(site.GetParentGroup() == this);
    m_sites.erase(std::remove(m_sites.begin(), m_sites.end(), &site), m_sites.end());
    delete &site;
}

SiteItemGroup::SiteIterator SiteItemGroup::GetSitesBegin()
{
    return m_sites.begin();
}

SiteItemGroup::ConstSiteIterator SiteItemGroup::GetSitesBegin() const
{
    return m_sites.begin();
}

SiteItemGroup::SiteIterator SiteItemGroup::GetSitesEnd()
{
    return m_sites.end();
}

SiteItemGroup::ConstSiteIterator SiteItemGroup::GetSitesEnd() const
{
    return m_sites.end();
}

// Site management - Deals with direct sites

bool SiteItemGroup::Check(CSiteCheckDlg::Hook & hook, CWnd *parent)
{
    Sites sites;
    AddSitesIf(*this, std::back_inserter(sites), IncludeDefaultCheckedSites(), IncludeAllGroups());
    if (sites.empty() == true)
        return true;
    SiteOperationManager::Check check(hook, parent);
    return check(sites.begin(), sites.end());
}

void SiteItemGroup::UnmarkAllSites()
{
    Sites sites;
    AddSitesIf(*this, std::back_inserter(sites), IncludeAllSites(), IncludeAllGroups());
    if (sites.empty() == true)
        return;
    SiteOperationManager::Unmark()(sites.begin(), sites.end());
}

// Auto-Checking management

void SiteItemGroup::SetNextCheck()
{
    if (m_prop.autoCheckData.period == 0) {
        m_prop.autoCheckData.nextCheck = 0;
        return;
    }

    time(&m_prop.autoCheckData.nextCheck);

    // Period is stored as a count of hour-halves
    m_prop.autoCheckData.nextCheck += (30 * 60) * m_prop.autoCheckData.period;
}

std::time_t SiteItemGroup::GetAutoCheckTime() const
{
    return m_prop.autoCheckData.nextCheck;
}

void SiteItemGroup::SetIncludedInAutoCheck(bool included)
{
    m_prop.autoCheckData.included = included;
}

void SiteItemGroup::SetAutoCheckPeriod(int period)
{
    if (period >= 0 && period <= AutoCheckData::MaxPeriods) {
        m_prop.autoCheckData.period = period;
    } else {
        ASSERT(!"Invalid auto-check period");
        m_prop.autoCheckData.period = 0;
    }
}

bool SiteItemGroup::IsIncludedInAutoCheck() const
{
    return m_prop.autoCheckData.included;
}

int SiteItemGroup::GetAutoCheckMaxPeriods()
{
    return AutoCheckData::MaxPeriods;
}

int SiteItemGroup::GetAutoCheckPeriod() const
{
    return m_prop.autoCheckData.period;
}

// Proxy management

void SiteItemGroup::SetProxyData(const ProxyData & proxyData)
{
    m_prop.proxyData = proxyData;
}

SiteItemGroup::ProxyData SiteItemGroup::GetProxyData() const
{
    return m_prop.proxyData;
}

// Misc. helper functions

#ifndef NDEBUG

bool SiteItemGroup::SanityCheck() const
{
    {
    // Make sure all child groups know we are their parent
    Groups::const_iterator it = m_groups.begin();
    Groups::const_iterator end = m_groups.end();

    while (it != end) {
        const SiteItemGroup *group = *it;
        if (group->m_parent != this) {
            ASSERT(!"Child-group parent mismatch");
            return false;
        }
        ++it;
    }
    }

    {
    // Make sure all child sites know we are their parent
    Sites::const_iterator it = m_sites.begin();
    Sites::const_iterator end = m_sites.end();

    while (it != end) {
        const SiteItem *site = *it;
        if (site->GetParentGroup() != this) {
            ASSERT(!"Child-site parent mismatch");
            return false;
        }
        ++it;
    }
    }

    return true;
}

#endif // !defined(NDEBUG)

XML::CXML *SiteItemGroup::Write(XML::CXML & xmlParent) const
{
    XML::CXML *xml = xmlParent.CreateChild(GetXMLName());
    
    if (xml) {
        xml->CreateChild("Name", m_prop.name.c_str());

        // Save auto-check parameters if not default
        if (m_prop.autoCheckData.period || m_prop.autoCheckData.included == false) {
            XML::CXML *xmlAutoCheck = xml->CreateChild("AutoCheck");
            if (xmlAutoCheck) {
                if (m_parent)
                    xmlAutoCheck->SetParam("Included", m_prop.autoCheckData.included);
                xmlAutoCheck->SetParam("Period", m_prop.autoCheckData.period);
            }
        }

        // Save proxy settings
        XML::CXML *xmlProxy = xml->CreateChild("Proxy");
        if (xmlProxy) {
            std::string usage;
            switch (m_prop.proxyData.usage) {
                case ProxyData::useProxy:
                    usage = "Use";
                    break;
                case ProxyData::inheritProxy:
                    usage = "Inherit";
                    break;
                case ProxyData::noProxy:
                default:
                    usage = "No";
                    break;
            }
            xmlProxy->SetParam("Usage", usage.c_str());

            if (m_prop.proxyData.address.empty() == false)
                xmlProxy->CreateChild("Address", m_prop.proxyData.address.c_str());
            
            if (m_prop.proxyData.type.empty() == false)
                xmlProxy->SetParam("Type", m_prop.proxyData.type.c_str());
        }

        std::for_each(m_groups.begin(), m_groups.end(), boost::bind(&SiteItemGroup::Write, _1, boost::ref(*xml)));
        std::for_each(m_sites.begin(), m_sites.end(), Util::DereferencingStreamInserter<XML::CXML, SiteItem>(*xml));
    }
    
    return xml;
}

const char *SiteItemGroup::GetXMLName()
{
    return "SiteItemGroup";
}

void SiteItemGroup::MoveGroup(SiteItemGroup & group)
{
    GroupMover(*this)(&group);
    ASSERT(SanityCheck());
}

void SiteItemGroup::MergeGroup(SiteItemGroup & group)
{
    if (&group == this)
        return;

    ASSERT(Store::Instance().IsRootGroup(group) == false);
    ASSERT(IsChildOf(group, *this) == false);

    // NOTE: I don't use std::for_each() because it caches the iterators
    
    SiteMover siteMover(*this);
    while (group.GetSiteCount() > 0)
        siteMover(*(group.GetSitesEnd() - 1));

    GroupMover groupMover(*this);
    while (group.GetGroupCount() > 0)
        groupMover(*(group.GetGroupsEnd() - 1));

    SiteItemGroup *parent = group.GetParentGroup();
    ASSERT(parent);

    parent->DeleteGroup(group);

    ASSERT(SanityCheck());
}

void SiteItemGroup::MoveSite(SiteItem & site)
{
    SiteMover(*this)(&site);
    ASSERT(SanityCheck());
}

SiteItemGroup & SiteItemGroup::DetachGroup(SiteItemGroup & group)
{
    ASSERT(group.m_parent);
    Groups & groups = group.m_parent->m_groups;
    groups.erase(std::remove(groups.begin(), groups.end(), &group));
    group.m_parent = 0;
    return group;
}

SiteItem & SiteItemGroup::DetachSite(SiteItem & site)
{
    SiteItemGroup *parent = site.GetParentGroup();
    ASSERT(parent);
    Sites & sites = parent->m_sites;
    sites.erase(std::remove(sites.begin(), sites.end(), &site));
    site.SetParentGroup(0);
    return site;
}

// Free functions

int GetOverallSiteCount(const SiteItemGroup & group)
{
    return std::accumulate(group.GetGroupsBegin(), group.GetGroupsEnd(), group.GetSiteCount(), OverallSiteCountAccumulator());
}

bool IsChildOf(const SiteItemGroup & pgroup, const SiteItemGroup & cgroup)
{
    const SiteItemGroup *parent = cgroup.GetParentGroup();
    while (parent) {
        if (Store::Instance().IsRootGroup(*parent) == true)
            return Store::Instance().IsRootGroup(pgroup);

        if (parent == &pgroup)
            return true;

        parent = parent->GetParentGroup();
    }

    return false;
}

} 
