// WebWatch UI - by DEATH, 2004
//
// $Workfile: MainDlg.cpp $ - Main WebWatch dialog
//
// $Author: Death $
// $Revision: 3 $
// $Date: 5/13/05 17:16 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "nicon/NotificationIcon.h"

#include "core/Core.h"
#include "core/CoreException.h"

#include "util/Util.h"
#include "util/TimeUtil.h"

#include "WebWatch.h"
#include "MainDlg.h"
#include "AboutDlg.h"
#include "SiteItem.h"
#include "SiteItemGroup.h"
#include "Store.h"
#include "AutoCheckManager.h"
#include "Statistics.h"
#include "SettingsDlg.h"
#include "GeneralSettings.h"
#include "CheckNotification.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

using WebWatch::CheckNotification;
using WebWatch::CheckNotificationChooserImpl;
using WebWatch::CheckNotificationChooser;
using WebWatch::Store;
using WebWatch::AutoCheckManager;
using WebWatch::SiteItemGroup;
using WebWatch::GeneralSettings;

namespace {
    // Timer intervals in milliseconds
    const UINT AutoCheckInterval = 45 * 1000;
    const UINT RefreshInterval = 5 * 60 * 1000;
    const UINT AutoSaveInterval = 5 * 60 * 1000;
    const UINT DefaultBlinkInterval = 1 * 1000;

    // Timer IDs
    const UINT AutoCheckTimerID = 1;
    const UINT RefreshTimerID = 2;
    const UINT AutoSaveTimerID = 3;
    const UINT BlinkTimerID = 4;

    // Notification Icon IDs
    const UINT NotificationIconID = 1;

    // Message IDs
    const UINT WM_NOTIFY_ICON = WM_USER + 0x29A; // Hehe...

    enum WindowMode
    {
        Shown, Minimized
    };

    std::auto_ptr<CMenu> CreateNotifyMenu(WindowMode mode);
    BOOL CALLBACK ShowWindowEnumCallback(HWND hwnd, LPARAM lParam);

    // Toolbar stuff
    const UINT ToolbarButtonWidth = 32;
    const UINT ToolbarButtonHeight = 32;
    const UINT ToolbarButtonDepth = ILC_COLOR24;
    const RGBTRIPLE ToolbarButtonBackground = {0xC0, 0xC0, 0xC0};

    struct ToolbarImageList
    {
        UINT id;
        UINT width;
        UINT height;
        UINT depth;
        RGBTRIPLE background;
        CImageList *imageList;
    };

    void AttachToolbarImages(CToolBarCtrl & toolbar, ToolbarImageList *normal, ToolbarImageList *disabled, ToolbarImageList *hot);
    bool MakeToolbarImageList(ToolbarImageList & lst);
    bool ReplaceBackgroundColor(CBitmap & bmp, const RGBTRIPLE & background);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_ERASEBKGND()
    ON_WM_CREATE()
    ON_WM_GETMINMAXINFO()
    ON_WM_INITMENU()
    ON_MESSAGE(WM_NOTIFY_ICON, OnNotifyIcon)
    ON_COMMAND(ID_FILE_SETTINGS, OnSettings)
    ON_COMMAND(ID_FILE_SHOWHIDE, OnShowHide)
    ON_COMMAND(ID_FILE_EXIT, OnExit)
    ON_COMMAND(ID_VIEW_ORDER_BY_IMPORTANCE, OnViewOrderByImportance)
    ON_COMMAND(ID_VIEW_UPDATE_DIFFERENCE, OnViewUpdateDifference)
    ON_COMMAND(ID_SITE_CHECK_SPECIFIC, OnSiteCheckSpecific)
    ON_COMMAND(ID_SITE_BROWSE, OnSiteBrowse)
    ON_COMMAND(ID_SITE_COPY_ADDRESS, OnSiteCopyAddress)
    ON_COMMAND(ID_SITE_MARK_AS_UPDATED, OnSiteMarkAsUpdated)
    ON_COMMAND(ID_SITE_UNMARK, OnSiteUnmark)
    ON_COMMAND(ID_SITE_BEGIN_MU, OnSiteBeginMU)
    ON_COMMAND(ID_SITE_END_MU, OnSiteEndMU)
    ON_COMMAND(ID_SITE_MODIFY, OnSiteModify)
    ON_COMMAND(ID_SITE_REMOVE, OnSiteRemove)
    ON_COMMAND(ID_SITE_MOVE_TO_GROUP, OnSiteMoveToGroup)
    ON_COMMAND(ID_SITE_ADD, OnSiteAdd)
    ON_COMMAND(ID_SITEGROUP_CHECK, OnSiteGroupCheck)
    ON_COMMAND(ID_SITEGROUP_UNMARK_ALL_SITES, OnSiteGroupUnmarkAllSites)
    ON_COMMAND(ID_SITEGROUP_ADD_CHILD_GROUP, OnSiteGroupAddChildGroup)
    ON_COMMAND(ID_SITEGROUP_IMPORT_CHILD_GROUP, OnSiteGroupImportChildGroup)
    ON_COMMAND(ID_SITEGROUP_MODIFY, OnSiteGroupModify)
    ON_COMMAND(ID_SITEGROUP_REMOVE, OnSiteGroupRemove)
    ON_COMMAND(ID_SITEGROUP_MERGE_WITH_GROUP, OnSiteGroupMergeWithGroup)
    ON_COMMAND(ID_SITEGROUP_MOVE, OnSiteGroupMove)
    ON_COMMAND(ID_SITEGROUP_EXPORT, OnSiteGroupExport)
    ON_COMMAND(IDC_CHECK, OnCheck)
    ON_COMMAND(IDC_UNMARK_ALL, OnUnmarkAll)
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolbarTipNeeded)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_SITE_LIST, OnSiteChanged)
    ON_NOTIFY(TVN_SELCHANGED, IDC_SITE_GROUPS_TREE, OnGroupChanged)
