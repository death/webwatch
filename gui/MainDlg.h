// WebWatch UI - by DEATH, 2004
//
// $Workfile: MainDlg.h $ - Main WebWatch dialog
//
// $Author: Death $
// $Revision: 22 $
// $Date: 25/09/04 3:18 $
// $NoKeywords: $
//
#pragma once

#include "SiteList.h"
#include "SiteGroupsTree.h"
#include "ResizableLayout.h"
#include "ResizableGrip.h"
#include "ResizableMinMax.h"

class CNotificationIcon;

class CMainDlg : public CDialog, private CResizableLayout, private CResizableGrip, private CResizableMinMax
{
public:
    CMainDlg(CWnd *pParent = 0);
    virtual ~CMainDlg();

    enum { IDD = IDD_MAIN };

private: // Overrides and message handlers
    virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual void OnOK();
    virtual void OnCancel();
    virtual BOOL PreTranslateMessage(MSG *msg);

    // CResizableLayout interface
    virtual CWnd *GetResizableWnd();

    afx_msg void OnSiteChanged(NMHDR *header, LRESULT *result);
    afx_msg void OnGroupChanged(NMHDR *header, LRESULT *result);
    afx_msg BOOL OnToolbarTipNeeded(UINT id, NMHDR *header, LRESULT *result);
    afx_msg int OnCreate(LPCREATESTRUCT pCreateStruct);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDestroy();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClose();
    afx_msg void OnTaskbarCreated();
    afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnShowHide();
    afx_msg void OnExit();
    afx_msg void OnSettings();
    afx_msg void OnViewOrderByImportance();
    afx_msg void OnViewUpdateDifference();
    afx_msg BOOL OnEraseBkgnd(CDC *pDC);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO *info);
    afx_msg void OnSiteCheckSpecific();
    afx_msg void OnInitMenu(CMenu *pMenu);
    afx_msg void OnSiteBrowse();
    afx_msg void OnSiteCopyAddress();
    afx_msg void OnSiteMarkAsUpdated();
    afx_msg void OnSiteUnmark();
    afx_msg void OnSiteBeginMU();
    afx_msg void OnSiteEndMU();
    afx_msg void OnSiteModify();
    afx_msg void OnSiteRemove();
    afx_msg void OnSiteMoveToGroup();
    afx_msg void OnSiteAdd();
    afx_msg void OnSiteGroupCheck();
    afx_msg void OnSiteGroupUnmarkAllSites();
    afx_msg void OnSiteGroupAddChildGroup();
    afx_msg void OnSiteGroupImportChildGroup();
    afx_msg void OnSiteGroupModify();
    afx_msg void OnSiteGroupRemove();
    afx_msg void OnSiteGroupMergeWithGroup();
    afx_msg void OnSiteGroupMove();
    afx_msg void OnSiteGroupExport();
    afx_msg void OnCheck();
    afx_msg void OnUnmarkAll();

	DECLARE_MESSAGE_MAP()

private:
    void InitializeSystemMenu();
    void InitializeToolbar();

    void Minimize();
    void Restore();
    void UpdateNotificationIcon(WebWatch::SiteItemGroup *group);
    
    void SetViewUpdateDifferenceCheck();
    void SetOrderByImportanceCheck();

    // Notify menu - the menu shown when right clicking the notification icon
    void ShowNotifyMenu();

    BOOL IsMenuItemEnabled(UINT id) const;
    void UpdateMenu(CMenu & menu);
    void UpdateToolbar();
    void UpdateButtonStates();
    void UpdateActionStates();

    class SiteListHook;
    void DetachFromSiteList();
    void UpdateStatistics();

    bool SaveConfiguration() const;

    class CheckHook;
    
    void OnSitesUpdated();
    void StartBlinking();
    void StopBlinking();

private: // Data
	HICON m_hIcon;
    std::auto_ptr<CSiteList> m_siteList;
    std::auto_ptr<CSiteGroupsTree> m_groupsTree;
    std::auto_ptr<CNotificationIcon> m_notificationIcon;
    HWND m_hwndForeground;
    bool m_minimized;
    std::auto_ptr<CMenu> m_notifyMenu;
    DWORD m_sizeGripState;
    CToolBarCtrl m_toolbar;
    CImageList m_toolbarImageList;
    CImageList m_disabledToolbarImageList;
    std::auto_ptr<SiteListHook> m_siteListHook;
    std::auto_ptr<CheckHook> m_checkHook;
    HACCEL m_siteListAccel;
    HICON m_blinkIcon;
    bool m_blinking;
    bool m_blinkState;

    static UINT s_taskbarCreatedMessage;
};
