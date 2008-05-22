// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteList.cpp $ - Site list control
//
// $Author: Death $
// $Revision: 4 $
// $Date: 8/06/05 19:32 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/TimeUtil.h"
#include "util/Common.h"
#include "util/EmptyString.h"

#include "WebWatch.h"
#include "SiteList.h"
#include "SiteItemGroup.h"
#include "SiteItem.h"
#include "Store.h"
#include "SiteOperationManager.h"
#include "MU.h"
#include "ChooseGroupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

using WebWatch::Store;
using WebWatch::ColorPair;
using WebWatch::SiteItem;
using WebWatch::SiteItemGroup;

namespace SiteOperationManager = WebWatch::SiteOperationManager;

namespace 
{
    const int NoSortColumn = -1;
    const int ColumnCount = 5;
    const bool DefaultViewUpdateDifference = false;
    const bool DefaultOrderByImportance = true;

    typedef boost::function<void (CSiteList::Hook *)> HookFn;

    SiteItemGroup *ChooseGroup(CWnd *parentWnd);

    class SiteSelector
    {
        CSiteList & m_owner;
        bool m_sel;
    public:
        explicit SiteSelector(bool sel, CSiteList & owner) : m_owner(owner), m_sel(sel) {}

        void operator() (int item)
        {
            m_owner.SetItemState(item, m_sel ? LVIS_SELECTED : 0, LVIS_SELECTED);
        }
    };
}

// Site List control message map

BEGIN_MESSAGE_MAP(CSiteList, CListCtrl)
    ON_WM_INITMENU()
    ON_NOTIFY(HDN_ENDTRACKA, 0, OnHeaderEndTrack)
    ON_NOTIFY(HDN_ENDTRACKW, 0, OnHeaderEndTrack)
    ON_NOTIFY(HDN_BEGINTRACKA, 0, OnHeaderBeginTrack)
    ON_NOTIFY(HDN_BEGINTRACKW, 0, OnHeaderBeginTrack)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
    ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
    ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
    ON_NOTIFY_REFLECT(LVN_ITEMACTIVATE, OnItemActivate)
    ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
    ON_COMMAND(ID_SITE_ACTIVATE, OnActivateSite)
    ON_COMMAND(ID_SITE_ADD, OnAddSite)
    ON_COMMAND(ID_SITE_CHECK_SPECIFIC, OnCheckSpecific)
    ON_COMMAND(ID_SITE_BROWSE, OnBrowse)
    ON_COMMAND(ID_SITE_COPY_ADDRESS, OnCopySiteAddress)
    ON_COMMAND(ID_SITE_MARK_AS_UPDATED, OnMarkAsUpdated)
    ON_COMMAND(ID_SITE_UNMARK, OnUnmark)
    ON_COMMAND(ID_SITE_BEGIN_MU, OnBeginMUSession)
    ON_COMMAND(ID_SITE_END_MU, OnEndMUSession)
    ON_COMMAND(ID_SITE_MODIFY, OnModifySite)
    ON_COMMAND(ID_SITE_REMOVE, OnRemove)
    ON_COMMAND(ID_SITE_MOVE_TO_GROUP, OnMoveToGroup)
    ON_COMMAND(ID_SITE_SET_CBD, OnSetCBD)
    ON_COMMAND(ID_SITE_UNSET_CBD, OnUnsetCBD)
    ON_COMMAND(ID_SITE_INVERT_CBD, OnInvertCBD)
    ON_COMMAND(ID_SITE_SELECT_ALL, OnSelectAll)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CSiteList, CListCtrl);

CSiteList::CSiteList()
: m_checkWidth(0)
, m_scrollerWidth(0)
, m_group(0)
, m_sortColumn(NoSortColumn)
, m_addSiteEnabled(true)
, m_itemActivationAction(GetDefaultItemActivationAction())
, m_viewUpdateDifference(DefaultViewUpdateDifference)
, m_orderByImportance(DefaultOrderByImportance)
, m_checkHook(0)
{
}

CSiteList::~CSiteList()
{
    while (m_hooks.empty() == false)
        m_hooks.front()->OnDestruction(); // Handlers should detach
}

void CSiteList::DoDataExchange(CDataExchange *pDX)
{
    CListCtrl::DoDataExchange(pDX);
}

void CSiteList::Initialize(CSiteCheckDlg::Hook & checkHook)
{
    if (::IsWindow(m_hWnd) == FALSE)
        return;

    m_checkHook = &checkHook;

    // Visibility customization
    SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

    // Load settings
    XML::CXML params;
    Store::Instance().GetSiteListSettings(params);

    ASSERT(strcmp(params.GetTagName(), GetXMLName()) == 0);

    LoadColumnSettings(params);
    LoadColorSettings(params);
    LoadSortSettings(params);
    LoadItemActivationSettings(params);
    LoadViewSettings(params);

    // Menu initialization
    InitializeMenus();
}

void CSiteList::StoreSettings() const
{
    XML::CXML params(GetXMLName());

    StoreColumnSettings(params);
    StoreColorSettings(params);
    StoreSortSettings(params);
    StoreItemActivationSettings(params);
    StoreViewSettings(params);

    Store::Instance().SetSiteListSettings(params);
}

// Columns

void CSiteList::StoreColumnSettings(XML::CXML & params) const
{
    for_each(m_columns.begin(), m_columns.end(), Util::StreamInserter<XML::CXML, ColumnEntry>(params));
}

void CSiteList::LoadColumnSettings(const XML::CXML & params)
{
    RemoveAllColumns();
    AddFirstColumn();
    AddColumns(params);
    ColumnsSanityCheck();
}

void CSiteList::RemoveAllColumns()
{
    for (int i = GetHeaderCtrl()->GetItemCount(); i > 0; i--)
        DeleteColumn(0);

    m_columns.clear();
}

