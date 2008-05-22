// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupsTree.cpp $ - Site groups tree control
//
// $Author: Death $
// $Revision: 4 $
// $Date: 8/08/05 0:27 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "WebWatch.h"
#include "SiteGroupsTree.h"
#include "SiteItemGroup.h"
#include "Store.h"
#include "SiteList.h"
#include "SiteGroupPropertiesDlg.h"
#include "Colors.h"
#include "ChooseGroupDlg.h"
#include "Statistics.h"

namespace {
    const char * const DefaultGroupName = "New Group";
    const CPoint DragHotspot(-15, 0);

    // Import/Export settings
    const char * const DefaultFilter = "XML files (*.xml)|*.xml|All files (*.*)|*.*||";
    const char * const DefaultExtension = "xml";

    // Get a group's name to display in the tree, probably be different than
    // just calling SiteItemGroup::GetName()
    std::string GetGroupName(const WebWatch::SiteItemGroup & group);

    // A choose-group-dialog hook for Move action
    class MoveChooseGroupHook : public CChooseGroupDlg::Hook
    {
    public:
        MoveChooseGroupHook(WebWatch::SiteItemGroup & toMove)
        : m_toMove(toMove)
        {
        }

        virtual ~MoveChooseGroupHook();
        virtual bool CanChoose(const WebWatch::SiteItemGroup *group) const;

    private:
        WebWatch::SiteItemGroup & m_toMove;
    };

    // A choose-group-dialog hook for Merge action
    class MergeChooseGroupHook : public CChooseGroupDlg::Hook
    {
    public:
        MergeChooseGroupHook(WebWatch::SiteItemGroup & toMerge)
        : m_toMerge(toMerge)
        {
        }

        virtual ~MergeChooseGroupHook();
        virtual bool CanChoose(const WebWatch::SiteItemGroup *group) const;

    private:
        WebWatch::SiteItemGroup & m_toMerge;
    };
}

// GroupAppender class

class CSiteGroupsTree::GroupAppender : public std::unary_function<WebWatch::SiteItemGroup *, void>
{
public:
    GroupAppender(CSiteGroupsTree & owner, HTREEITEM parentItem)
    : m_owner(owner)
    , m_parentItem(parentItem)
    {
    }

    void operator() (WebWatch::SiteItemGroup *group)
    {
        // NOTE: We need SetRedraw() here because of the text callback
        m_owner.SetRedraw(FALSE);

        ASSERT(group);
        HTREEITEM item = m_owner.InsertGroupItem(*group, m_parentItem);
        m_owner.InsertChildGroups(item, *group);

        m_owner.SetRedraw(TRUE);
    }

private:
    CSiteGroupsTree & m_owner;
    HTREEITEM m_parentItem;
};

// GroupUpdater class

class CSiteGroupsTree::GroupUpdater : public std::unary_function<Group2Item::value_type, void>
{
public:
    GroupUpdater(CSiteGroupsTree & owner)
    : m_owner(owner)
    {
    }

    void operator() (Group2Item::value_type entry)
    {
        // Nothing to update because we use LPSTR_TEXTCALLBACK for the
        // name shown.
    }

private:
    CSiteGroupsTree & m_owner;
};

// GroupRemover class

class CSiteGroupsTree::GroupRemover : public std::unary_function<const WebWatch::SiteItemGroup *, void>
{
public:
    GroupRemover(CSiteGroupsTree & owner)
    : m_owner(owner)
    {
    }

    void operator() (const WebWatch::SiteItemGroup *group)
    {
        Group2Item::iterator it = m_owner.m_group2item.find(group);
        if (it != m_owner.m_group2item.end()) {
            HTREEITEM item = it->second;
            m_owner.DeleteItem(item);
            m_owner.m_group2item.erase(it);
        }
    }

private:
    CSiteGroupsTree & m_owner;
};

// SiteListHook class

class CSiteGroupsTree::SiteListHook : public CSiteList::Hook
{
public:
    SiteListHook(CSiteGroupsTree & owner)
    : m_owner(owner)
    {
    }

    virtual void OnSiteAddition();
    virtual void OnSiteDeletion();
    virtual void OnSiteMove();
    virtual void OnUpdate();
    virtual void OnDestruction();

private:
    CSiteGroupsTree & m_owner;
};

