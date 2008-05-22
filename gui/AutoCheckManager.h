// WebWatch UI - by DEATH, 2004
//
// $Workfile: AutoCheckManager.h $ - Auto-Check manager
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_AUTOCHECKMANAGER_H
#define WEBWATCH_INC_AUTOCHECKMANAGER_H

#include "SiteCheckDlg.h"

namespace WebWatch
{

class SiteItemGroup;
class SiteItem;

class AutoCheckManager
{
public:
    explicit AutoCheckManager(CSiteCheckDlg::Hook & checkHook, SiteItemGroup & rootGroup);

    bool AutoCheck(CWnd *parent = 0);
    SiteItemGroup *GetNearestAutoCheckGroup();

private:
    SiteItemGroup *GetNearestAutoCheckGroup(SiteItemGroup & rootGroup, SiteItemGroup *nearestGroup = 0);
    bool CheckGroup(SiteItemGroup & group, CWnd *parent);

private:
    SiteItemGroup & m_rootGroup;
    CSiteCheckDlg::Hook & m_checkHook;
};

} 

#endif // WEBWATCH_INC_AUTOCHECKMANAGER_H