void CSiteList::AddFirstColumn()
{
    // First column isn't user defined, it's a column used only to hold a check
    // box. It's size is also constant.
    m_scrollerWidth = ::GetSystemMetrics(SM_CXVSCROLL);
    m_checkWidth = -1; // Automatic sizing
    InsertColumn(0, "C");
}

void CSiteList::AddColumns(const XML::CXML & params)
{
    int col = GetHeaderCtrl()->GetItemCount();
    const XML::CXML *column = params.FindFirstChild("Column");
    while (column) {
        const char *name = column->GetParamText("Name");
        double width = column->GetParamDouble("Width", 1.0);

        m_columns.push_back(ColumnEntry(name, width));
        InsertColumn(col++, name);

        column = params.FindNextChild("Column");
    }
}

void CSiteList::ColumnsSanityCheck()
{
    if (GetHeaderCtrl()->GetItemCount() != ColumnCount)
        throw std::runtime_error("Bad list of columns for SiteList");
}

void CSiteList::UpdateColumns()
{
    if (::IsWindow(m_hWnd) == FALSE)
        return;

    SetColumnWidth(0, m_checkWidth);
    if (m_checkWidth == -1)
        m_checkWidth = GetColumnWidth(0);

    int totalWidth = GetTotalWidth();

    ColumnEntries::const_iterator i = m_columns.begin();
    ColumnEntries::const_iterator end = m_columns.end();

    int col = 1;

    while (i != end) {
        const ColumnEntry & entry = *i;

        int cx = static_cast<int>(entry.width * totalWidth / 100.0);
        SetColumnWidth(col++, cx);

        ++i;
    }
}

void CSiteList::StoreColumnWidths()
{
    if (::IsWindow(m_hWnd) == FALSE)
        return;

    double totalWidth = GetTotalWidth();
    if (totalWidth <= 0.0)
        totalWidth = 0.1;

    int col = 1;

    ColumnEntries::iterator i = m_columns.begin();
    ColumnEntries::iterator end = m_columns.end();

    while (i != end) {
        ColumnEntry & entry = *i;

        entry.width = static_cast<double>(GetColumnWidth(col++)) * 100.0 / totalWidth;

        ++i;
    }
}

void CSiteList::OnHeaderEndTrack(NMHDR *, LRESULT *result)
{
    StoreColumnWidths();
    *result = 0;
}

void CSiteList::OnHeaderBeginTrack(NMHDR *pNMHDR, LRESULT *result)
{
    // Allow sizing of any column except the first one
    NMHEADER *header = reinterpret_cast<NMHEADER *>(pNMHDR);
    *result = header->iItem == 0 ? TRUE : FALSE;
}

XML::CXML & operator<< (XML::CXML & xml, const CSiteList::ColumnEntry & entry)
{
    XML::CXML *column = xml.CreateChild("Column");

    if (column) {
        column->SetParam("Name", entry.name);
        column->SetParam("Width", entry.width);
    }

    return xml;
}

// Colors

void CSiteList::StoreColorSettings(XML::CXML & params) const
{
    // Colors
    if (m_colors.get())
        m_colors->Write(params);
}

void CSiteList::LoadColorSettings(const XML::CXML & params)
{
    // Color initialization
    const XML::CXML *colors = params.FindFirstChild(Colors::GetXMLName());
    if (colors)
        m_colors.reset(new Colors(*colors));
    else
        m_colors.reset(new Colors);

    UpdateListColors();
}

void CSiteList::UpdateListColors()
{
    ASSERT(m_colors.get());
    ColorPair cpair = m_colors->GetColorPair("Regular");
    // NOTE: SetTextColor() not really needed
    SetTextColor(cpair.text.color);
    SetBkColor(cpair.background.color);
}

void CSiteList::OnCustomDraw(NMHDR *pNMHDR, LRESULT *result)
{
    LPNMLVCUSTOMDRAW pInfo = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

    switch (pInfo->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
            // Control prepaint, tell Windows we want a notification for each item
            *result = CDRF_NOTIFYITEMDRAW;
            break;
        case CDDS_ITEMPREPAINT:
            *result = OnItemPrepaint(*pInfo);
            break;
        default:
            *result = CDRF_DODEFAULT;
            break;
    }
}

LRESULT CSiteList::OnItemPrepaint(NMLVCUSTOMDRAW & info)
{
    const SiteItem *site = GetSiteFromItem(static_cast<int>(info.nmcd.dwItemSpec));
    if (site) {
        const char *cname = 0;
        if (site->IsMUInfoAvailable() && site->GetMUDayCount(time(0)) == 0) {
            cname = "MU";
        } else {
            switch (site->GetState()) {
                case SiteItem::erroneous:
                    cname = "Erroneous";
                    break;
                case SiteItem::updated:
                    cname = "Updated";
                    break;
                case SiteItem::none:
                default:
                    cname = "Regular";
                    break;
            }
        }
        ASSERT(cname);
        const ColorPair & cpair = m_colors->GetColorPair(cname);
        info.clrText = cpair.text.color;
        info.clrTextBk = cpair.background.color;
    }

    return CDRF_DODEFAULT;
}

CSiteList::Colors CSiteList::GetColors() const
{
    ASSERT(m_colors.get());
    return *m_colors;
}

void CSiteList::SetColors(const Colors & colors)
{
    m_colors.reset(new Colors(colors));
    UpdateListColors();
    Invalidate();
}

// Sorting

void CSiteList::StoreSortSettings(XML::CXML & params) const
{
    // Sort
    if (m_sortColumn != NoSortColumn) {
        XML::CXML *sortParams = params.CreateChild("Sort");
        if (sortParams) {
            sortParams->SetParam("Column", m_sortColumn);

            switch (m_sortOrder) {
                case descending:
                    sortParams->SetParam("Order", "Descending");
                    break;
                case ascending:
                default:
                    sortParams->SetParam("Order", "Ascending");
                    break;
            }
        }
    }
}