void CSiteGroupsTree::SiteListHook::OnSiteAddition()
{
    m_owner.UpdateTree();
}

void CSiteGroupsTree::SiteListHook::OnSiteDeletion()
{
    m_owner.UpdateTree();
}

void CSiteGroupsTree::SiteListHook::OnSiteMove()
{
    m_owner.UpdateTree();
}

void CSiteGroupsTree::SiteListHook::OnUpdate()
{
    // This is called when the site list just got updated,
    // we redraw the site groups tree (update colors)
    m_owner.InvalidateRect(0);
}

void CSiteGroupsTree::SiteListHook::OnDestruction()
{
    m_owner.DetachFromSiteList();
}

// Message map

BEGIN_MESSAGE_MAP(CSiteGroupsTree, CTreeCtrl)
    ON_WM_CREATE()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelectionChanged)
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
    ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
    ON_NOTIFY_REFLECT(TVN_GETDISPINFO, OnGetDispInfo)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
    ON_COMMAND(ID_SITEGROUP_CHECK, OnSiteGroupCheck)
    ON_COMMAND(ID_SITEGROUP_UNMARK_ALL_SITES, OnSiteGroupUnmarkAllSites)
    ON_COMMAND(ID_SITEGROUP_ADD_CHILD_GROUP, OnSiteGroupAddChildGroup)
    ON_COMMAND(ID_SITEGROUP_IMPORT_CHILD_GROUP, OnSiteGroupImportChildGroup)
    ON_COMMAND(ID_SITEGROUP_MODIFY, OnSiteGroupModify)
    ON_COMMAND(ID_SITEGROUP_REMOVE, OnSiteGroupRemove)
    ON_COMMAND(ID_SITEGROUP_MERGE_WITH_GROUP, OnSiteGroupMergeWithGroup)
    ON_COMMAND(ID_SITEGROUP_MOVE, OnSiteGroupMove)
    ON_COMMAND(ID_SITEGROUP_EXPORT, OnSiteGroupExport)
    ON_COMMAND(ID_TREE_EXPAND_CHILD_ITEMS, OnExpandChildItems)
    ON_COMMAND(ID_TREE_COLLAPSE_CHILD_ITEMS, OnCollapseChildItems)
END_MESSAGE_MAP()

CSiteGroupsTree::CSiteGroupsTree()
: m_siteList(0)
, m_draggedItem(0)
, m_droppedItem(0)
, m_isDragging(false)
, m_checkHook(0)
{
}

CSiteGroupsTree::~CSiteGroupsTree()
{
    DetachFromSiteList();
}

void CSiteGroupsTree::Initialize(CSiteCheckDlg::Hook & checkHook, CSiteList & siteList)
{
    // Initialize settings

    XML::CXML params;
    WebWatch::Store::Instance().GetSiteGroupsTreeSettings(params);

    ASSERT(strcmp(params.GetTagName(), GetXMLName()) == 0);

    // Color initialization
    XML::CXML *colors = params.FindFirstChild(Colors::GetXMLName());
    if (colors)
        m_colors.reset(new Colors(*colors));
    else
        m_colors.reset(new Colors);

    // NOTE: SetTextColor() not really needed
    SetTextColor(m_colors->regular.color);

    // Rest of initialization...

    m_siteList = &siteList;
    m_siteListHook.reset(new SiteListHook(*this));
    m_siteList->AttachHook(*m_siteListHook);

    m_checkHook = &checkHook;

    WebWatch::SiteItemGroup & defaultGroup = BuildTree();
    SelectGroup(defaultGroup);

    InitializeMenus();
}

WebWatch::SiteItemGroup & CSiteGroupsTree::BuildTree()
{
    WebWatch::SiteItemGroup & rootGroup = WebWatch::Store::Instance().GetRootGroup();
    return BuildTree(rootGroup);
}