END_MESSAGE_MAP()

UINT CMainDlg::s_taskbarCreatedMessage = RegisterWindowMessage("TaskbarCreated");

// SiteListHook class

class CMainDlg::SiteListHook : public CSiteList::Hook
{
public:
    SiteListHook(CMainDlg & owner)
    : m_owner(owner)
    {
    }

    virtual void OnSiteAddition();
    virtual void OnSiteDeletion();
    virtual void OnSiteMove();
    virtual void OnUpdate();
    virtual void OnDestruction();

private:
    CMainDlg & m_owner;
};

void CMainDlg::SiteListHook::OnSiteAddition()
{
    m_owner.UpdateStatistics();
}

void CMainDlg::SiteListHook::OnSiteDeletion()
{
    m_owner.UpdateStatistics();
}

void CMainDlg::SiteListHook::OnSiteMove()
{
    m_owner.UpdateStatistics();
}

void CMainDlg::SiteListHook::OnUpdate()
{
    m_owner.UpdateStatistics();
}

void CMainDlg::SiteListHook::OnDestruction()
{
    m_owner.DetachFromSiteList();
}

void CMainDlg::DetachFromSiteList()
{
    m_siteList->DetachHook(*m_siteListHook);
}

// CheckHook class

class CMainDlg::CheckHook : public CSiteCheckDlg::Hook
{
public:
    CheckHook(CMainDlg & owner);

    virtual void OnSiteChecked(WebWatch::SiteItem & site, const XML::CXML & results);
    virtual void OnException(WebWatch::SiteItem & site, const Core::Exception & ex);
    virtual void OnAllSitesChecked(CWnd & wnd);

    void PreAutoCheck();
    void PostAutoCheck();

private:
    CMainDlg & m_owner;
    int m_nupdated;
    bool m_auto;
};

CMainDlg::CheckHook::CheckHook(CMainDlg & owner)
: m_owner(owner)
, m_nupdated(0)
, m_auto(false)
{
}

void CMainDlg::CheckHook::OnSiteChecked(WebWatch::SiteItem & site, const XML::CXML & results)
{
    bool oldAvail = site.AreCheckResultsAvailable();

    if (results.GetChildCount() > 0) {
        if (oldAvail == false || site.IsUpdated(results) == true) {
            site.SetState(WebWatch::SiteItem::updated);
            site.SetCheckResults(results);
            m_nupdated++;
        } else {
            if (site.GetState() != WebWatch::SiteItem::updated)
                site.SetState(WebWatch::SiteItem::none);
        }
    } else {
        // No information in 'results', but we know there was no error
        if (oldAvail == false) {
            site.SetState(WebWatch::SiteItem::none);
        }
    }
}

void CMainDlg::CheckHook::OnException(WebWatch::SiteItem & site, const Core::Exception & ex)
{
    site.SetState(WebWatch::SiteItem::erroneous);
    site.SetVerboseStatus(ex.what());
}

namespace 
{
    class ActivateNotification : public std::unary_function<CheckNotification *, void>
    {
    public:
        explicit ActivateNotification(CWnd & wnd)
        : m_wnd(wnd)
        {
        }

        void operator() (CheckNotification *notification)
        {
            if (notification && notification->IsEnabled() == true)
                notification->Activate(m_wnd);
        }

    private:
        CWnd & m_wnd;
    };
}

void CMainDlg::CheckHook::OnAllSitesChecked(CWnd & wnd)
{
    if (m_nupdated) {
        m_owner.OnSitesUpdated();
        m_nupdated = 0;
    }

    if (m_auto == false) {
        const CheckNotificationChooserImpl::Notifications & notifications = CheckNotificationChooser::Instance().GetAllNotifications();
        std::for_each(notifications.begin(), notifications.end(), ActivateNotification(wnd));
    }
}

void CMainDlg::CheckHook::PreAutoCheck()
{
    m_auto = true;
}