void CSiteList::LoadSortSettings(const XML::CXML & params)
{
    // Sort parameters initialization (no sorting by default)
    m_sortColumn = NoSortColumn;
    const XML::CXML *sortParams = params.FindFirstChild("Sort");
    if (sortParams) {
        const char *sortOrder = sortParams->GetParamText("Order", "Ascending");
        // Default sort column: Name
        int sortColumn = sortParams->GetParamInt("Column", 1);
        if (strcmp(sortOrder, "Descending") == 0)
            m_sortOrder = descending;
        else
            m_sortOrder = ascending;
        if (sortColumn >= 0 && sortColumn < ColumnCount)
            m_sortColumn = sortColumn;
    }
}

void CSiteList::OnColumnClick(NMHDR *pNMHDR, LRESULT *result)
{
    LPNMLISTVIEW info = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if (m_sortColumn == info->iSubItem) {
        // Same sort column was chosen, switch between ascending/descending
        // sort order.
        m_sortOrder = static_cast<SortOrder>((m_sortOrder + 1) % sortOrderingCount);
    } else {
        // A different sort column was chosen, default order is ascending
        m_sortColumn = info->iSubItem;
        m_sortOrder = ascending;
    }

    Sort();

    *result = 0;
}

class CSiteList::SortPred
{
    CSiteList & m_owner;
public:
    SortPred(CSiteList & owner) : m_owner(owner) {}

    boost::tribool OrderByImportance(const WebWatch::SiteItem & s1, const WebWatch::SiteItem & s2) const
    {
        if (m_owner.IsOrderedByImportance()) {
            std::time_t now = std::time(0);
            int d1 = s1.IsMUInfoAvailable() ? s1.GetMUDayCount(now) : WebWatch::MUSettings::GetMaxSpan() + 1;
            int d2 = s2.IsMUInfoAvailable() ? s2.GetMUDayCount(now) : WebWatch::MUSettings::GetMaxSpan() + 1;
            if (d1 != d2) {
                if (d1 == 0)
                    return true;
                else if (d2 == 0)
                    return false;
            }

            // Both items have the same MU day count or none of them expires today,
            // continue with sorting by state

            SiteItem::State st1 = s1.GetState();
            SiteItem::State st2 = s2.GetState();

            if (st1 > st2)
                return true;
            else if (st1 < st2)
                return false;

            // Both items have the same state, continue with usual sorting
        }

        return boost::indeterminate;
    }

    bool operator() (const WebWatch::SiteItem *s1, const WebWatch::SiteItem *s2) const
    {
        ASSERT(s1 && s2);

        boost::tribool r = OrderByImportance(*s1, *s2);
        if (indeterminate(r) == false)
            return r;

        int result = 0;

        const WebWatch::SiteItem & item1 = *s1;
        const WebWatch::SiteItem & item2 = *s2;

        switch (m_owner.m_sortColumn) {
            case 0:
                // Sort by checkboxes
                {
                    bool check1 = item1.IsCheckedByDefault();
                    bool check2 = item2.IsCheckedByDefault();
                    if (check1 && !check2)
                        result = -1;
                    else if (!check1 && check2)
                        result = 1;
                }
                break;
            case 1:
                // Sort by name
                result = item1.GetName().compare(item2.GetName());
                break;
            case 2:
                // Sort by address
                result = item1.GetAddress().compare(item2.GetAddress());
                break;
            case 3:
                // Sort by status
                result = item1.GetVerboseStatus().compare(item2.GetVerboseStatus());
                break;
            case 4:
                // Sort by check time
                {
                    bool checkResultsAvail1 = item1.AreCheckResultsAvailable();
                    bool checkResultsAvail2 = item2.AreCheckResultsAvailable();
                    if (checkResultsAvail1 && !checkResultsAvail2) {
                        result = -1;
                    } else if (!checkResultsAvail1 && checkResultsAvail2) {
                        result = 1;
                    } else if (checkResultsAvail1 && checkResultsAvail2) {
                        XML::CXML checkTime;
                        item1.GetCheckTime(checkTime);

                        struct tm tm1;
                        Util::ParseXMLTimeNode(checkTime, 0, tm1);

                        checkTime.ResetContents();
                        item2.GetCheckTime(checkTime);

                        struct tm tm2;
                        Util::ParseXMLTimeNode(checkTime, 0, tm2);

                        time_t time1 = mktime(&tm1);
                        time_t time2 = mktime(&tm2);

                        double diff = difftime(time1, time2);
                        if (diff > 0.0)
                            result = -1;
                        else if (diff < 0.0)
                            result = 1;

                    }
                }
                break;
            case NoSortColumn:
            default:
                // No sort
                break;

        }

        return (result * (m_owner.m_sortOrder == ascending ? 1 : -1)) < 1;
    }
};

void CSiteList::Sort()
{
    std::vector<int> items;
    items.reserve(m_items.size());
    GetSelectedSites(std::back_inserter(items));
    std::vector<WebWatch::SiteItem *> sites(items.size());
    std::transform(items.begin(), items.end(), sites.begin(), std::bind1st(std::mem_fun(&CSiteList::GetSiteFromItem), this));
    int markItem = GetSelectionMark();
    const WebWatch::SiteItem *markSite = markItem == -1 ? 0 : GetSiteFromItem(markItem);

    std::sort(m_items.begin(), m_items.end(), SortPred(*this));

    SelectAll(false);
    std::transform(sites.begin(), sites.end(), items.begin(), std::bind1st(std::mem_fun(&CSiteList::GetItemFromSite), this));
    std::for_each(items.begin(), items.end(), SiteSelector(true, *this));
    if (markSite) {
        markItem = GetItemFromSite(markSite);
        SetItemState(markItem, LVIS_FOCUSED, LVIS_FOCUSED);
        SetSelectionMark(markItem);
    }

    RedrawItems(0, static_cast<int>(m_items.size()));
    Invalidate();
}