WebWatch::SiteItemGroup & CSiteGroupsTree::BuildTree(WebWatch::SiteItemGroup & group)
{
    SetRedraw(FALSE);

    DeleteAllItems();
    m_group2item.clear();

    HTREEITEM root = InsertGroupItem(group);

    WebWatch::SiteItemGroup *defaultGroup = 0;

    WebWatch::SiteItemGroup::GroupIterator it = group.GetGroupsBegin();
    WebWatch::SiteItemGroup::GroupIterator end = group.GetGroupsEnd();
    GroupAppender appender(*this, root);

    while (it != end) {
        WebWatch::SiteItemGroup *subgroup = *it;
        appender(subgroup);

        if (defaultGroup == 0 && subgroup->GetName() == "Default")
            defaultGroup = subgroup;

        ++it;
    }

    SetRedraw();

    if (defaultGroup == 0)
        defaultGroup = &group;

    return *defaultGroup;
}

void CSiteGroupsTree::InsertChildGroups(HTREEITEM parentItem, WebWatch::SiteItemGroup & parentGroup)
{
    std::for_each(parentGroup.GetGroupsBegin(), parentGroup.GetGroupsEnd(), GroupAppender(*this, parentItem));
}

void CSiteGroupsTree::SelectGroup(WebWatch::SiteItemGroup & group, bool forceRefresh)
{
    HTREEITEM previous = GetSelectedItem();
    HTREEITEM item = GetItemFromGroup(group);
    SelectItem(item);
    if ((item == previous) && forceRefresh)
        RefreshSiteList(group);
}

void CSiteGroupsTree::RefreshSiteList(WebWatch::SiteItemGroup & group)
{
    ASSERT(m_siteList);

    m_siteList->SetSiteItemGroup(group);

    bool enableAddSite = WebWatch::Store::Instance().IsRootGroup(group) == false;
    m_siteList->EnableAddSite(enableAddSite);
}

HTREEITEM CSiteGroupsTree::GetItemFromGroup(const WebWatch::SiteItemGroup & group)
{
    Group2Item::const_iterator it = m_group2item.find(&group);
    if (it == m_group2item.end())
        throw std::runtime_error("Couldn't find tree item for specified group");
    
    return it->second;
}

WebWatch::SiteItemGroup & CSiteGroupsTree::GetGroupFromItem(HTREEITEM item)
{
    if (item == 0)
        throw std::runtime_error("Null tree item passed to CSiteGroupsTree::GetGroupFromItem()");

    WebWatch::SiteItemGroup *group = reinterpret_cast<WebWatch::SiteItemGroup *>(GetItemData(item));
    if (group == 0)
        throw std::runtime_error("Bad item data in CSiteGroupsTree::GetGroupFromItem()");

    return *group;
}

const WebWatch::SiteItemGroup & CSiteGroupsTree::GetGroupFromItem(HTREEITEM item) const
{
    if (item == 0)
        throw std::runtime_error("Null tree item passed to CSiteGroupsTree::GetGroupFromItem() const");

    const WebWatch::SiteItemGroup *group = reinterpret_cast<const WebWatch::SiteItemGroup *>(GetItemData(item));
    if (group == 0)
        throw std::runtime_error("Bad item data in CSiteGroupsTree::GetGroupFromItem() const");

    return *group;
}

void CSiteGroupsTree::UpdateTree()
{
    SetRedraw(FALSE);
    std::for_each(m_group2item.begin(), m_group2item.end(), GroupUpdater(*this));
    SetRedraw();
}

void CSiteGroupsTree::InitializeMenus()
{
    m_menu.CreatePopupMenu();
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_CHECK, "Check...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_UNMARK_ALL_SITES, "Unmark all sites");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_ADD_CHILD_GROUP, "Add child group...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_IMPORT_CHILD_GROUP, "Import child group...");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_MODIFY, "Modify...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_REMOVE, "Remove");
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_MERGE_WITH_GROUP, "Merge with group...");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_MOVE, "Move...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITEGROUP_EXPORT, "Export...");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_TREE_EXPAND_CHILD_ITEMS, "Expand child items");
    m_menu.AppendMenu(MF_ENABLED, ID_TREE_COLLAPSE_CHILD_ITEMS, "Collapse child items");
}

BOOL CSiteGroupsTree::OnSelectionChanged(NMHDR *header, LRESULT *result)
{
	LPNMTREEVIEW tvh = reinterpret_cast<LPNMTREEVIEW>(header);
    
	// Set the site list according to the selected site
    WebWatch::SiteItemGroup & group = GetGroupFromItem(tvh->itemNew.hItem);
    RefreshSiteList(group);
    
    *result = 0;
    return FALSE;
}