void CMainDlg::CheckHook::PostAutoCheck()
{
    m_auto = false;
}

// CMainDlg class

CMainDlg::CMainDlg(CWnd *pParent)
: CDialog(CMainDlg::IDD, pParent)
, m_hIcon(AfxGetApp()->LoadIcon(IDI_WEBWATCH))
, m_hwndForeground(0)
, m_minimized(false)
, m_sizeGripState(1)
, m_siteListAccel(LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_SITE_LIST_ACCEL)))
, m_blinkIcon(AfxGetApp()->LoadIcon(IDI_WEBWATCH_BURN))
, m_blinking(false)
{
}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SITE_LIST, *m_siteList);
    DDX_Control(pDX, IDC_SITE_GROUPS_TREE, *m_groupsTree);
}

int CMainDlg::OnCreate(LPCREATESTRUCT pCreateStruct)
{
    if (CDialog::OnCreate(pCreateStruct) == -1)
        return -1;

    if (!CreateSizeGrip())
        return -1;

    SetMinTrackSize(CSize(600, 400));

    m_siteList.reset(new CSiteList);
    m_groupsTree.reset(new CSiteGroupsTree);

    return 0;
}

BOOL CMainDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

	InitializeSystemMenu();
    InitializeToolbar();

    // Set an icon for this dialog
    SetIcon(m_hIcon, TRUE);     // Big icon
	SetIcon(m_hIcon, FALSE);    // Small icon

    // Load settings to store
    try {
        Store::Instance().Load("WebWatch.xml");
    }
    catch (const std::exception & ex) {
        MessageBox(ex.what(), "Error", MB_OK | MB_ICONERROR);
    }
    
    // Set up check hook
    m_checkHook.reset(new CheckHook(*this));

    // Initialize site list
    try {
        m_siteList->Initialize(*m_checkHook);
    }
    catch (const std::exception & ex) {
        MessageBox(ex.what(), "Error", MB_OK | MB_ICONERROR);
        Store::Instance().SetDefaultSiteListSettings();
        m_siteList->Initialize(*m_checkHook);
    }

    m_siteListHook.reset(new SiteListHook(*this));
    m_siteList->AttachHook(*m_siteListHook);

    // Initialize groups tree
    m_groupsTree->Initialize(*m_checkHook, *m_siteList);

    // Add anchors for controls
    AddAnchor(IDC_SITE_LIST, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_SITE_GROUPS_TREE, TOP_LEFT, BOTTOM_LEFT);
    AddAnchor(IDC_CHECK, BOTTOM_RIGHT);
    AddAnchor(IDC_UNMARK_ALL, BOTTOM_RIGHT);
    AddAnchor(IDC_STATISTICS_TEXT, BOTTOM_LEFT, BOTTOM_RIGHT);

    // Restore main window placement
    WINDOWPLACEMENT wp;
    Store::Instance().GetWindowPlacement(wp);
    SetWindowPlacement(&wp);

    // Initialize notification icon
    m_notificationIcon.reset(new CNotificationIcon(m_hWnd, NotificationIconID));
    m_notificationIcon->SetIcon(m_hIcon);
    m_notificationIcon->SetTip("WebWatch");
    m_notificationIcon->SetCallbackMessage(WM_NOTIFY_ICON);
    m_notificationIcon->Add();

    // Set up auto-check timer and call it for the first time
    SetTimer(AutoCheckTimerID, AutoCheckInterval, 0);
    OnTimer(AutoCheckTimerID);

    // Set up refresh timer
    SetTimer(RefreshTimerID, RefreshInterval, 0);

    // Set up autosave timer
    SetTimer(AutoSaveTimerID, AutoSaveInterval, 0);

    UpdateActionStates();

	return TRUE;
}

void CMainDlg::InitializeSystemMenu()
{
    // Add "About..." menu item to system menu.
	if (CMenu *sysMenu = GetSystemMenu(FALSE)) {
		CString str;
		str.LoadString(IDS_ABOUT);
		if (!str.IsEmpty()) {
			sysMenu->AppendMenu(MF_SEPARATOR);

            ASSERT((ID_ABOUT & 0xFFF0) == ID_ABOUT);
	        ASSERT(ID_ABOUT < 0xF000);
			sysMenu->AppendMenu(MF_STRING, ID_ABOUT, str);
		}
	}
}