// Popup menus

void CSiteList::OnRightClick(NMHDR *pNMHDR, LRESULT *result)
{
    LPNMITEMACTIVATE info = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    CPoint pt = info->ptAction;
    ClientToScreen(&pt);

    CMenu & menu = info->iItem == -1 ? m_menuNoItem : m_menu;
    menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);

    *result = 1;
}

void CSiteList::OnInitMenu(CMenu *menu)
{
    CListCtrl::OnInitMenu(menu);
    if (menu)
        UpdateMenu(*menu);
}

void CSiteList::UpdateMenu(CMenu & menu)
{
    UINT count = GetSelectedCount();

    menu.EnableMenuItem(ID_SITE_CHECK_SPECIFIC, (count > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITE_BROWSE, (count > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITE_COPY_ADDRESS, (count > 0) ? MF_ENABLED : MF_GRAYED);

    menu.EnableMenuItem(ID_SITE_MARK_AS_UPDATED, (count > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITE_UNMARK, (count > 0) ? MF_ENABLED : MF_GRAYED);

    menu.EnableMenuItem(ID_SITE_BEGIN_MU, (count > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITE_END_MU, (count > 0) ? MF_ENABLED : MF_GRAYED);

    menu.EnableMenuItem(ID_SITE_MODIFY, (count == 1) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITE_REMOVE, (count > 0) ? MF_ENABLED : MF_GRAYED);
    menu.EnableMenuItem(ID_SITE_MOVE_TO_GROUP, (count > 0) ? MF_ENABLED : MF_GRAYED);

    menu.EnableMenuItem(ID_SITE_ADD, m_addSiteEnabled ? MF_ENABLED : MF_GRAYED);
}

void CSiteList::InitializeMenus()
{
    m_menuNoItem.CreatePopupMenu();
    m_menuNoItem.AppendMenu(MF_ENABLED, ID_SITE_ADD, "Add...");

    m_menu.CreatePopupMenu();
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_CHECK_SPECIFIC, "Check...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_BROWSE, "Browse...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_COPY_ADDRESS, "Copy address to clipboard");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_MARK_AS_UPDATED, "Mark as updated");
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_UNMARK, "Unmark");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_BEGIN_MU, "Begin MU session...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_END_MU, "End MU session");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_MODIFY, "Modify...");
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_REMOVE, "Remove");
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_MOVE_TO_GROUP, "Move to group...");
    m_menu.AppendMenu(MF_SEPARATOR);
    m_menu.AppendMenu(MF_ENABLED, ID_SITE_ADD, "Add...");
}

void CSiteList::EnableAddSite(bool enable)
{
    m_addSiteEnabled = enable;
}

// Site manipulation functions & functors

template<class Iter>
Iter CSiteList::GetSelectedSites(Iter iter)
{
    POSITION pos = GetFirstSelectedItemPosition();

    while (pos) {
        int item = GetNextSelectedItem(pos);
        if (item != -1)
            *iter++ = item;
    }

    return iter;
}

namespace {
    class SiteDeleter
    {
        SiteItemGroup & m_group;
    public:
        explicit SiteDeleter(SiteItemGroup & group) : m_group(group) {}

        void operator() (SiteItem *site)
        {
            m_group.DeleteSite(*site);
        }
    };

    class SiteMover
    {
        SiteItemGroup & m_group;
    public:
        explicit SiteMover(SiteItemGroup & group) : m_group(group) {}

        void operator() (SiteItem *site)
        {
            ASSERT(site);
            m_group.MoveSite(*site);
        }
    };

    struct IncludeAll
    {
        bool operator() (const WebWatch::SiteItem *site) const
        {
            return site != 0;
        }

        bool operator() (const WebWatch::SiteItemGroup *group) const
        {
            return group != 0;
        }
    };
}

class CSiteList::SiteRemover
{
    CSiteList & m_owner;
public:
    explicit SiteRemover(CSiteList & owner) : m_owner(owner) {}

    void operator() (int item)
    {
        m_owner.SetItemCountEx(static_cast<int>(m_owner.m_items.size()) - 1);
        m_owner.m_items.erase(m_owner.m_items.begin() + item);
    }
};

class CSiteList::GroupAppender
{
    CSiteList & m_owner;
public:
    GroupAppender(CSiteList & owner) : m_owner(owner) {}

    void operator() (SiteItemGroup *group)
    {
        if (group) {
            WebWatch::AddSitesIf(*group, std::back_inserter(m_owner.m_items), IncludeAll(), IncludeAll());
            m_owner.SetItemCountEx(static_cast<int>(m_owner.m_items.size()));
        }
    }
};

class CSiteList::SiteAppender
{
public:
    explicit SiteAppender(CSiteList & owner)
    : m_owner(owner)
    {
    }

    void operator() (SiteItem *site)
    {
        m_owner.m_items.push_back(site);
        m_owner.SetItemCountEx(static_cast<int>(m_owner.m_items.size()));
    }

private:
    CSiteList & m_owner;
};

// Algorithms

namespace {
    template<typename T1, typename T2>
    class ComposedAlgorithm
    {
    public:
        ComposedAlgorithm(T1 t1, T2 t2)
        : m_t1(t1)
        , m_t2(t2)
        {
        }

        template<typename Iter>
        bool operator() (Iter first, Iter last)
        {
            return m_t1(first, last) ? m_t2(first, last) : false;
        }

