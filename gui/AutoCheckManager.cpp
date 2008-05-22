// WebWatch UI - by DEATH, 2004
//
// $Workfile: AutoCheckManager.cpp $ - Auto-Check manager
//
// $Author: Death $
// $Revision: 3 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "AutoCheckManager.h"
#include "SiteItem.h"
#include "SiteItemGroup.h"
#include "SiteOperationManager.h"
#include "util/Common.h"

namespace WebWatch
{
    namespace
    {
        struct IncludeDefaultCheckedSites
        {
            bool operator() (const SiteItem *site) const
            {
                return site->IsCheckedByDefault();
            }
        };
    }

    AutoCheckManager::AutoCheckManager(CSiteCheckDlg::Hook & checkHook, SiteItemGroup & rootGroup)
    : m_rootGroup(rootGroup)
    , m_checkHook(checkHook)
    {
    }

    bool AutoCheckManager::AutoCheck(CWnd *parent)
    {
        if (SiteOperationManager::IsCurrentlyChecking())
            return false;

        std::time_t now = time(0);
        SiteItemGroup *group = GetNearestAutoCheckGroup();

        if (group && (group->GetAutoCheckTime() <= now))
            return CheckGroup(*group, parent);

        return false;
    }

    SiteItemGroup *AutoCheckManager::GetNearestAutoCheckGroup()
    {
        return GetNearestAutoCheckGroup(m_rootGroup);
    }

    SiteItemGroup *AutoCheckManager::GetNearestAutoCheckGroup(SiteItemGroup & rootGroup, SiteItemGroup *nearestGroup)
    {
        std::time_t rootGroupNextCheck = rootGroup.GetAutoCheckTime();
        if (rootGroupNextCheck) {
            if (nearestGroup == 0) {
                nearestGroup = &rootGroup;
            } else {
                if (nearestGroup->GetAutoCheckTime() > rootGroupNextCheck)
                    nearestGroup = &rootGroup;
            }
        }

        SiteItemGroup::ConstGroupIterator it = rootGroup.GetGroupsBegin();
        SiteItemGroup::ConstGroupIterator end = rootGroup.GetGroupsEnd();

        for (; it != end; ++it) {
            if (SiteItemGroup *group = *it)
                nearestGroup = GetNearestAutoCheckGroup(*group, nearestGroup);
        }

        return nearestGroup;
    }

    bool AutoCheckManager::CheckGroup(SiteItemGroup & group, CWnd *parent)
    {
        std::vector<SiteItem *> sites;
        AddSitesIf(group, 
                   std::back_inserter(sites), 
                   std::mem_fun(&SiteItem::IsCheckedByDefault), 
                   std::mem_fun(&SiteItemGroup::IsIncludedInAutoCheck));
        SiteOperationManager::Check check(m_checkHook, parent);
        bool checked = false;

        try {
            checked = check(sites.begin(), sites.end());
        }
        catch (...) {
            // This is to avoid infinite auto check loops
            group.SetNextCheck();
            throw;
        }

        if (checked)
            group.SetNextCheck();

        return checked;
    }
}
