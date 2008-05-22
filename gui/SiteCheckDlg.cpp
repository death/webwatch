// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteCheckDlg.cpp $ - Site check dialog
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/ThreadUtil.h"

#include "core/Core.h"
#include "core/CheckMethod.h"

#include "SiteCheckDlg.h"
#include "SiteItem.h"
#include "Store.h"
#include "CheckSettings.h"
#include "SiteItemGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

// HACK: If lame min macro is defined, we undef it
#ifdef min
#undef min
#endif // min

namespace {
    const UINT_PTR HideWindowTimerID = 1;

    struct CheckSiteParams
    {
        CheckSiteParams(CSiteCheckDlg & checkDlg_, WebWatch::SiteItem & site_, int threadNumber_)
        : checkDlg(checkDlg_)
        , site(site_)
        , threadNumber(threadNumber_)
        , aborted(false)
        {
        }

        CSiteCheckDlg & checkDlg;
        WebWatch::SiteItem & site;
        int threadNumber;
        bool aborted;
    };

    Util::CriticalSection csAbort;

    class MyStatusNotifyClient : public Core::StatusNotifyClient
    {
    public:
        MyStatusNotifyClient(CListCtrl & lst, int threadNumber);
        virtual ~MyStatusNotifyClient();
        virtual void SetStatus(const char *status);

    private:
        CListCtrl & m_lst;
        int m_threadNumber;
    };

    void SetProxySettings(WebWatch::SiteItem & site);
}

BEGIN_MESSAGE_MAP(CSiteCheckDlg, CDialog)
    ON_BN_CLICKED(IDC_ABORT, OnAbort)
    ON_BN_CLICKED(IDC_MINIMIZE, OnMinimize)
    ON_WM_CLOSE()
    ON_WM_TIMER()
END_MESSAGE_MAP()

CSiteCheckDlg::CSiteCheckDlg(const CSiteCheckDlg::SiteItems & sites, Hook & hook, CWnd *parent)
: CDialog(CSiteCheckDlg::IDD, parent)
, m_sites(sites)
, m_hook(hook)
, m_abort(false)
{
}

void CSiteCheckDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_CHECKER_LIST, m_checkerList);
    CDialog::DoDataExchange(pDX);
}