    private:
        T1 m_t1;
        T2 m_t2;
    };

    template<typename T1, typename T2>
    ComposedAlgorithm<T1, T2> ComposeAlgorithm(T1 t1, T2 t2)
    {
        return ComposedAlgorithm<T1, T2>(t1, t2);
    }

    template<typename T>
    class SiteOperatingAlgorithm
    {
    public:
        SiteOperatingAlgorithm(CSiteList & owner, T t)
        : m_owner(owner)
        , m_t(t)
        {
        }

        template<typename Iter>
        bool operator() (Iter first, Iter last)
        {
            std::vector<WebWatch::SiteItem *> sites(std::distance(first, last));
            std::transform(first, last, sites.begin(), std::bind1st(std::mem_fun(&CSiteList::GetSiteFromItem), &m_owner));
            return m_t(sites.begin(), sites.end());
        }

    private:
        CSiteList & m_owner;
        T m_t;
    };

    template<typename T>
    SiteOperatingAlgorithm<T> AdaptSOAlg(CSiteList & owner, T t)
    {
        return SiteOperatingAlgorithm<T>(owner, t);
    }
}

class CSiteList::SortAlgorithm
{
    CSiteList & m_owner;
public:
    SortAlgorithm(CSiteList & owner) : m_owner(owner) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        m_owner.Sort();
        return true;
    }
};

class CSiteList::UpdateAlgorithm
{
    CSiteList & m_owner;
public:
    UpdateAlgorithm(CSiteList & owner) : m_owner(owner) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        if (first != last) {
            Iter mine = std::min_element(first, last);
            Iter maxe = std::max_element(first, last);
            m_owner.RedrawItems(*mine, *maxe);
            std::for_each(m_owner.m_hooks.begin(), m_owner.m_hooks.end(), HookFn(&Hook::OnUpdate));
        }
        return true;
    }
};

class CSiteList::NotifyAlgorithm
{
    CSiteList & m_owner;
    HookFn & m_fn;
public:
    NotifyAlgorithm(CSiteList & owner, HookFn & fn) : m_owner(owner), m_fn(fn) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        std::for_each(m_owner.m_hooks.begin(), m_owner.m_hooks.end(), m_fn);
        return true;
    }
};

class CSiteList::TotalUpdateAlgorithm
{
    CSiteList & m_owner;
public:
    TotalUpdateAlgorithm(CSiteList & owner) : m_owner(owner) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        ASSERT(m_owner.m_group);
        m_owner.SetSiteItemGroup(*m_owner.m_group);
        std::for_each(first, last, SiteSelector(true, m_owner));
        return true;
    }
};

class CSiteList::RemoveAlgorithm
{
    CSiteList & m_owner;
public:
    RemoveAlgorithm(CSiteList & owner) : m_owner(owner) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        // Make sure elements are removed from bottom of the list
        // to top, since item indices are updated after each removal
        std::for_each(first, last, SiteSelector(false, m_owner));
        std::sort(first, last, std::greater<int>());
        std::vector<WebWatch::SiteItem *> sites(std::distance(first, last));
        std::transform(first, last, sites.begin(), std::bind1st(std::mem_fun(&CSiteList::GetSiteFromItem), &m_owner));
        std::for_each(first, last, SiteRemover(m_owner));
        ASSERT(m_owner.m_group);
        std::for_each(sites.begin(), sites.end(), SiteDeleter(*m_owner.m_group));
        m_owner.RedrawItems(0, static_cast<int>(m_owner.m_items.size()));
        return true;
    }
};

class CSiteList::ModifyAlgorithm
{
public:
    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        if (std::distance(first, last) == 1) {
            SiteItem *site = *first;
            return SiteOperationManager::Modify(*site);
        } else {
            return false;
        }
    }
};

class CSiteList::MoveAlgorithm
{
    CSiteList & m_owner;
    WebWatch::SiteItemGroup & m_group;
public:
    MoveAlgorithm(CSiteList & owner, WebWatch::SiteItemGroup & group) : m_owner(owner), m_group(group) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        std::for_each(first, last, SiteMover(m_group));
        return true;
    }
};

class CSiteList::SetCBDAlgorithm
{
public:
    typedef bool (*SetFn)(const SiteItem *site);

    explicit SetCBDAlgorithm(SetFn op) : m_op(op) {}

    template<typename Iter>
    bool operator() (Iter first, Iter last)
    {
        for (; first != last; ++first) {
            SiteItem *site = *first;
            bool set = m_op(site);
            site->SetCheckedByDefault(set);
        }
        return true;
    }

    static bool Set(const SiteItem *site) { return true; }
    static bool Unset(const SiteItem *site) { return false; }
    static bool Invert(const SiteItem *site) { return !site->IsCheckedByDefault(); }

private:
    SetFn m_op;
};

class CSiteList::SelectedSitesPerform
{
    CSiteList & m_owner;
public:
    SelectedSitesPerform(CSiteList & owner) : m_owner(owner) {}

    template<typename Action>
    bool operator() (Action action)
    {
        typedef std::vector<int> Sites;
        Sites sites(m_owner.GetItemCount());
        Sites::iterator first = sites.begin();
        Sites::iterator last = m_owner.GetSelectedSites(first);
        return action(first, last);
    }
};

// Site actions

void CSiteList::OnCheckSpecific()
{
    ASSERT(m_checkHook);
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SiteOperationManager::Check(*m_checkHook, GetParent())), ComposeAlgorithm(
        UpdateAlgorithm(*this),
        SortAlgorithm(*this))));
}

void CSiteList::OnBrowse()
{
    CWnd *wnd = AfxGetMainWnd();
    ASSERT(wnd);
    SelectedSitesPerform(*this)(
        AdaptSOAlg(*this, SiteOperationManager::Browse(*wnd)));
}

