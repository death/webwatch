// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteOperationManager.h $ - Site operation manager
//
// $Author: Death $
// $Revision: 3 $
// $Date: 5/07/05 20:03 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SITE_OPERATION_MANAGER_H
#define WEBWATCH_INC_SITE_OPERATION_MANAGER_H

#include "util/Util.h"
#include "util/Clipboard.h"

#include "core/Core.h"
#include "core/CoreException.h"

#include "SiteItem.h"
#include "SetMUDlg.h"
#include "SiteCheckDlg.h"

namespace WebWatch
{
    class SiteItem;

    namespace SiteOperationManager
    {
        namespace Private
        {
            bool StartChecking();
            void EndChecking();

            // TODO: Not sure about the name
            class SiteStateAssign
            {
            public:
                SiteStateAssign(SiteItem::State state, 
                                const std::string & verbose)
                : m_state(state)
                , m_verbose(verbose)
                {
                }

                void operator() (SiteItem *site)
                {
                    site->SetState(m_state);
                    if (m_verbose.empty() == false)
                        site->SetVerboseStatus(m_verbose.c_str());
                }

            private:
                SiteItem::State m_state;
                std::string m_verbose;
            };

            struct SiteEndMUSession
            {
                void operator() (SiteItem *site)
                {
                    if (site->IsMUInfoAvailable())
                        site->ResetMU();
                }
            };

            struct BrowseParameters
            {
                typedef std::vector<std::string> Addresses;

                Addresses addresses;
                HWND hwndParent;
            };

            UINT BrowseThread(LPVOID param);
        }

        SiteItem *Add();
        bool Modify(SiteItem & site);

        struct CopyAddresses
        {
            explicit CopyAddresses(CWnd & wnd)
            : m_wnd(wnd)
            {
            }

            template<class Iter>
            bool operator() (Iter first, Iter last)
            {
                std::ostringstream os;
                bool newline = false;

                while (first != last) {
                    const SiteItem *site = *first;
                    if (site) {
                        if (newline)
                            os << "\r\n";
                        os << site->GetAddress();
                        newline = true;
                    }
                    ++first;
                }

                const std::string & s = os.str();
                return Util::CopyTextToClipboard(m_wnd.GetSafeHwnd(), s);
            }

        private:
            CWnd & m_wnd;
        };

        bool IsCurrentlyChecking();

        struct Check
        {
            explicit Check(CSiteCheckDlg::Hook & hook, CWnd *parent = 0)
            : m_hook(hook)
            , m_parent(parent)
            {
            }

            template<class Iter>
            bool operator() (Iter first, Iter last)
            {
                if (Private::StartChecking() == false)
                    return false;

                try {
                    CSiteCheckDlg::SiteItems sites(first, last);
                    CSiteCheckDlg dlg(sites, m_hook, m_parent);

                    switch (dlg.DoModal()) {
                        case IDOK:
                            break;
                        case IDCANCEL:
                        default:
                            break;
                    }
                }
                catch(...) {
                    Private::EndChecking();
                    throw;
                }

                Private::EndChecking();

                return true;
            }

        private:
            CSiteCheckDlg::Hook & m_hook;
            CWnd *m_parent;
        };

        struct Browse
        {
            explicit Browse(CWnd & wnd)
            : m_wnd(wnd)
            {
            }

            template<class Iter>
            bool operator() (Iter first, Iter last)
            {
                Private::BrowseParameters *params = new Private::BrowseParameters;
                params->addresses.reserve(std::distance(first, last));
                std::transform(first, 
                               last, 
                               std::back_inserter(params->addresses), 
                               boost::mem_fn(&SiteItem::GetAddress));
                params->hwndParent = m_wnd.GetSafeHwnd();
                AfxBeginThread(Private::BrowseThread, params);
                return true;
            }

        private:
            CWnd & m_wnd;
        };

        struct MarkAsUpdated
        {
            template<class Iter>
            bool operator() (Iter first, Iter last)
            {
                using Private::SiteStateAssign;
                std::for_each(first, 
                              last, 
                              SiteStateAssign(SiteItem::updated, "Updated"));
                return true;
            }
        };

        struct Unmark
        {
            template<class Iter>
            bool operator() (Iter first, Iter last)
            {
                std::for_each(first, 
                              last, 
                              Private::SiteStateAssign(SiteItem::none, "OK"));
                return true;
            }
        };

        struct BeginMUSession
        {
            template<class Iter>
            bool operator() (Iter first, Iter last)
            {
                time_t now = time(0);
                
                CSetMUDlg dlg(now);
                if (dlg.DoModal() == IDCANCEL)
                    return false;

                time_t expiry = dlg.GetExpiry();
                std::for_each(first, 
                              last, 
                              boost::bind(&SiteItem::SetMU, _1, expiry));
                return true;
            }
        };

        struct EndMUSession
        {
            template<typename Iter>
            bool operator() (Iter first, Iter last)
            {
                std::for_each(first, last, Private::SiteEndMUSession());
                return true;
            }
        };
    }
} 

#endif // WEBWATCH_INC_SITE_OPERATION_MANAGER_H