BOOL CSiteCheckDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    // Start as hidden dialog if parent is not the foreground window
    // and determine whether we have a minimize option
    bool hasMinimize = false;
    CWnd *parent = GetParent();
    if (parent) {
        CWnd *foreground = GetForegroundWindow();
        if (foreground == 0 || parent->m_hWnd != foreground->m_hWnd) {
            if (parent->IsIconic() || parent->IsWindowVisible() == FALSE) {
                // This is to avoid flicker
                SetWindowPos(&wndBottom, -1000, -1000, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
                // Notify parent that this dialog exists as well
                OnMinimize();
                SetTimer(HideWindowTimerID, 0, 0);
            } else {
                SetWindowPos(parent, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
                CenterWindow();
            }
        }

        if (parent->m_hWnd == AfxGetMainWnd()->m_hWnd)
            hasMinimize = true;
    }

    GetDlgItem(IDC_MINIMIZE)->ShowWindow(hasMinimize ? SW_SHOW : SW_HIDE);

    SetDlgItemText(IDC_SITES_CHECKED_INFO, "Checking site(s)...");
    
    CRect rect;
    m_checkerList.GetClientRect(&rect);
    m_checkerList.SetExtendedStyle(m_checkerList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    int width = rect.Width();
    m_checkerList.InsertColumn(0, "#", LVCFMT_LEFT, width * 8 / 100);
    m_checkerList.InsertColumn(1, "Name", LVCFMT_LEFT, width * 60 / 100);
    m_checkerList.InsertColumn(2, "Status", LVCFMT_LEFT, width * 30 / 100);

    AfxBeginThread(CheckThread, this);
    return TRUE;
}

UINT CSiteCheckDlg::CheckThread(LPVOID param)
{
    CSiteCheckDlg *self = static_cast<CSiteCheckDlg *>(param);
    return self->Check();
}

UINT CSiteCheckDlg::Check()
{
    m_abort = false;

    WebWatch::CheckSettings checkSettings;
    WebWatch::Store::Instance().GetCheckSettings(checkSettings);
    int threadCount = checkSettings.GetThreadCount();

    typedef std::vector<std::pair<CheckSiteParams *, CWinThread *> > Threads;
    Threads threads(threadCount, std::make_pair(static_cast<CheckSiteParams *>(0), static_cast<CWinThread *>(0)));
    Threads::iterator threadsEnd = threads.end();
    Threads::iterator threadsBegin = threads.begin();

    SiteItems::iterator currentSite = m_sites.begin();
    SiteItems::iterator sitesEnd = m_sites.end();

    int sitesChecked = 0;
    int siteCount = static_cast<int>(m_sites.size());

    m_checkerList.DeleteAllItems();
    int rows = std::min(siteCount, threadCount);
    for (int i = 0; i < rows; i++) {
        std::ostringstream os;
        os << (i + 1);
        m_checkerList.InsertItem(i, os.str().c_str());
    }

    CProgressCtrl progress;
    CWnd *wnd = GetDlgItem(IDC_PROGRESS);
    ASSERT(wnd);
    progress.Attach(wnd->m_hWnd);
    progress.SetRange32(0, siteCount);
    progress.SetStep(1);
    progress.SetPos(0);

    while (1) {
        int emptyEntryCount = 0;
        bool abort = m_abort;

        Threads::iterator it = threads.begin();
        while (it != threadsEnd) {
            CWinThread *thread = it->second;
            if (thread == 0) {
                if ((abort == false) && (currentSite != sitesEnd)) {
                    // Start a new thread
                    CheckSiteParams *params = new CheckSiteParams(*this, **currentSite, static_cast<int>(std::distance(threadsBegin, it)));
                    it->first = params;
                    ++currentSite;
                    thread = AfxBeginThread(CheckSiteThread, &it->first, 0, 0, CREATE_SUSPENDED);
                    thread->m_bAutoDelete = FALSE;
                    it->second = thread;
                    thread->ResumeThread();
                } else {
                    emptyEntryCount++;
                }
            } else {
                // A thread exists, check if it's still running - if not, assign
                // null to the entry in threads table
                if (WaitForSingleObject(thread->m_hThread, 0) == WAIT_OBJECT_0) {
                    *it = std::make_pair(static_cast<CheckSiteParams *>(0), static_cast<CWinThread *>(0));
                    delete thread;
                    emptyEntryCount++;
                    
                    if (abort == false) {
                        sitesChecked++;
                        progress.StepIt();

                        std::ostringstream os;
                        os << sitesChecked << " / " << siteCount << " Sites were checked";
                        SetDlgItemText(IDC_SITES_CHECKED_INFO, os.str().c_str());
                    }
                } else {
                    // The thread is still running, abort if necessary
                    if (abort) {
                        Util::ScopedLock lock(csAbort);
                        CheckSiteParams *params = it->first;
                        if (params && params->aborted == false) {
                            params->aborted = true;
                            params->site.Abort();
                        }
                    }
                }
            }
            ++it;
        }

        if (emptyEntryCount == threadCount) {
            if (abort || currentSite == sitesEnd)
                break;
        }

        Sleep(1);
    }

    progress.Detach();
    
    if (m_abort == false)
        m_hook.OnAllSitesChecked(*this);

    PostMessage(WM_CLOSE);

    return 0;
}

UINT CSiteCheckDlg::CheckSiteThread(LPVOID param)
{
    CheckSiteParams **checkParamsPtr = static_cast<CheckSiteParams **>(param);
    ASSERT(checkParamsPtr);
    CheckSiteParams *checkParams = *checkParamsPtr;
    ASSERT(checkParams);
    UINT ret = checkParams->checkDlg.CheckSite(checkParams->site, checkParams->threadNumber);
    Util::ScopedLock lock(csAbort);
    *checkParamsPtr = 0;
    delete checkParams;
    return ret;
}

UINT CSiteCheckDlg::CheckSite(WebWatch::SiteItem & site, int threadNumber)
{
    m_checkerList.SetItem(threadNumber, 1, LVIF_TEXT, site.GetName().c_str(), 0, 0, 0, 0);
    std::auto_ptr<MyStatusNotifyClient> statusNotifyClient(new MyStatusNotifyClient(m_checkerList, threadNumber));
    
    SetProxySettings(site);

    try {
        XML::CXML results;
        site.Check(results, statusNotifyClient.get());
        m_hook.OnSiteChecked(site, results);
    }
    catch (const Core::Exception & ex) {
        m_hook.OnException(site, ex);
    }

    m_checkerList.SetItem(threadNumber, 1, LVIF_TEXT, "", 0, 0, 0, 0);
    statusNotifyClient->SetStatus("");

    return 0;
}

void CSiteCheckDlg::OnAbort()
{
    GetDlgItem(IDC_ABORT)->EnableWindow(FALSE);
    m_abort = true;
}

void CSiteCheckDlg::OnCancel()
{
}

void CSiteCheckDlg::OnOK()
{
}

void CSiteCheckDlg::OnClose()
{
    EndDialog(m_abort ? IDCANCEL : IDOK);
}

void CSiteCheckDlg::OnMinimize()
{
    // Send a minimize message to parent
    CWnd *parent = GetParent();
    ASSERT(parent);
    CRect rect;
    parent->GetClientRect(&rect);
    parent->PostMessage(WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(rect.Width(), rect.Height()));
}

void CSiteCheckDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent) {
        case HideWindowTimerID:
            ShowWindow(SW_HIDE);
            CenterWindow();
            KillTimer(nIDEvent);
            return;
        default:
            break;
    }
    
    CDialog::OnTimer(nIDEvent);
}