void CSiteList::OnCopySiteAddress()
{
    CWnd *wnd = AfxGetMainWnd();
    ASSERT(wnd);
    if (SelectedSitesPerform(*this)(AdaptSOAlg(*this, SiteOperationManager::CopyAddresses(*wnd))) == false)
        MessageBox("Could not copy address(es) to clipboard", "Problem", MB_ICONEXCLAMATION);
}

void CSiteList::OnMarkAsUpdated()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SiteOperationManager::MarkAsUpdated()), ComposeAlgorithm(
        UpdateAlgorithm(*this),
        SortAlgorithm(*this))));
}

void CSiteList::OnUnmark()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SiteOperationManager::Unmark()), ComposeAlgorithm(
        UpdateAlgorithm(*this), 
        SortAlgorithm(*this))));
}

void CSiteList::OnBeginMUSession()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SiteOperationManager::BeginMUSession()), ComposeAlgorithm(
        NotifyAlgorithm(*this, HookFn(&Hook::OnUpdate)),
        SortAlgorithm(*this))));
}

void CSiteList::OnEndMUSession()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SiteOperationManager::EndMUSession()), ComposeAlgorithm(
        NotifyAlgorithm(*this, HookFn(&Hook::OnUpdate)),
        SortAlgorithm(*this))));
}

void CSiteList::OnRemove()
{
    int res = MessageBox("Are you sure you want to remove the selected site(s)?", "Warning", MB_YESNO);
    if (res == IDNO)
        return;

    SelectedSitesPerform(*this)(ComposeAlgorithm(
        RemoveAlgorithm(*this),
        NotifyAlgorithm(*this, HookFn(&Hook::OnSiteDeletion))));
}

void CSiteList::OnModifySite()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, ModifyAlgorithm()), ComposeAlgorithm(
        UpdateAlgorithm(*this),
        SortAlgorithm(*this))));
}

void CSiteList::OnMoveToGroup()
{
    SiteItemGroup *group = ChooseGroup(GetParent());
    if (group == 0)
        return;

    ASSERT(Store::Instance().IsRootGroup(*group) == false);

    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, MoveAlgorithm(*this, *group)), ComposeAlgorithm(
        TotalUpdateAlgorithm(*this),
        NotifyAlgorithm(*this, HookFn(&Hook::OnSiteMove)))));
}

void CSiteList::OnSetCBD()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SetCBDAlgorithm(&SetCBDAlgorithm::Set)), ComposeAlgorithm(
        UpdateAlgorithm(*this),
        SortAlgorithm(*this))));
}

void CSiteList::OnUnsetCBD()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SetCBDAlgorithm(&SetCBDAlgorithm::Unset)), ComposeAlgorithm(
        UpdateAlgorithm(*this),
        SortAlgorithm(*this))));
}

void CSiteList::OnInvertCBD()
{
    SelectedSitesPerform(*this)(ComposeAlgorithm(
        AdaptSOAlg(*this, SetCBDAlgorithm(&SetCBDAlgorithm::Invert)), ComposeAlgorithm(
        UpdateAlgorithm(*this),
        SortAlgorithm(*this))));
}

void CSiteList::OnActivateSite()
{
    ASSERT(m_itemActivationAction);
    (this->*m_itemActivationAction)();
}

void CSiteList::OnAddSite()
{
    SiteItem *site = SiteOperationManager::Add();
    if (site) {
        m_group->AddSite(*site);
        SiteAppender(*this)(site);
        Sort();
        std::for_each(m_hooks.begin(), m_hooks.end(), HookFn(&Hook::OnSiteAddition));
        std::for_each(m_hooks.begin(), m_hooks.end(), HookFn(&Hook::OnUpdate));
    }
}

void CSiteList::OnSelectAll()
{
    SelectAll(true);
}

void CSiteList::SelectAll(bool sel)
{
    SiteSelector selector(sel, *this);
    int count = GetItemCount();
    for (int i = 0; i < count; i++)
        selector(i);
}

// Hook management

void CSiteList::AttachHook(Hook & hook)
{
    ASSERT(std::find(m_hooks.begin(), m_hooks.end(), &hook) == m_hooks.end());
    m_hooks.push_back(&hook);
}

void CSiteList::DetachHook(Hook & hook)
{
    Hooks::iterator it = std::find(m_hooks.begin(), m_hooks.end(), &hook);
    if (it != m_hooks.end())
        m_hooks.erase(it);
}

// Misc. functions

BOOL CSiteList::OnChildNotify(UINT msg, WPARAM wp, LPARAM lp, LRESULT *result)
{
    if (msg == WM_NOTIFY) {
        NMHDR *header = reinterpret_cast<NMHDR *>(lp);
        if (header->code == LVN_ODFINDITEM) {
            NMLVFINDITEM *info = reinterpret_cast<NMLVFINDITEM *>(header);
            *result = OnODFindItem(*info);
            return TRUE;
        }
    }

    return CListCtrl::OnChildNotify(msg, wp, lp, result);
}

LRESULT CSiteList::OnODFindItem(NMLVFINDITEM & info)
{
    int idx = info.iStart;
    int item = -1;

    if (info.lvfi.flags & LVFI_STRING || info.lvfi.flags & LVFI_PARTIAL) {
        int nitems = static_cast<int>(m_items.size());
        int rounds = info.lvfi.flags & LVFI_WRAP ? nitems - idx : nitems;
        const char *text = info.lvfi.psz;
        int len = lstrlen(text);
        while (rounds-- > 0) {
            ASSERT(nitems);
            idx %= nitems;

            const WebWatch::SiteItem *site = m_items[idx];
            if (site) {
                const std::string & name = site->GetName();
                // Always partial
                if (name.compare(0, len, text) == 0) {
                    item = idx;
                    break;
                }
            }

            idx++;
        }
    }

    return item;
}

