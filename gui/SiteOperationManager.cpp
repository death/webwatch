// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteOperationManager.cpp $ - Site operation manager
//
// $Author: Death $
// $Revision: 4 $
// $Date: 10/28/05 18:59 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/ThreadUtil.h"
#include "util/OpenBrowser.h"

#include "SiteOperationManager.h"
#include "SitePropertiesDlg.h"
#include "CheckSettings.h"
#include "Store.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace 
{
    bool currentlyChecking;
    Util::CriticalSection currentlyCheckingCS;
}

namespace WebWatch
{
    namespace SiteOperationManager
    {
        SiteItem *Add()
        {
            std::auto_ptr<SiteItem> site(new SiteItem("", ""));
            CSitePropertiesDlg dlg(*site, "Add Site");
            return dlg.DoModal() == IDOK ? site.release() : 0;
        }

        bool Modify(SiteItem & site)
        {
            SiteItem copy(site);
            CSitePropertiesDlg dlg(copy, "Modify Site");
            return dlg.DoModal() == IDOK ? (site = copy, true) : false;
        }

        bool IsCurrentlyChecking()
        {
            Util::ScopedLock lock(currentlyCheckingCS);
            return currentlyChecking;
        }

        namespace Private
        {
            bool StartChecking()
            {
                Util::ScopedLock lock(currentlyCheckingCS);
                if (currentlyChecking == true)
                    return false;
                currentlyChecking = true;
                return true;
            }

            void EndChecking()
            {
                Util::ScopedLock lock(currentlyCheckingCS);
                ASSERT(currentlyChecking == true);
                currentlyChecking = false;
            }

            UINT BrowseThread(LPVOID param)
            {
                ASSERT(param);

                std::auto_ptr<BrowseParameters> params(
                    static_cast<BrowseParameters *>(param));

                std::for_each(params->addresses.begin(), 
                              params->addresses.end(), 
                              Util::OpenBrowser(params->hwndParent));

                return 0;
            }
        }
    }
} 