namespace {

MyStatusNotifyClient::MyStatusNotifyClient(CListCtrl & lst, int threadNumber)
: m_lst(lst)
, m_threadNumber(threadNumber)
{
}

MyStatusNotifyClient::~MyStatusNotifyClient()
{
}

void MyStatusNotifyClient::SetStatus(const char *status)
{
    m_lst.SetItem(m_threadNumber, 2, LVIF_TEXT, status, 0, 0, 0, 0);
}

void SetProxySettings(WebWatch::SiteItem & site)
{
    WebWatch::SiteItemGroup *group = site.GetParentGroup();
    if (group) {
        // Get proxy settings (consider settings inheritance)
        WebWatch::SiteItemGroup::ProxyData proxyData = group->GetProxyData();
        while (proxyData.usage == WebWatch::SiteItemGroup::ProxyData::inheritProxy) {
            WebWatch::SiteItemGroup *parent = group->GetParentGroup();
            if (parent) {
                group = parent;
                proxyData = group->GetProxyData();
            } else {
                proxyData.usage = WebWatch::SiteItemGroup::ProxyData::noProxy;
            }
        }
        
        XML::CXML params;
        site.GetCheckParameters(params);
        bool removeProxy = true;
        XML::CXML *xmlSocket = params.FindFirstChild("Socket", true);
        ASSERT(xmlSocket);

        if (proxyData.usage == WebWatch::SiteItemGroup::ProxyData::useProxy) {
            if (proxyData.address.empty() == false && proxyData.type.empty() == false) {
                XML::CXML *xmlProxy = xmlSocket->FindFirstChild("Proxy", true);
                if (xmlProxy) {
                    xmlProxy->SetParam("Type", proxyData.type.c_str());
                    xmlProxy->SetText(proxyData.address.c_str());
                    removeProxy = false;
                }
            }
        }

        if (removeProxy) {
            XML::CXML *xmlProxy = xmlSocket->FindFirstChild("Proxy");
            if (xmlProxy) {
                xmlSocket->RemoveChild(xmlProxy);
                delete xmlProxy;
            }
        }

        site.SetCheckParameters(params);
    }
}

}