void CSiteList::OnClick(NMHDR *header, LRESULT *result)
{
    NMLISTVIEW *info = reinterpret_cast<NMLISTVIEW *>(header);
    LVHITTESTINFO hitinfo;
    hitinfo.pt = info->ptAction;
    int item = SubItemHitTest(&hitinfo);
    if (item != -1 && hitinfo.flags & LVHT_ONITEMSTATEICON) {
        WebWatch::SiteItem *site = GetSiteFromItem(item);
        if (site) {
            site->SetCheckedByDefault(!site->IsCheckedByDefault());
            RedrawItems(item, item);
        }
    }

    *result = 0;
}

void CSiteList::OnGetDispInfo(NMHDR *header, LRESULT *result)
{
    LV_DISPINFO *info = reinterpret_cast<LV_DISPINFO *>(header);
    LV_ITEM & item = info->item;
    std::size_t idx = item.iItem;
    if (idx < m_items.size()) {
        WebWatch::SiteItem *site = m_items[idx];
        if (site) {
            if (item.mask & LVIF_TEXT) {
                char *text = item.pszText;
                int maxlen = item.cchTextMax;

                switch (item.iSubItem) {
                    case 0:
                        break;
                    case 1:
                        {
                            const std::string & name = site->GetName();
                            lstrcpyn(text, name.c_str(), maxlen);
                        }
                        break;
                    case 2:
                        {
                            const std::string & address = site->GetAddress();
                            lstrcpyn(text, address.c_str(), maxlen);
                        }
                        break;
                    case 3:
                        {
                            const std::string & status = site->GetVerboseStatus();
                            lstrcpyn(text, status.c_str(), maxlen);
                        }
                        break;
                    case 4:
                        {
                            if (site->AreCheckResultsAvailable()) {
                                XML::CXML checkTime;
                                site->GetCheckTime(checkTime);
                                struct tm tm;
                                Util::ParseXMLTimeNode(checkTime, 0, tm);
                                
                                if (IsViewUpdateDifferenceEnabled()) {
                                    time_t now = time(0);
                                    time_t then = mktime(&tm);
                                    if (now != -1 && then != -1) {
                                        int diff = static_cast<int>(difftime(now, then));
                                        
                                        int seconds = diff % 60;
                                        int minutes = diff / 60;
                                        int hours = minutes / 60;
                                        int days = hours / 24;
                                        hours = hours % 24;
                                        minutes = minutes % 60;

                                        std::ostringstream os;
                                        if (days)
                                            os << days << "D ";
                                        if (hours)
                                            os << hours << "H ";
                                        if (minutes)
                                            os << minutes << "M ";
                                        if ((!days && !hours && !minutes) || seconds)
                                            os << seconds << "S ";

                                        const std::string & lupdate = os.str();
                                        lstrcpyn(text, lupdate.c_str(), maxlen);
                                        if (text[0]) {
                                            int len = lstrlen(text);
                                            while (len && text[len - 1] == ' ')
                                                text[--len] = '\0';
                                        }
                                    }
                                }

                                if (text[0] == '\0')
                                    strftime(text, maxlen, "%c", &tm);
                            } else {
                                lstrcpyn(text, "-", maxlen);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        if (item.mask & LVIF_IMAGE) {
            if (item.iSubItem == 0) {
                item.mask |= LVIF_STATE;
                item.stateMask = LVIS_STATEIMAGEMASK;
                int tmp = site->IsCheckedByDefault() ? 2 : 1;
                item.state = INDEXTOSTATEIMAGEMASK(tmp);
            }
        }
    }

    *result = 0;
}

WebWatch::SiteItem *CSiteList::GetSiteFromItem(int item)
{
    return static_cast<std::size_t>(item) < m_items.size() ? m_items[item] : 0;
}

int CSiteList::GetItemFromSite(const WebWatch::SiteItem *site)
{
    ASSERT(site);
    Items::const_iterator i = std::find(m_items.begin(), m_items.end(), site);
    return i == m_items.end() ? -1 : static_cast<int>(i - m_items.begin());
}

int CSiteList::GetTotalWidth()
{
    CRect rect;
    GetClientRect(&rect);
    int width = rect.Width() - m_checkWidth;
    if (GetItemCount() < GetCountPerPage())
        width -= m_scrollerWidth;
    return width;
}

void CSiteList::UpdateView()
{
    Sort();
    std::for_each(m_hooks.begin(), m_hooks.end(), HookFn(&Hook::OnUpdate));
}

const char *CSiteList::GetXMLName()
{
    return "SiteList";
}

void CSiteList::GetDefaultSettings(XML::CXML & xml)
{
    xml.ResetContents();
    xml.SetTagName(GetXMLName());

    // Set up columns
    std::runtime_error columnError("Could not create Column XML for SiteList settings");

    XML::CXML *column = xml.CreateChild("Column");
    if (column == 0)
        throw columnError;
    column->SetParam("Name", "Name");
    column->SetParam("Width", 25.0);

    column = xml.CreateChild("Column");
    if (column == 0)
        throw columnError;
    column->SetParam("Name", "Address");
    column->SetParam("Width", 45.0);

    column = xml.CreateChild("Column");
    if (column == 0)
        throw columnError;
    column->SetParam("Name", "Status");
    column->SetParam("Width", 20.0);

    column = xml.CreateChild("Column");
    if (column == 0)
        throw columnError;
    column->SetParam("Name", "Last Updated");
    column->SetParam("Width", 10.0);
}

void CSiteList::SetSiteItemGroup(SiteItemGroup & group)
{
    m_group = &group;
    DeleteAllItems();
    m_items.clear();
    GroupAppender(*this)(m_group);
    Sort();
    std::for_each(m_hooks.begin(), m_hooks.end(), HookFn(&Hook::OnUpdate));
}

const SiteItemGroup & CSiteList::GetSiteItemGroup() const
{
    ASSERT(m_group);
    return *m_group;
}

// Site item activation

class CSiteList::ItemActivationActionMapperImpl
{
    typedef stdext::hash_map<std::string, SiteListFn> ActionMap;

public:
    ItemActivationActionMapperImpl()
    {
        m_actions.insert(std::make_pair("Browse", &CSiteList::OnBrowse));
        m_actions.insert(std::make_pair("Check", &CSiteList::OnCheckSpecific));
        m_actions.insert(std::make_pair("Modify", &CSiteList::OnModifySite));
        m_actions.insert(std::make_pair("CopyAddress", &CSiteList::OnCopySiteAddress));
    }

    SiteListFn GetAction(const std::string & name) const
    {
        ActionMap::const_iterator it = m_actions.find(name);
        if (it != m_actions.end())
            return it->second;
        return 0;
    }

    std::string GetName(SiteListFn action) const
    {
        ActionMap::const_iterator it = m_actions.begin();
        while (it != m_actions.end()) {
            if (it->second == action)
                return it->first;
            ++it;
        }
        
        return Util::EmptyString();
    }

    ActionNames GetActionNames() const
    {
        ActionNames names;
        ActionMap::const_iterator it = m_actions.begin();
        ActionMap::const_iterator end = m_actions.end();
        while (it != end) {
            names.push_back(it->first);
            ++it;
        }
        return names;
    };

private:
    ActionMap m_actions;
};

CSiteList::SiteListFn CSiteList::GetDefaultItemActivationAction()
{
    return &CSiteList::OnBrowse;
}

void CSiteList::LoadItemActivationSettings(const XML::CXML & params)
{
    const XML::CXML *xmlActivation = params.FindFirstChild("ItemActivation");
    std::string actionName;
    if (xmlActivation)
        actionName = xmlActivation->GetParamText("Action");
    else
        actionName = "Default";
    SetItemActivationAction(actionName);
}

void CSiteList::StoreItemActivationSettings(XML::CXML & params) const
{
    if (m_itemActivationAction != GetDefaultItemActivationAction()) {
        XML::CXML *xmlActivation = params.CreateChild("ItemActivation");
        if (xmlActivation) {
            std::string actionName = ItemActivationActionMapper::Instance().GetName(m_itemActivationAction);
            ASSERT(actionName.empty() == false);
            xmlActivation->SetParam("Action", actionName.c_str());
        }
    }
}

void CSiteList::OnItemActivate(NMHDR *header, LRESULT *result)
{
    OnActivateSite();
    *result = 0;
}

CSiteList::ActionNames CSiteList::GetItemActivationActions()
{
    return ItemActivationActionMapper::Instance().GetActionNames();
}

void CSiteList::SetItemActivationAction(const std::string & actionName)
{
    SiteListFn action = ItemActivationActionMapper::Instance().GetAction(actionName);
    m_itemActivationAction = action ? action : GetDefaultItemActivationAction();
}

std::string CSiteList::GetItemActivationAction() const
{
    return ItemActivationActionMapper::Instance().GetName(m_itemActivationAction);
}

// View stuff

void CSiteList::LoadViewSettings(const XML::CXML & params)
{
    bool viewUpdateDifference = DefaultViewUpdateDifference;
    bool orderByImportance = DefaultOrderByImportance;

    const XML::CXML *xmlView = params.FindFirstChild("View");
    if (xmlView) {
        viewUpdateDifference = xmlView->GetParamBool("UpdateDifference", viewUpdateDifference);
        orderByImportance = xmlView->GetParamBool("OrderByImportance", orderByImportance);
    }

    m_viewUpdateDifference = viewUpdateDifference;
    m_orderByImportance = orderByImportance;
}

void CSiteList::StoreViewSettings(XML::CXML & params) const
{
    std::auto_ptr<XML::CXML> xmlView(new XML::CXML("View"));
    
    if (m_viewUpdateDifference != DefaultViewUpdateDifference) {
        xmlView->SetParam("UpdateDifference", m_viewUpdateDifference);
    }

    if (m_orderByImportance != DefaultOrderByImportance) {
        xmlView->SetParam("OrderByImportance", m_orderByImportance);
    }

    if (xmlView->IsEmpty() == false)
        params.AddChild(xmlView.release());
}

void CSiteList::EnableViewUpdateDifference(bool enable)
{
    if (m_viewUpdateDifference != enable) {
        m_viewUpdateDifference = enable;
        UpdateView();
    }
}

bool CSiteList::IsViewUpdateDifferenceEnabled() const
{
    return m_viewUpdateDifference;
}

void CSiteList::EnableOrderByImportance(bool enable)
{
    if (m_orderByImportance != enable) {
        m_orderByImportance = enable;
        UpdateView();
    }
}

bool CSiteList::IsOrderedByImportance() const
{
    return m_orderByImportance;
}

// Implementation of internal stuff

namespace {

SiteItemGroup *ChooseGroup(CWnd *parentWnd)
{
    class MyHook : public CChooseGroupDlg::Hook
    {
    public:
        virtual ~MyHook() {}
        virtual bool CanChoose(const SiteItemGroup *group) const
        {
            return group != 0 && Store::Instance().IsRootGroup(*group) == false;
        }
    } hook;

    CChooseGroupDlg dlg(Store::Instance().GetRootGroup(), &hook, parentWnd);

    if (dlg.DoModal() != IDOK)
        return 0;

    return dlg.GetChosenGroup();
}

}
