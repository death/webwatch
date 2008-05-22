// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupsTree.h $ - Site groups tree control
//
// $Author: Death $
// $Revision: 4 $
// $Date: 8/08/05 0:27 $
// $NoKeywords: $
//
#pragma once

#include "SiteCheckDlg.h"
#include "Colors.h"

namespace WebWatch
{
    class SiteItemGroup;
}

class CSiteList;

class CSiteGroupsTree : public CTreeCtrl
{
    typedef std::map<const WebWatch::SiteItemGroup *, HTREEITEM> Group2Item;

    DECLARE_MESSAGE_MAP()

    class GroupAppender;
    class GroupUpdater;
    class SiteListHook;
    class GroupRemover;

public:

    struct Colors
    {
        Colors();
        Colors(const XML::CXML & xml);

        XML::CXML *Write(XML::CXML & xmlParent) const;

        void Init();

        static const char *GetXMLName();

        WebWatch::Color regular;
        WebWatch::Color updated;
        WebWatch::Color mu;
        
        // Drag & Drop colors
        WebWatch::Color droppable;
        WebWatch::Color nondroppable;
        WebWatch::Color dragMask;
    };

public:
    CSiteGroupsTree();
    virtual ~CSiteGroupsTree();

    void Initialize(CSiteCheckDlg::Hook & checkHook, CSiteList & siteList);
    void StoreSettings() const;
    void UpdateMenu(CMenu & menu, HTREEITEM item);

    Colors GetColors() const;
    void SetColors(const Colors & colors);

    HTREEITEM InsertGroupItem(WebWatch::SiteItemGroup & group, 
                              HTREEITEM parent = 0);

    static void GetDefaultSettings(XML::CXML & xml);
    static const char *GetXMLName();

public:
    afx_msg BOOL OnSelectionChanged(NMHDR *header, LRESULT *result);
    afx_msg void OnCustomDraw(NMHDR *header, LRESULT *result);
    afx_msg void OnRightClick(NMHDR *header, LRESULT *result);
    afx_msg void OnSiteGroupCheck();
    afx_msg void OnSiteGroupUnmarkAllSites();
    afx_msg void OnSiteGroupAddChildGroup();
    afx_msg void OnSiteGroupImportChildGroup();
    afx_msg void OnSiteGroupModify();
    afx_msg void OnSiteGroupRemove();
    afx_msg void OnSiteGroupMergeWithGroup();
    afx_msg void OnSiteGroupMove();
    afx_msg void OnSiteGroupExport();
    afx_msg void OnExpandChildItems();
    afx_msg void OnCollapseChildItems();

    afx_msg void OnBeginDrag(NMHDR *header, LRESULT *result);
    afx_msg void OnMouseMove(UINT flags, CPoint point);
    afx_msg void OnLButtonUp(UINT flags, CPoint point);

    afx_msg void OnBeginLabelEdit(NMHDR *header, LRESULT *result);
    afx_msg void OnEndLabelEdit(NMHDR *header, LRESULT *result);

    afx_msg void OnGetDispInfo(NMHDR *header, LRESULT *result);

    virtual BOOL PreTranslateMessage(MSG *msg);

private:
    COLORREF GetGroupColor(const WebWatch::SiteItemGroup & group, COLORREF defColor);

    WebWatch::SiteItemGroup & BuildTree();
    WebWatch::SiteItemGroup & BuildTree(WebWatch::SiteItemGroup & group);
    void InsertChildGroups(HTREEITEM parentItem, WebWatch::SiteItemGroup & parentGroup);
    void UpdateTree();

    void SelectGroup(WebWatch::SiteItemGroup & group, bool forceRefresh = false);
    void RefreshSiteList(WebWatch::SiteItemGroup & group);
    
    HTREEITEM GetItemFromGroup(const WebWatch::SiteItemGroup & group);
    WebWatch::SiteItemGroup & GetGroupFromItem(HTREEITEM item);
    const WebWatch::SiteItemGroup & GetGroupFromItem(HTREEITEM item) const;

    void InitializeMenus();

    void DetachFromSiteList();

    bool IsDroppableItem(HTREEITEM item) const;
    CImageList *CreateDragImageEx(HTREEITEM item, COLORREF color);

    void DeepExpand(HTREEITEM item, UINT code);

private:
    Group2Item m_group2item;
    CSiteList *m_siteList;
    CMenu m_menu;
    WebWatch::SiteItemGroup *m_actionGroup;
    std::auto_ptr<Colors> m_colors;
    std::auto_ptr<SiteListHook> m_siteListHook;
    
    HTREEITEM m_draggedItem;
    HTREEITEM m_droppedItem;
    std::auto_ptr<CImageList> m_dragImageList;
    bool m_isDragging;

    CSiteCheckDlg::Hook *m_checkHook;
};