void CSiteGroupsTree::OnRightClick(NMHDR *, LRESULT *result)
{
    CPoint pt;
    GetCursorPos(&pt);
    CPoint ptClient = pt;
    ScreenToClient(&ptClient);
    
    HTREEITEM item = HitTest(ptClient);
    if (item == 0)
        item = GetSelectedItem();

    UpdateMenu(m_menu, item);
    m_menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);

    *result = 1;
}

void CSiteGroupsTree::UpdateMenu(CMenu & menu, HTREEITEM item)
{
    ASSERT(item);

    WebWatch::SiteItemGroup & group = GetGroupFromItem(item);
    m_actionGroup = &group;

    bool isRootGroup = WebWatch::Store::Instance().IsRootGroup(group);
    int siteCount = WebWatch::GetOverallSiteCount(group);

    menu.EnableMenuItem(ID_SITEGROUP_CHECK, (siteCount > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITEGROUP_UNMARK_ALL_SITES, (siteCount > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITEGROUP_ADD_CHILD_GROUP, MF_ENABLED);
    menu.EnableMenuItem(ID_SITEGROUP_IMPORT_CHILD_GROUP, MF_ENABLED);
    menu.EnableMenuItem(ID_SITEGROUP_MODIFY, MF_ENABLED);
    menu.EnableMenuItem(ID_SITEGROUP_REMOVE, (isRootGroup == false) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITEGROUP_MERGE_WITH_GROUP, (isRootGroup == false) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITEGROUP_MOVE, (isRootGroup == false) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITEGROUP_EXPORT, MF_ENABLED);
}

void CSiteGroupsTree::OnCustomDraw(NMHDR *header, LRESULT *result)
{
    NMTVCUSTOMDRAW *info = reinterpret_cast<NMTVCUSTOMDRAW *>(header);

    switch (info->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
            // Control prepaint, tell Windows we want a notification for each item
            *result = CDRF_NOTIFYITEMDRAW;
            break;
        case CDDS_ITEMPREPAINT:
            {
            const WebWatch::SiteItemGroup *group = reinterpret_cast<const WebWatch::SiteItemGroup *>(info->nmcd.lItemlParam);
            ASSERT(group);
            info->clrText = GetGroupColor(*group, info->clrText);
            }
            // Fallthrough
        default:
            *result = CDRF_DODEFAULT;
            break;
    }
}

void CSiteGroupsTree::DetachFromSiteList()
{
    if (m_siteList) {
        m_siteList->DetachHook(*m_siteListHook);
        m_siteList = 0;
    }
}

// Site group operations

void CSiteGroupsTree::OnSiteGroupCheck()
{
    CWnd *parent = GetParent();
    ASSERT(parent);
    
    ASSERT(m_actionGroup);
    ASSERT(m_checkHook);
    m_actionGroup->Check(*m_checkHook, parent);

    ASSERT(m_siteList);
    m_siteList->UpdateView();
}

void CSiteGroupsTree::OnSiteGroupUnmarkAllSites()
{
    ASSERT(m_actionGroup);
    m_actionGroup->UnmarkAllSites();

    ASSERT(m_siteList);
    m_siteList->UpdateView();
}

void CSiteGroupsTree::OnSiteGroupAddChildGroup()
{
    ASSERT(m_actionGroup);

    WebWatch::SiteItemGroup group(DefaultGroupName);
    CSiteGroupPropertiesDlg dlg(group, false, "Add Group");

    if (dlg.DoModal() != IDOK)
        return;

    WebWatch::SiteItemGroup *newGroup = new WebWatch::SiteItemGroup(group.GetName().c_str());
    newGroup->SetProperties(group.GetProperties());

    m_actionGroup->AddGroup(*newGroup);

    HTREEITEM parentItem = GetItemFromGroup(*m_actionGroup);
    ASSERT(parentItem);
    GroupAppender(*this, parentItem)(newGroup);

    UpdateTree();
}

void CSiteGroupsTree::OnSiteGroupImportChildGroup()
{
    ASSERT(m_actionGroup);

    CWnd *parent = GetParent();
    ASSERT(parent);

    CFileDialog dlg(TRUE, DefaultExtension, 0, OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST, DefaultFilter, parent);
    char currentDir[_MAX_PATH];
    dlg.m_ofn.lpstrInitialDir = _getcwd(currentDir, _MAX_PATH);
    if (dlg.DoModal() != IDOK)
        return;

    CString filename = dlg.GetPathName();

    try {
        WebWatch::SiteItemGroup & importedGroup = WebWatch::Store::Instance().ImportGroup(*m_actionGroup, filename);
        GroupAppender appender(*this, GetItemFromGroup(*m_actionGroup));
        appender(&importedGroup);
    }
    catch(const WebWatch::StoreError & ex) {
        parent->MessageBox(ex.what(), "Failure", MB_OK | MB_ICONERROR);
        return;
    }

    UpdateTree();
    
    WebWatch::SiteItemGroup & selectedGroup = GetGroupFromItem(GetSelectedItem());
    RefreshSiteList(selectedGroup);

    std::string message("Group(s) in file '");
    message += dlg.GetFileName();
    message += "' imported successfully under group '";
    message += m_actionGroup->GetName();
    message += '\'';
    parent->MessageBox(message.c_str(), "Success", MB_OK | MB_ICONINFORMATION);
}

void CSiteGroupsTree::OnSiteGroupModify()
{
    ASSERT(m_actionGroup);

    WebWatch::SiteItemGroup groupCopy(m_actionGroup->GetProperties());
    CSiteGroupPropertiesDlg dlg(groupCopy, WebWatch::Store::Instance().IsRootGroup(*m_actionGroup), "Modify Group");

    if (dlg.DoModal() != IDOK)
        return;

    m_actionGroup->SetProperties(groupCopy.GetProperties());

    //
    // NOTE: Known-bug:
    // If the user has changed auto-check period, and the group was the nearest
    // group to check, he will see out-of-date status in WebWatch notification
    // for maximum 45 seconds. This happens because we don't notify the main
    // dialog that we actually updated the period.
    //

    UpdateTree();
}

void CSiteGroupsTree::OnSiteGroupRemove()
{
    // Ask user whether to delete the selected group or not

    ASSERT(m_actionGroup);
    ASSERT(WebWatch::Store::Instance().IsRootGroup(*m_actionGroup) == false);

    CWnd *parent = GetParent();
    ASSERT(parent);

    std::string query("Are you sure you want to remove the group '");
    query += m_actionGroup->GetName();
    query += "' ?";

    UINT ret = parent->MessageBox(query.c_str(), "Remove group?", MB_YESNO | MB_ICONQUESTION);

    if (ret == IDNO)
        return;

    // Remove group
    
    // Gather all groups to remove from tree

    typedef std::vector<WebWatch::SiteItemGroup *> Groups;
    Groups groupsToRemove(1, m_actionGroup);
    groupsToRemove.reserve(m_actionGroup->GetGroupCount() + groupsToRemove.size());
    WebWatch::GatherChildGroups(*m_actionGroup, std::back_inserter(groupsToRemove));

    // Make sure site list is in stable state

    ASSERT(m_siteList);
    WebWatch::SiteItemGroup dummy("Dummy");
    m_siteList->SetSiteItemGroup(dummy);

    SetRedraw(FALSE);

    // Delete actual group and its sub-groups

    WebWatch::SiteItemGroup *parentGroup = m_actionGroup->GetParentGroup();
    ASSERT(parentGroup);

    parentGroup->DeleteGroup(*m_actionGroup);

    // Remove groups/items from tree and update tree

    std::for_each(groupsToRemove.begin(), groupsToRemove.end(), GroupRemover(*this));

    UpdateTree();

    SetRedraw();

    SelectGroup(*parentGroup, true);
}

void CSiteGroupsTree::OnSiteGroupMergeWithGroup()
{
    ASSERT(m_actionGroup);
    MergeChooseGroupHook hook(*m_actionGroup);
    CChooseGroupDlg dlg(WebWatch::Store::Instance().GetRootGroup(), &hook, GetParent());

    if (dlg.DoModal() != IDOK)
        return;

    WebWatch::SiteItemGroup *group = dlg.GetChosenGroup();
    if (group == 0)
        return;

    group->MergeGroup(*m_actionGroup);

    BuildTree();
    
    // The action group no longer exists, it was merged onto 'group'
    SelectGroup(*group, true); 
}

void CSiteGroupsTree::OnSiteGroupMove()
{
    ASSERT(m_actionGroup);
    MoveChooseGroupHook hook(*m_actionGroup);
    CChooseGroupDlg dlg(WebWatch::Store::Instance().GetRootGroup(), &hook, GetParent());

    if (dlg.DoModal() != IDOK)
        return;

    WebWatch::SiteItemGroup *group = dlg.GetChosenGroup();
    if (group == 0)
        return;

    group->MoveGroup(*m_actionGroup);
    
    BuildTree();
    SelectGroup(*m_actionGroup, true);
}

void CSiteGroupsTree::OnSiteGroupExport()
{
    ASSERT(m_actionGroup);

    CWnd *parent = GetParent();
    ASSERT(parent);

    std::string defaultFileName(m_actionGroup->GetName());
    defaultFileName += '.';
    defaultFileName += DefaultExtension;

    CFileDialog dlg(FALSE, DefaultExtension, defaultFileName.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST, DefaultFilter, parent);
    char currentDir[_MAX_PATH];
    dlg.m_ofn.lpstrInitialDir = _getcwd(currentDir, _MAX_PATH);
    if (dlg.DoModal() != IDOK)
        return;

    CString filename = dlg.GetPathName();

    try {
        WebWatch::Store::Instance().ExportGroup(*m_actionGroup, filename);
    }
    catch(const WebWatch::StoreError & ex) {
        parent->MessageBox(ex.what(), "Failure", MB_OK | MB_ICONERROR);
        return;
    }

    std::string message("Group '");
    message += m_actionGroup->GetName();
    message += "' exported successfully to '";
    message += dlg.GetFileName();
    message += '\'';
    parent->MessageBox(message.c_str(), "Success", MB_OK | MB_ICONINFORMATION);
}

void CSiteGroupsTree::OnExpandChildItems()
{
    ASSERT(m_actionGroup);
    DeepExpand(GetItemFromGroup(*m_actionGroup), TVE_EXPAND);
}

void CSiteGroupsTree::OnCollapseChildItems()
{
    ASSERT(m_actionGroup);
    DeepExpand(GetItemFromGroup(*m_actionGroup), TVE_COLLAPSE);
}

// Colors

COLORREF CSiteGroupsTree::GetGroupColor(const WebWatch::SiteItemGroup & group, COLORREF defColor)
{
    HTREEITEM item = GetItemFromGroup(group);
    HTREEITEM selected = GetSelectedItem();
    HTREEITEM dropTarget = GetDropHilightItem();
    
    if (item == selected || item == dropTarget)
        return defColor;

    COLORREF color = m_colors->regular.color;

    int muCount = WebWatch::Statistics::GetMUSiteCount(group, WebWatch::Statistics::recursiveCount);
    if (muCount > 0) {
        color = m_colors->mu.color;
    } else {
        int updatedCount = WebWatch::Statistics::GetSiteCountByState(group, WebWatch::SiteItem::updated, WebWatch::Statistics::deepCount);
        if (updatedCount > 0)
            color = m_colors->updated.color;
    }
    
    return color;
}

CSiteGroupsTree::Colors CSiteGroupsTree::GetColors() const
{
    ASSERT(m_colors.get());
    return *m_colors;
}

void CSiteGroupsTree::SetColors(const Colors & colors)
{
    m_colors.reset(new Colors(colors));
    Invalidate();
}

// Settings

const char *CSiteGroupsTree::GetXMLName()
{
    return "SiteGroupsTree";
}

void CSiteGroupsTree::GetDefaultSettings(XML::CXML & xml)
{
    xml.ResetContents();
    xml.SetTagName(GetXMLName());
}

void CSiteGroupsTree::StoreSettings() const
{
    XML::CXML params(GetXMLName());

    if (m_colors.get())
        m_colors->Write(params);

    WebWatch::Store::Instance().SetSiteGroupsTreeSettings(params);
}

// Drag & Drop support

void CSiteGroupsTree::OnBeginDrag(NMHDR *header, LRESULT *result)
{
    NMTREEVIEW *info = reinterpret_cast<NMTREEVIEW *>(header);

    ASSERT(m_isDragging == false);

    m_draggedItem = info->itemNew.hItem;
    if (m_draggedItem) {
        m_droppedItem = 0;
        ASSERT(m_dragImageList.get() == 0);
        m_dragImageList.reset(CreateDragImageEx(m_draggedItem, IsDroppableItem(m_draggedItem) ? m_colors->droppable.color : m_colors->nondroppable.color));
        if (m_dragImageList.get()) {
            m_isDragging = true;
            m_dragImageList->BeginDrag(0, DragHotspot);
            CPoint pt(info->ptDrag);
            ClientToScreen(&pt);
            m_dragImageList->DragEnter(this, pt);
            SetCapture();
        }
    }

    *result = 0;
}

void CSiteGroupsTree::OnMouseMove(UINT flags, CPoint point)
{
    if (m_isDragging) {
        CImageList::DragMove(point);
        UINT hitFlags;
        HTREEITEM item = HitTest(point, &hitFlags);
        if (item != m_droppedItem) {
            CImageList::DragShowNolock(FALSE);
            SelectDropTarget(item);
            m_droppedItem = item;
            CImageList::DragShowNolock(TRUE);
        }
    }

    CTreeCtrl::OnMouseMove(flags, point);
}

void CSiteGroupsTree::OnLButtonUp(UINT flags, CPoint point)
{
    CTreeCtrl::OnLButtonUp(flags, point);

    if (m_isDragging == false)
        return;

    // User is dropping an item

    m_isDragging = false;
    CImageList::DragLeave(this);
    CImageList::EndDrag();
    ReleaseCapture();
    m_dragImageList.reset(0);
    SelectDropTarget(0);

    // Determine whether dropping the item has an effect
    
    // Here we allow a group to be dropped on itself without any effect and without any
    // error message
    if ((m_draggedItem == m_droppedItem) || (m_droppedItem == 0))
        return;

    if (IsDroppableItem(m_draggedItem) == false)
        return;

    WebWatch::SiteItemGroup & draggedGroup = GetGroupFromItem(m_draggedItem);
    WebWatch::SiteItemGroup & droppedGroup = GetGroupFromItem(m_droppedItem);

    if (WebWatch::IsChildOf(draggedGroup, droppedGroup) == true) {
        GetParent()->MessageBox("Cannot move a group into one of its child groups", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Do the actual moving
    
    droppedGroup.MoveGroup(draggedGroup);
    
    BuildTree();
    SelectGroup(draggedGroup, true);
}

bool CSiteGroupsTree::IsDroppableItem(HTREEITEM item) const
{
    if (item == 0)
        return false;

    const WebWatch::SiteItemGroup & group = GetGroupFromItem(item);
    
    return WebWatch::Store::Instance().IsRootGroup(group) == false;
}

CImageList *CSiteGroupsTree::CreateDragImageEx(HTREEITEM item, COLORREF color)
{
    if (GetImageList(TVSIL_NORMAL))
        return CreateDragImage(item);

    // Create bitmap
    
    std::auto_ptr<CClientDC> dc;
    try {
        dc.reset(new CClientDC(this));
    }
    catch (CResourceException *ex) {
        ex->Delete();
        return 0;
    }

    CDC memdc;
    if (memdc.CreateCompatibleDC(dc.get()) == FALSE)
        return 0;

    CRect rect;
    GetItemRect(item, rect, TRUE);
    rect.top = rect.left = 0;

    CBitmap bitmap;
    if (bitmap.CreateCompatibleBitmap(dc.get(), rect.Width(), rect.Height()) == FALSE)
        return 0;

    CBitmap *oldBitmap = memdc.SelectObject(&bitmap);
    CFont *oldFont = memdc.SelectObject(GetFont());

    COLORREF dragMask = m_colors->dragMask.color;

    memdc.FillSolidRect(&rect, dragMask);
    memdc.SetTextColor(color);
    memdc.TextOut(rect.left, rect.top, GetItemText(item));

    memdc.SelectObject(oldFont);
    memdc.SelectObject(oldBitmap);

    // Create image list

    std::auto_ptr<CImageList> imageList(new CImageList);
    if (imageList->Create(rect.Width(), rect.Height(), ILC_COLOR24 | ILC_MASK, 0, 1) == FALSE)
        return 0;

    if (imageList->Add(&bitmap, dragMask) == -1)
        return 0;

    return imageList.release();
}

void CSiteGroupsTree::OnGetDispInfo(NMHDR *header, LRESULT *result)
{
    NMTVDISPINFO *info = reinterpret_cast<NMTVDISPINFO *>(header);

    ASSERT((info->item.mask & TVIF_SELECTEDIMAGE) == 0);
    ASSERT((info->item.mask & TVIF_CHILDREN) == 0);

    if (info->item.mask & TVIF_TEXT) {
        WebWatch::SiteItemGroup & group = GetGroupFromItem(info->item.hItem);
        const std::string & name = GetGroupName(group);
        strncpy(info->item.pszText, name.c_str(), info->item.cchTextMax - 2);
        info->item.pszText[info->item.cchTextMax - 1] = '\0';
    }
}

// Label edit

void CSiteGroupsTree::OnBeginLabelEdit(NMHDR *header, LRESULT *result)
{
    NMTVDISPINFO *info = reinterpret_cast<NMTVDISPINFO *>(header);

    CEdit *edit = GetEditControl();
    if (edit) {
        WebWatch::SiteItemGroup & group = GetGroupFromItem(info->item.hItem);
        const std::string & name = group.GetName();
        edit->SetWindowText(name.c_str());
        *result = 0;
    } else {
        // Cancel editing
        *result = TRUE;
    }
}

void CSiteGroupsTree::OnEndLabelEdit(NMHDR *header, LRESULT *result)
{
    NMTVDISPINFO *info = reinterpret_cast<NMTVDISPINFO *>(header);

    if (info->item.pszText) {
        // Continue with new name
        WebWatch::SiteItemGroup & group = GetGroupFromItem(info->item.hItem);
        group.SetName(info->item.pszText);
        Group2Item::const_iterator it = m_group2item.find(&group);
        if (it != m_group2item.end()) {
            GroupUpdater updater(*this);
            updater(*it);
        }
        *result = TRUE;
    } else {
        // Ignore
        *result = 0;
    }
}

BOOL CSiteGroupsTree::PreTranslateMessage(MSG *msg)
{
    if (msg->message == WM_KEYDOWN) {
		// When an item is being edited make sure the edit control
		// receives certain important key strokes
		if (GetEditControl() && (msg->wParam == VK_RETURN || msg->wParam == VK_DELETE || msg->wParam == VK_ESCAPE || GetKeyState(VK_CONTROL))) {
			::TranslateMessage(msg);
			::DispatchMessage(msg);
			return TRUE; // Do NOT process further
		}
    }

    return CTreeCtrl::PreTranslateMessage(msg);
}

void CSiteGroupsTree::DeepExpand(HTREEITEM item, UINT code)
{
    std::stack<HTREEITEM> items;
    items.push(item);
    while (items.empty() == false) {
        HTREEITEM item = items.top();
        items.pop();
        Expand(item, code);
        if (ItemHasChildren(item)) {
            HTREEITEM child = GetChildItem(item);
            while (child) {
                items.push(child);
                child = GetNextItem(child, TVGN_NEXT);
            }
        }
    }
}


HTREEITEM CSiteGroupsTree::InsertGroupItem(WebWatch::SiteItemGroup & group,
                                           HTREEITEM parent)
{
    HTREEITEM item = InsertItem(LPSTR_TEXTCALLBACK, parent);
    if (item) {
        SetItemData(item, reinterpret_cast<DWORD_PTR>(&group));
        m_group2item.insert(std::make_pair(&group, item));
    }
   return item;
}

namespace {

std::string GetGroupName(const WebWatch::SiteItemGroup & group)
{
    std::ostringstream os;
    os << group.GetName() << " (" << WebWatch::GetOverallSiteCount(group) << ")";
    return os.str();
}

MoveChooseGroupHook::~MoveChooseGroupHook()
{
}

bool MoveChooseGroupHook::CanChoose(const WebWatch::SiteItemGroup *group) const
{
    if (group == 0)
        return false;

    if (group == &m_toMove)
        return false;

    return IsChildOf(m_toMove, *group) == false;
}

MergeChooseGroupHook::~MergeChooseGroupHook()
{
}

bool MergeChooseGroupHook::CanChoose(const WebWatch::SiteItemGroup *group) const
{
    if (group == 0)
        return false;

    if (WebWatch::Store::Instance().IsRootGroup(*group) == true)
        return false;

    return IsChildOf(m_toMerge, *group) == false;
}

}