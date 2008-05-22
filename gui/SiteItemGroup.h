// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteItemGroup.h $ - SiteItem group
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SITEITEMGROUP_H
#define WEBWATCH_INC_SITEITEMGROUP_H

#include "util/Util.h"
#include "util/Common.h"

#include "SiteCheckDlg.h"

namespace WebWatch
{

class SiteItem;

class SiteItemGroup
{
    typedef std::vector<SiteItem *> Sites;
    typedef std::vector<SiteItemGroup *> Groups;

    class SiteMover;
    class GroupMover;

    struct AutoCheckData
    {
        enum { MaxPeriods = 20 };

        std::time_t nextCheck;
        int period; // Count of 1/2 hours
        bool included; // Is the group included in parent's auto-check?
    };

public: // Public data types
    struct ProxyData
    {
        ProxyData()
        : usage(inheritProxy)
        {
        }

        enum Usage
        {
            noProxy, useProxy, inheritProxy
        };

        Usage usage;
        std::string address;
        std::string type;
    };

private: // Private data types
    struct Properties
    {
        std::string name;
        AutoCheckData autoCheckData;
        ProxyData proxyData;
    };

public: // Public data types
    typedef Sites::const_iterator ConstSiteIterator;
    typedef Sites::iterator SiteIterator;

    typedef Groups::const_iterator ConstGroupIterator;
    typedef Groups::iterator GroupIterator;

public: // Public interface
    explicit SiteItemGroup(const char *name);
    explicit SiteItemGroup(const XML::CXML & xml);
    explicit SiteItemGroup(const Properties & prop);
    ~SiteItemGroup();

    void SetName(const char *name);
    std::string GetName() const;

    SiteItemGroup *GetParentGroup();
    const SiteItemGroup *GetParentGroup() const;

    Properties GetProperties() const;
    void SetProperties(const Properties & prop);

    // Sub-group management
    void AddGroup(SiteItemGroup & group);
    void DeleteAllGroups();
    int GetGroupCount() const;
    void DeleteGroup(SiteItemGroup & group);

    GroupIterator GetGroupsBegin();
    ConstGroupIterator GetGroupsBegin() const;
    GroupIterator GetGroupsEnd();
    ConstGroupIterator GetGroupsEnd() const;

    void MoveGroup(SiteItemGroup & group);
    void MergeGroup(SiteItemGroup & group);

    // Site management - Deals with direct sites
    void AddSite(SiteItem & site);
    void DeleteAllSites();
    void DeleteSite(SiteItem & site);
    int GetSiteCount() const;
    
    SiteIterator GetSitesBegin();
    ConstSiteIterator GetSitesBegin() const;
    SiteIterator GetSitesEnd();
    ConstSiteIterator GetSitesEnd() const;

    // Site management - Deals with all sites (also from sub-groups)
    bool Check(CSiteCheckDlg::Hook & hook, CWnd *parent = 0);
    void UnmarkAllSites();

    void MoveSite(SiteItem & site);

    // Auto-Check management
    std::time_t GetAutoCheckTime() const;
    void SetNextCheck();
    void SetIncludedInAutoCheck(bool included);
    void SetAutoCheckPeriod(int period);
    bool IsIncludedInAutoCheck() const;
    int GetAutoCheckPeriod() const;
    static int GetAutoCheckMaxPeriods();

    // Proxy management
    void SetProxyData(const ProxyData & proxyData);
    ProxyData GetProxyData() const;

    static const char *GetXMLName();

private: // Private interface

#ifndef NDEBUG
    bool SanityCheck() const;
#endif // !defined(NDEBUG)

    friend inline XML::CXML & operator<< (XML::CXML & xmlParent, const SiteItemGroup & group);
    XML::CXML *Write(XML::CXML & xmlParent) const;

    static SiteItemGroup & DetachGroup(SiteItemGroup & group);
    static SiteItem & DetachSite(SiteItem & site);

private: // Preventive declarations
    SiteItemGroup(const SiteItemGroup &);
    SiteItemGroup & operator= (const SiteItemGroup &);

private: // Data members
    Sites m_sites;
    Groups m_groups;
    Properties m_prop;
    SiteItemGroup *m_parent;
};

// SiteItemGroup interface - Free functions

template<class OutIter, class SitePred, class GroupPred>
void AddSitesIf(const SiteItemGroup & group, OutIter out, SitePred spred, GroupPred gpred);

template<class OutIter, class SitePred, class GroupPred>
class AddGroupIf
{
    OutIter m_out;
    SitePred m_spred;
    GroupPred m_gpred;
public:
    AddGroupIf(OutIter out, SitePred spred, GroupPred gpred)
    : m_out(out), m_spred(spred), m_gpred(gpred) {}

    void operator() (const SiteItemGroup *group)
    {
        if (group && m_gpred(group))
            AddSitesIf(*group, m_out, m_spred, m_gpred);
    }
};

template<class OutIter, class SitePred, class GroupPred>
void AddSitesIf(const SiteItemGroup & group, OutIter out, SitePred spred, GroupPred gpred)
{
    Util::CopyIf(group.GetSitesBegin(), group.GetSitesEnd(), out, spred);
    std::for_each(group.GetGroupsBegin(), group.GetGroupsEnd(), AddGroupIf<OutIter, SitePred, GroupPred>(out, spred, gpred));
}

// Gathers (direct or indirect) child groups of the specified group
template<class OutIter>
void GatherChildGroups(SiteItemGroup & parent, OutIter outIt)
{
    SiteItemGroup::GroupIterator it = parent.GetGroupsBegin();
    SiteItemGroup::GroupIterator end = parent.GetGroupsEnd();

    while (it != end) {
        SiteItemGroup *group = *it;
        if (group) {
            *outIt++ = group;
            GatherChildGroups(*group, outIt);
        }
        ++it;
    }
}

template<class OutIter>
void GatherChildGroups(const SiteItemGroup & parent, OutIter outIt)
{
    SiteItemGroup::ConstGroupIterator it = parent.GetGroupsBegin();
    SiteItemGroup::ConstGroupIterator end = parent.GetGroupsEnd();

    while (it != end) {
        const SiteItemGroup *group = *it;
        if (group) {
            *outIt++ = group;
            GatherChildGroups(*group, outIt);
        }
        ++it;
    }
}

// Retrieves count of sites owned by the group itself and its (direct or indirect)
// child groups
int GetOverallSiteCount(const SiteItemGroup & group);

// Checks whether 'cgroup' is (direct or indirect) child of 'pgroup'
bool IsChildOf(const SiteItemGroup & pgroup, const SiteItemGroup & cgroup);

inline XML::CXML & operator<< (XML::CXML & xmlParent, const SiteItemGroup & group)
{
    group.Write(xmlParent);
    return xmlParent;
}

// All exceptions thrown by SiteItemGroup related functions are of the following
// type

class SiteItemGroupError : public std::runtime_error
{
public:
    SiteItemGroupError(const std::string & error)
    : std::runtime_error(error)
    {
    }
};

} 

#endif // WEBWATCH_INC_SITEITEMGROUP_H