void CMainDlg::InitializeToolbar()
{
    static TBBUTTON buttons[] = {
        {0 , ID_FILE_SETTINGS           , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {9 , ID_VIEW_UPDATE_DIFFERENCE  , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {0 , 0                          , TBSTATE_ENABLED, TBSTYLE_SEP      , 0, 0, 0, 0},
        {1 , ID_SITE_ADD                , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {2 , ID_SITE_REMOVE             , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {4 , ID_SITE_MODIFY             , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {6 , ID_SITE_BROWSE             , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {0 , 0                          , TBSTATE_ENABLED, TBSTYLE_SEP      , 0, 0, 0, 0},
        {3 , ID_SITE_MARK_AS_UPDATED    , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {5 , ID_SITE_UNMARK             , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {0 , 0                          , TBSTATE_ENABLED, TBSTYLE_SEP      , 0, 0, 0, 0},
        {8 , ID_SITE_CHECK_SPECIFIC     , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {7 , ID_SITEGROUP_CHECK         , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0},
        {0 , 0                          , TBSTATE_ENABLED, TBSTYLE_SEP      , 0, 0, 0, 0},
        {10, ID_FILE_EXIT               , TBSTATE_ENABLED, TBSTYLE_BUTTON   , 0, 0, 0, 0}
    };

    static const UINT buttonCount = sizeof(buttons) / sizeof(TBBUTTON);

    CRect rect;
    m_toolbar.Create(WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | CCS_TOP | WS_CLIPSIBLINGS, rect, this, AFX_IDW_TOOLBAR);
    
    ToolbarImageList normal;
    normal.id = IDB_TOOLBAR;
    normal.width = ToolbarButtonWidth;
    normal.height = ToolbarButtonHeight;
    normal.depth = ToolbarButtonDepth;
    normal.background = ToolbarButtonBackground;
    normal.imageList = &m_toolbarImageList;

    ToolbarImageList disabled;
    disabled.id = IDB_TOOLBAR_DISABLED;
    disabled.width = ToolbarButtonWidth;
    disabled.height = ToolbarButtonHeight;
    disabled.depth = ToolbarButtonDepth;
    disabled.background = ToolbarButtonBackground;
    disabled.imageList = &m_disabledToolbarImageList;

    AttachToolbarImages(m_toolbar, &normal, &disabled, 0);

    m_toolbar.SetButtonSize(CSize(ToolbarButtonWidth, ToolbarButtonHeight));
    m_toolbar.AddButtons(buttonCount, buttons);
}

void CMainDlg::UpdateActionStates()
{
    CMenu *menu = GetMenu();
    ASSERT(menu);
    UpdateMenu(*menu);
    UpdateToolbar();
    UpdateButtonStates();
}

void CMainDlg::UpdateButtonStates()
{
    GetDlgItem(IDC_CHECK)->EnableWindow(IsMenuItemEnabled(ID_SITEGROUP_CHECK));
    GetDlgItem(IDC_UNMARK_ALL)->EnableWindow(IsMenuItemEnabled(ID_SITEGROUP_UNMARK_ALL_SITES));
}

void CMainDlg::UpdateToolbar()
{
    m_toolbar.EnableButton(ID_FILE_SETTINGS, IsMenuItemEnabled(ID_FILE_SETTINGS));
    m_toolbar.EnableButton(ID_VIEW_UPDATE_DIFFERENCE, IsMenuItemEnabled(ID_VIEW_UPDATE_DIFFERENCE));
    m_toolbar.EnableButton(ID_SITE_ADD, IsMenuItemEnabled(ID_SITE_ADD));
    m_toolbar.EnableButton(ID_SITE_REMOVE, IsMenuItemEnabled(ID_SITE_REMOVE));
    m_toolbar.EnableButton(ID_SITE_MODIFY, IsMenuItemEnabled(ID_SITE_MODIFY));
    m_toolbar.EnableButton(ID_SITE_BROWSE, IsMenuItemEnabled(ID_SITE_BROWSE));
    m_toolbar.EnableButton(ID_SITE_MARK_AS_UPDATED, IsMenuItemEnabled(ID_SITE_MARK_AS_UPDATED));
    m_toolbar.EnableButton(ID_SITE_UNMARK, IsMenuItemEnabled(ID_SITE_UNMARK));
    m_toolbar.EnableButton(ID_SITE_CHECK_SPECIFIC, IsMenuItemEnabled(ID_SITE_CHECK_SPECIFIC));
    m_toolbar.EnableButton(ID_SITEGROUP_CHECK, IsMenuItemEnabled(ID_SITEGROUP_CHECK));
    m_toolbar.EnableButton(ID_FILE_EXIT, IsMenuItemEnabled(ID_FILE_EXIT));
}

BOOL CMainDlg::IsMenuItemEnabled(UINT id) const
{
    CMenu *menu = GetMenu();
    ASSERT(menu);
    return (menu->GetMenuState(id, MF_BYCOMMAND) & MF_GRAYED) ? FALSE : TRUE;
}

BOOL CMainDlg::OnToolbarTipNeeded(UINT id, NMHDR *header, LRESULT *result)
{
    TOOLTIPTEXT *text = reinterpret_cast<TOOLTIPTEXT *>(header);
    UINT commandID = static_cast<UINT>(text->hdr.idFrom);
    
    // Special case
    if (commandID == ID_SITE_CHECK_SPECIFIC) {
        lstrcpy(text->szText, "Check specific site...");
    } else {
        if (CMenu *menu = GetMenu()) {
            CString str;
            menu->GetMenuString(commandID, str, MF_BYCOMMAND);
            lstrcpy(text->szText, str);
        } else {
            text->szText[0] = '\0';
        }
    }

    *result = 0;
    return FALSE;
}

void CMainDlg::OnSiteChanged(NMHDR *header, LRESULT *result)
{
    UpdateActionStates();
    *result = 0;
}

void CMainDlg::OnGroupChanged(NMHDR *header, LRESULT *result)
{
    UpdateActionStates();
    UpdateStatistics();
    *result = 0;
}

void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == ID_ABOUT) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CMainDlg::OnPaint() 
{
	if (IsIconic()) {
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialog::OnPaint();
	}
}

HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainDlg::OnDestroy()
{
    CDialog::OnDestroy();

    RemoveAllAnchors();

    m_groupsTree.reset();
    m_siteList.reset();

    if (m_notificationIcon.get()) {
        m_notificationIcon->Remove();
        m_notificationIcon.reset();
    }
}

void CMainDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (nType == SIZE_MINIMIZED) {
        // Window gets minimized
        Minimize();
        return;
    }

	if (nType == SIZE_MAXIMIZED)
		HideSizeGrip(&m_sizeGripState);
	else
		ShowSizeGrip(&m_sizeGripState);

    UpdateSizeGrip();
    ArrangeLayout();

    m_siteList->UpdateColumns();
    
    if (::IsWindow(m_toolbar.GetSafeHwnd())) {
        CRect rectToolbar;
        m_toolbar.GetWindowRect(&rectToolbar);
        m_toolbar.MoveWindow(0, 0, cx, rectToolbar.Height(), FALSE);
    }

    InvalidateRect(0, TRUE);
}

void CMainDlg::OnClose()
{
    if (SaveConfiguration() == false) {
        // Don't destroy window
        return;
    }

    KillTimer(AutoSaveTimerID);
    KillTimer(RefreshTimerID);
    KillTimer(AutoCheckTimerID);

    if (m_blinking)
        StopBlinking();

    EndDialog(IDOK);
}

LRESULT CMainDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == s_taskbarCreatedMessage) {
        OnTaskbarCreated();
        return 0;
    }

    return CDialog::DefWindowProc(message, wParam, lParam);
}

void CMainDlg::OnTaskbarCreated()
{
    if (m_notificationIcon.get())
        m_notificationIcon->Add();
}

LRESULT CMainDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
    if (wParam != NotificationIconID)
        return 0;

    switch (lParam) {
        case WM_LBUTTONDOWN:
            if (m_blinking)
                StopBlinking();
            break;
        case WM_LBUTTONDBLCLK:
            // If window is hidden, show it
            // If window is visible, set it to be the foreground window
            Restore();
            break;
        case WM_RBUTTONDOWN:
            ShowNotifyMenu();
            break;
        default:
            break;
    }

    return 0;
}

void CMainDlg::Minimize()
{
    // Grab foreground window handle
    HWND hwndForeground;
    if (CWnd *wnd = GetForegroundWindow())
        hwndForeground = wnd->m_hWnd;
    else
        hwndForeground = m_hWnd;

    // Hide all current thread's windows
    EnumThreadWindows(GetCurrentThreadId(), ShowWindowEnumCallback, FALSE);

    if (m_minimized == true)
        return;

    m_hwndForeground = hwndForeground;

    m_minimized = true;
}

void CMainDlg::Restore()
{
    if (m_blinking)
        StopBlinking();

    if (m_minimized == false) {
        // Not minimized, just set as foreground window (and make sure the
        // window is visible and restored)
        if (IsWindowVisible() == FALSE)
            ShowWindow(SW_SHOW);
        if (IsIconic() == TRUE)
            ShowWindow(SW_RESTORE);
        SetForegroundWindow();
        return;
    }

    // Show all current thread's windows
    EnumThreadWindows(GetCurrentThreadId(), ShowWindowEnumCallback, TRUE);

    // Set previously set foreground window, if it still is a valid window
    if (::IsWindow(m_hwndForeground))
        ::SetForegroundWindow(m_hwndForeground);
    else
        SetForegroundWindow();

    m_minimized = false;
}

void CMainDlg::OnOK()
{
}

void CMainDlg::OnCancel()
{
}

BOOL CMainDlg::PreTranslateMessage(MSG *msg)
{
    CWnd *focusWindow = GetFocus();
    if (focusWindow && m_siteList.get()) {
        if (focusWindow->GetSafeHwnd() == m_siteList->GetSafeHwnd()) {
            if (TranslateAccelerator(m_siteList->GetSafeHwnd(), m_siteListAccel, msg))
                return TRUE;
        }
    }

    return CDialog::PreTranslateMessage(msg);
}

void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent) {
        case AutoCheckTimerID:
            {
            m_checkHook->PreAutoCheck();
            AutoCheckManager manager(*m_checkHook, Store::Instance().GetRootGroup());
            if (manager.AutoCheck(this))
                m_siteList->UpdateView();
            SiteItemGroup *group = manager.GetNearestAutoCheckGroup();
            UpdateNotificationIcon(group);
            m_checkHook->PostAutoCheck();
            }
            break;
        case RefreshTimerID:
            // So far the reason for refreshing views every once in a while
            // is to support MU-related display correctly.
            InvalidateRect(0);
            break;
        case AutoSaveTimerID:
            {
            GeneralSettings generalSettings;
            Store::Instance().GetGeneralSettings(generalSettings);
            if (generalSettings.autoSave) {
                SaveConfiguration();
            }
            }
            break;
        case BlinkTimerID:
            ASSERT(m_blinking == true);
            if (m_notificationIcon.get()) {
                m_notificationIcon->SetIcon(m_blinkState ? m_hIcon : m_blinkIcon);
                m_notificationIcon->Update(NIF_ICON);
            }
            m_blinkState = !m_blinkState;
            break;
        default:
            CDialog::OnTimer(nIDEvent);
            break;
    }
}

void CMainDlg::UpdateNotificationIcon(WebWatch::SiteItemGroup *group)
{
    std::string tip = "WebWatch";
    if (group) {
        std::time_t t = group->GetAutoCheckTime();
        struct tm *local = localtime(&t);
        if (local) {
            char buf[128];
            strftime(buf, 128, " - Next auto-check at %H:%M ('", local);
            tip += buf;
            tip += group->GetName();
            tip += "')";
        }
    }
    if (tip != m_notificationIcon->GetTip()) {
        m_notificationIcon->SetTip(tip.c_str());
        m_notificationIcon->Update(NIF_TIP);
    }
}

void CMainDlg::ShowNotifyMenu()
{
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow();

    m_notifyMenu = CreateNotifyMenu(IsWindowVisible() ? Shown : Minimized);
    if (m_notifyMenu.get() == 0)
        return;

    m_notifyMenu->TrackPopupMenu(0, pt.x, pt.y, this);
}

CWnd *CMainDlg::GetResizableWnd()
{
    return this;
}

BOOL CMainDlg::OnEraseBkgnd(CDC *pDC)
{
    EraseBackground(pDC);
    return TRUE;
}

void CMainDlg::OnGetMinMaxInfo(MINMAXINFO *info)
{
    MinMaxInfo(info);
}

void CMainDlg::OnInitMenu(CMenu *pMenu)
{
    CDialog::OnInitMenu(pMenu);
    if (pMenu)
        UpdateMenu(*pMenu);
}

void CMainDlg::UpdateMenu(CMenu & menu)
{
    SetViewUpdateDifferenceCheck();
    SetOrderByImportanceCheck();
    m_siteList->UpdateMenu(menu);
    m_groupsTree->UpdateMenu(menu, m_groupsTree->GetSelectedItem());
}

bool CMainDlg::SaveConfiguration() const
{
    m_siteList->StoreSettings();
    m_groupsTree->StoreSettings();

    WINDOWPLACEMENT wp;
    if (GetWindowPlacement(&wp)) {
        Store::Instance().SetWindowPlacement(wp);
    }

    try {
        Store::Instance().Save("WebWatch.xml");
    }
    catch(std::exception & ex) {
        // HACK: MessageBox() isn't declared as a const member function
        ::MessageBox(m_hWnd, ex.what(), "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

void CMainDlg::OnSitesUpdated()
{
    if (IsWindowVisible() == FALSE && m_blinking == false)
        StartBlinking();
}

void CMainDlg::StartBlinking()
{
    ASSERT(m_blinking == false);
    m_blinking = true;
    m_blinkState = false;
    UINT blinkInterval = GetCaretBlinkTime();
    if (blinkInterval == 0)
        blinkInterval = DefaultBlinkInterval;
    SetTimer(BlinkTimerID, blinkInterval, 0);
}

void CMainDlg::StopBlinking()
{
    ASSERT(m_blinking == true);
    KillTimer(BlinkTimerID);
    if (m_notificationIcon.get()) {
        m_notificationIcon->SetIcon(m_hIcon);
        m_notificationIcon->Update(NIF_ICON);
    }
    m_blinking = false;
}

// Statistics

void CMainDlg::UpdateStatistics()
{
    const WebWatch::SiteItemGroup & group = m_siteList->GetSiteItemGroup();
    
    std::ostringstream os;
    os << WebWatch::GetOverallSiteCount(group) << " Sites total";
    
    int updatedCount = WebWatch::Statistics::GetSiteCountByState(group, WebWatch::SiteItem::updated, WebWatch::Statistics::recursiveCount);
    int erroneousCount = WebWatch::Statistics::GetSiteCountByState(group, WebWatch::SiteItem::erroneous, WebWatch::Statistics::recursiveCount);
    int muCount = WebWatch::Statistics::GetMUSiteCount(group, WebWatch::Statistics::recursiveCount);

    if (updatedCount > 0 || erroneousCount > 0 || muCount > 0) {
        os << " (";
        
        bool first = true;

        if (updatedCount > 0) {
            if (!first)
                os << ", ";
            else
                first = false;
            os << updatedCount << " updated";
        }

        if (erroneousCount > 0) {
            if (!first)
                os << ", ";
            else
                first = false;
            os << erroneousCount << " erroneous";
        }

        if (muCount > 0) {
            if (!first)
                os << ", ";
            else
                first = false;
            os << muCount << " mu";
        }
        
        os << ")";
    }

    const std::string & stats = os.str();
    SetDlgItemText(IDC_STATISTICS_TEXT, stats.c_str());
}

// Button actions

void CMainDlg::OnCheck()
{
    // The following call is needed in order to reset site groups tree's
    // action group
    UpdateActionStates();
    OnSiteGroupCheck();
    m_siteList->SetFocus();
}

void CMainDlg::OnUnmarkAll()
{
    // The following call is needed in order to reset site groups tree's
    // action group
    UpdateActionStates();
    OnSiteGroupUnmarkAllSites();
    m_siteList->SetFocus();
}

// File actions

void CMainDlg::OnShowHide()
{
    if (IsWindowVisible()) {
        Minimize();
    } else {
        Restore();
    }
}

void CMainDlg::OnExit()
{
    PostMessage(WM_CLOSE);
}

void CMainDlg::OnSettings()
{
    ASSERT(m_siteList.get());
    CSettingsDlg dlg(*m_siteList, *m_groupsTree, "WebWatch Settings", this);
    INT_PTR res = dlg.DoModal();
    if (res == IDOK)
        dlg.StoreSettings();
}

// View actions

void CMainDlg::OnViewOrderByImportance()
{
    bool isEnabled = m_siteList->IsOrderedByImportance();

#ifndef NDEBUG
    CMenu *menu = GetMenu();
    ASSERT(menu);
    UINT state = menu->GetMenuState(ID_VIEW_ORDER_BY_IMPORTANCE, MF_BYCOMMAND);
    ASSERT(isEnabled ? state & MF_CHECKED : !(state & MF_CHECKED));
#endif

    m_siteList->EnableOrderByImportance(!isEnabled);

    SetOrderByImportanceCheck();
}

void CMainDlg::SetOrderByImportanceCheck()
{
    CMenu *menu = GetMenu();
    ASSERT(menu);

    bool isEnabled = m_siteList->IsOrderedByImportance();
    menu->CheckMenuItem(ID_VIEW_ORDER_BY_IMPORTANCE, MF_BYCOMMAND | (isEnabled ? MF_CHECKED : 0));
}

void CMainDlg::OnViewUpdateDifference()
{
    bool isEnabled = m_siteList->IsViewUpdateDifferenceEnabled();

#ifndef NDEBUG
    CMenu *menu = GetMenu();
    ASSERT(menu);
    UINT state = menu->GetMenuState(ID_VIEW_UPDATE_DIFFERENCE, MF_BYCOMMAND);
    ASSERT(isEnabled ? state & MF_CHECKED : !(state & MF_CHECKED));
#endif

    m_siteList->EnableViewUpdateDifference(!isEnabled);
    
    SetViewUpdateDifferenceCheck();
}

void CMainDlg::SetViewUpdateDifferenceCheck()
{
    CMenu *menu = GetMenu();
    ASSERT(menu);

    bool isEnabled = m_siteList->IsViewUpdateDifferenceEnabled();
    menu->CheckMenuItem(ID_VIEW_UPDATE_DIFFERENCE, MF_BYCOMMAND | (isEnabled ? MF_CHECKED : 0));
}

// Site actions

void CMainDlg::OnSiteCheckSpecific()
{
    m_siteList->OnCheckSpecific();
}

void CMainDlg::OnSiteBrowse()
{
    m_siteList->OnBrowse();
}

void CMainDlg::OnSiteCopyAddress()
{
    m_siteList->OnCopySiteAddress();
}

void CMainDlg::OnSiteMarkAsUpdated()
{
    m_siteList->OnMarkAsUpdated();
}

void CMainDlg::OnSiteUnmark()
{
    m_siteList->OnUnmark();
}

void CMainDlg::OnSiteBeginMU()
{
    m_siteList->OnBeginMUSession();
}

void CMainDlg::OnSiteEndMU()
{
    m_siteList->OnEndMUSession();
}

void CMainDlg::OnSiteModify()
{
    m_siteList->OnModifySite();
}

void CMainDlg::OnSiteRemove()
{
    m_siteList->OnRemove();
}

void CMainDlg::OnSiteMoveToGroup()
{
    m_siteList->OnMoveToGroup();
}

void CMainDlg::OnSiteAdd()
{
    m_siteList->OnAddSite();
}

// Site group actions

void CMainDlg::OnSiteGroupCheck()
{
    m_groupsTree->OnSiteGroupCheck();
}

void CMainDlg::OnSiteGroupUnmarkAllSites()
{
    m_groupsTree->OnSiteGroupUnmarkAllSites();
}

void CMainDlg::OnSiteGroupAddChildGroup()
{
    m_groupsTree->OnSiteGroupAddChildGroup();
}

void CMainDlg::OnSiteGroupImportChildGroup()
{
    m_groupsTree->OnSiteGroupImportChildGroup();
}

void CMainDlg::OnSiteGroupModify()
{
    m_groupsTree->OnSiteGroupModify();
}

void CMainDlg::OnSiteGroupRemove()
{
    m_groupsTree->OnSiteGroupRemove();
}

void CMainDlg::OnSiteGroupMergeWithGroup()
{
    m_groupsTree->OnSiteGroupMergeWithGroup();
}

void CMainDlg::OnSiteGroupMove()
{
    m_groupsTree->OnSiteGroupMove();
}

void CMainDlg::OnSiteGroupExport()
{
    m_groupsTree->OnSiteGroupExport();
}

namespace {

std::auto_ptr<CMenu> CreateNotifyMenu(WindowMode mode)
{
    std::auto_ptr<CMenu> menu(new CMenu);
    
    if (menu->CreatePopupMenu() == FALSE)
        return std::auto_ptr<CMenu>();

    if (menu->AppendMenu(MF_ENABLED | MF_DEFAULT, ID_FILE_SHOWHIDE, mode == Shown ? "Hide" : "Show") == FALSE)
        return std::auto_ptr<CMenu>();

    if (menu->AppendMenu(MF_ENABLED, ID_FILE_EXIT, "Exit") == FALSE)
        return std::auto_ptr<CMenu>();

    return menu;
}

BOOL CALLBACK ShowWindowEnumCallback(HWND hwnd, LPARAM lParam)
{
    bool show = lParam == TRUE;
    BOOL visibleTest = show ? FALSE : TRUE;
    int showCmd = show ? SW_SHOW : SW_HIDE;

    if (::IsWindowVisible(hwnd) == visibleTest)
        ::ShowWindow(hwnd, showCmd);

    if (show) {
        if (::IsIconic(hwnd))
            ::ShowWindow(hwnd, SW_RESTORE);
    }

    return TRUE; // Continue enumeration
}

void AttachToolbarImages(CToolBarCtrl & toolbar, ToolbarImageList *normal, ToolbarImageList *disabled, ToolbarImageList *hot)
{
    if (normal && MakeToolbarImageList(*normal))
        toolbar.SetImageList(normal->imageList);

    if (disabled && MakeToolbarImageList(*disabled))
        toolbar.SetDisabledImageList(disabled->imageList);

    if (hot && MakeToolbarImageList(*hot))
        toolbar.SetHotImageList(hot->imageList);
}

bool MakeToolbarImageList(ToolbarImageList & lst)
{
    HINSTANCE hModule = AfxFindResourceHandle(MAKEINTRESOURCE(lst.id), RT_BITMAP);
    if (hModule == 0)
        return false;

    HANDLE hImage = LoadImage(hModule, MAKEINTRESOURCE(lst.id), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
    if (hImage == 0)
        return false;

    CBitmap bmp;
    if (bmp.Attach(hImage) == FALSE) {
        DeleteObject(hImage);
        return false;
    }

    ReplaceBackgroundColor(bmp, lst.background);

    if (lst.imageList->Create(lst.width, lst.height, lst.depth, 0, 4) == FALSE)
        return false;

    if (lst.imageList->Add(&bmp, RGB(0, 0, 0)) == -1)
        return false;

    return true;
}

bool ReplaceBackgroundColor(CBitmap & bmp, const RGBTRIPLE & background)
{
    BITMAP info;
    if (bmp.GetBitmap(&info) == 0)
        return false;

    if (info.bmBitsPixel != 24 || info.bmWidthBytes != (info.bmWidth * 3))
        return false;

    COLORREF buttonColor = GetSysColor(COLOR_BTNFACE);
    RGBTRIPLE newBackground = {
        GetBValue(buttonColor),
        GetGValue(buttonColor),
        GetRValue(buttonColor)
    };

    RGBTRIPLE *pixels = reinterpret_cast<RGBTRIPLE *>(info.bmBits);
    ASSERT(pixels);
    UINT count = info.bmWidth * info.bmHeight;
    ASSERT(count);
    RGBTRIPLE *pixel = pixels;
    RGBTRIPLE *end = pixels + count;
    while (pixel != end) {
        if (memcmp(pixel, &background, sizeof(RGBTRIPLE)) == 0)
            *pixel = newBackground;
        pixel++;
    }

    return true;
}

}