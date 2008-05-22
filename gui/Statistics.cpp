// WebWatch UI - by DEATH, 2004
//
// $Workfile: Statistics.cpp $ - Statistics provider
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "Statistics.h"
#include "SiteItemGroup.h"

namespace {
    // Determines whether a site is close to MU expiry
    struct IsSiteCloseToMUExpiry : public std::unary_function<const WebWatch::SiteItem *, bool>
    {
        bool operator() (const WebWatch::SiteItem *site) const
        {
            if (site->IsMUInfoAvailable()) {
                int days = site->GetMUDayCount(std::time(0));
                return days == 0;
            }

            return false;
        }
    };

    // Compares a site's state to a given state
    class SiteStateCompare : public std::unary_function<const WebWatch::SiteItem *, bool>
    {
    public:
        SiteStateCompare(WebWatch::SiteItem::State state)
        : m_state(state)
        {
        }

        bool operator() (const WebWatch::SiteItem *site) const
        {
            ASSERT(site);
            return site->GetState() == m_state;
        }

    private:
        WebWatch::SiteItem::State m_state;
    };

    class SiteStateCounter : public std::binary_function<int, const WebWatch::SiteItemGroup *, int>
    {
    public:
        explicit SiteStateCounter(WebWatch::SiteItem::State state)
        : m_state(state)
        {
        }

        int operator() (int count, const WebWatch::SiteItemGroup *group) const
        {
            ASSERT(group);
            return count + static_cast<int>(std::count_if(group->GetSitesBegin(), group->GetSitesEnd(), SiteStateCompare(m_state)));
        }
        
    private:
        WebWatch::SiteItem::State m_state;
    };

    struct SiteMUCounter : public std::binary_function<int, const WebWatch::SiteItemGroup *, int>
    {
        int operator() (int count, const WebWatch::SiteItemGroup *group) const
        {
            ASSERT(group);
            return count + static_cast<int>(std::count_if(group->GetSitesBegin(), group->GetSitesEnd(), IsSiteCloseToMUExpiry()));
        }
    };

    template<class Func>
    int SiteStatisticsAccumulator(const WebWatch::SiteItemGroup & group, WebWatch::Statistics::CountType type, Func func)
    {
        std::vector<const WebWatch::SiteItemGroup *> countedGroups;
        countedGroups.push_back(&group);

        switch (type) {
            case WebWatch::Statistics::directCount:
                break;
            case WebWatch::Statistics::deepCount:
                std::copy(group.GetGroupsBegin(), group.GetGroupsEnd(), std::back_inserter(countedGroups));
                break;
            case WebWatch::Statistics::recursiveCount:
                WebWatch::GatherChildGroups(group, std::back_inserter(countedGroups));
                break;
            default:
                ASSERT(!"Statistics: Unknown count type");
                break;
        }
        
        return std::accumulate(countedGroups.begin(), countedGroups.end(), 0, func);
    }
}

namespace WebWatch
{

namespace Statistics
{

int GetSiteCountByState(const SiteItemGroup & group, SiteItem::State state, CountType type)
{
    return SiteStatisticsAccumulator(group, type, SiteStateCounter(state));
}

int GetMUSiteCount(const SiteItemGroup & group, CountType type)
{
    return SiteStatisticsAccumulator(group, type, SiteMUCounter());
}

}

} 
