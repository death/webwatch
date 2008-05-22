// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteList.h $ - Site list control
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/06/05 19:32 $
// $NoKeywords: $
//
#pragma once

#include "SiteItem.h"
#include "Colors.h"
#include "SiteCheckDlg.h"
#include "Loki/Singleton.h"

namespace WebWatch
{
    class SiteItemGroup;
}

class CSiteList : public CListCtrl
{
public:
    class Hook
    {
    public:
        virtual ~Hook() {}
        virtual void OnSiteAddition() {}
        virtual void OnSiteDeletion() {}
        virtual void OnSiteMove() {}
        virtual void OnUpdate() {}
        virtual void OnDestruction() = 0;
    };

    typedef std::vector<std::string> ActionNames;

    class Colors
    {
    public:
        typedef std::vector<std::string> ColorPairNames;

    public:
        Colors();
        Colors(const WebWatch::ColorPair & regular, const WebWatch::ColorPair & updated, const WebWatch::ColorPair & erroneous, const WebWatch::ColorPair & mu);
        Colors(const XML::CXML & xml);

        XML::CXML *Write(XML::CXML & parent) const;

        static const char *GetXMLName();

        const WebWatch::ColorPair & GetColorPair(const char *name) const;
        void SetColorPair(const char *name, const WebWatch::ColorPair & cpair);
        static ColorPairNames GetColorPairNames();

    private:
        void Add(const std::string & name, const WebWatch::ColorPair & cpair);
        void AddDefault();

    private:
        typedef stdext::hash_map<std::string, WebWatch::ColorPair> ColorPairMap;
        ColorPairMap m_cpairs;
    };

private:
    DECLARE_DYNAMIC(CSiteList)
    DECLARE_MESSAGE_MAP()

    // Sort ordering
    enum SortOrder
    {
        ascending,
        descending,
        sortOrderingCount
    };

    // Column entry
    struct ColumnEntry
    {
        ColumnEntry(const char *name_, double width_)
        : name(name_)
        , width(width_)
        {
        }

        std::string name;       // Name
        double width;           // Width in percentage
    };

    typedef std::vector<ColumnEntry> ColumnEntries;

    friend XML::CXML & operator<< (XML::CXML & xml, const ColumnEntry & entry);

    typedef std::list<Hook *> Hooks;

    typedef void (CSiteList::*SiteListFn)();
    
    class ItemActivationActionMapperImpl;
    typedef Loki::SingletonHolder<ItemActivationActionMapperImpl> ItemActivationActionMapper;

public:
    CSiteList();
    virtual ~CSiteList();

    void Initialize(CSiteCheckDlg::Hook & checkHook);
    void StoreSettings() const;
    void UpdateColumns();
    void SetSiteItemGroup(WebWatch::SiteItemGroup & group);
    const WebWatch::SiteItemGroup & GetSiteItemGroup() const;
    void UpdateView();
    void EnableAddSite(bool enable = true);
    void UpdateMenu(CMenu & menu);

    void EnableViewUpdateDifference(bool enable = true);
    bool IsViewUpdateDifferenceEnabled() const;

    void EnableOrderByImportance(bool enable = true);
    bool IsOrderedByImportance() const;

    void AttachHook(Hook & hook);
    void DetachHook(Hook & hook);

    void SetItemActivationAction(const std::string & actionName);
    std::string GetItemActivationAction() const;
    static ActionNames GetItemActivationActions();

    Colors GetColors() const;
    void SetColors(const Colors & colors);

    static void GetDefaultSettings(XML::CXML & xml);
    static const char *GetXMLName();

    WebWatch::SiteItem *GetSiteFromItem(int item);
    int GetItemFromSite(const WebWatch::SiteItem *site);

public: // Overrides and message handlers
    virtual void DoDataExchange(CDataExchange *pDX);
    virtual BOOL OnChildNotify(UINT msg, WPARAM wp, LPARAM lp, LRESULT *result);

    afx_msg void OnHeaderEndTrack(NMHDR *pNMHDR, LRESULT *result);
    afx_msg void OnCustomDraw(NMHDR *pNMHDR, LRESULT *result);
    afx_msg void OnHeaderBeginTrack(NMHDR *pNMHDR, LRESULT *result);
    afx_msg BOOL OnItemChanged(NMHDR *pNMHDR, LRESULT *result);
    afx_msg void OnRightClick(NMHDR *pNMHDR, LRESULT *result);
    afx_msg void OnInitMenu(CMenu *menu);
    afx_msg void OnColumnClick(NMHDR *pNMHDR, LRESULT *result);
    afx_msg void OnItemActivate(NMHDR *header, LRESULT *result);
    afx_msg void OnGetDispInfo(NMHDR *header, LRESULT *result);
    afx_msg void OnClick(NMHDR *header, LRESULT *result);

    afx_msg void OnActivateSite();
    afx_msg void OnAddSite();
    afx_msg void OnCheckSpecific();
    afx_msg void OnBrowse();
    afx_msg void OnCopySiteAddress();
    afx_msg void OnMarkAsUpdated();
    afx_msg void OnUnmark();
    afx_msg void OnBeginMUSession();
    afx_msg void OnEndMUSession();
    afx_msg void OnModifySite();
    afx_msg void OnRemove();
    afx_msg void OnMoveToGroup();
    afx_msg void OnSetCBD();
    afx_msg void OnUnsetCBD();
    afx_msg void OnInvertCBD();
    afx_msg void OnSelectAll();

private: // Private interface
    void LoadColumnSettings(const XML::CXML & params);
    void StoreColumnSettings(XML::CXML & params) const;
    void LoadColorSettings(const XML::CXML & params);
    void StoreColorSettings(XML::CXML & params) const;
    void LoadSortSettings(const XML::CXML & params);
    void StoreSortSettings(XML::CXML & params) const;
    void LoadItemActivationSettings(const XML::CXML & params);
    void StoreItemActivationSettings(XML::CXML & params) const;
    void LoadViewSettings(const XML::CXML & params);
    void StoreViewSettings(XML::CXML & params) const;
    void StoreColumnWidths();
    void InitializeMenus();
    int GetTotalWidth();
    void SelectAll(bool sel);

    void RemoveAllColumns();
    void AddFirstColumn();
    void AddColumns(const XML::CXML & params);
    void ColumnsSanityCheck();

    LRESULT OnItemPrepaint(NMLVCUSTOMDRAW & info);
    LRESULT OnODFindItem(NMLVFINDITEM & info);

    class SortPred;
    void Sort();

    template<class Iter>
    Iter GetSelectedSites(Iter iter);

    static SiteListFn GetDefaultItemActivationAction();

    void UpdateListColors();

    class SortAlgorithm;
    class UpdateAlgorithm;
    class NotifyAlgorithm;
    class RemoveAlgorithm;
    class ModifyAlgorithm;
    class MoveAlgorithm;
    class TotalUpdateAlgorithm;
    class SetCBDAlgorithm;

    class SelectedSitesPerform;

    class SiteAppender;
    class GroupAppender;
    class SiteRemover;

private: // Data
    ColumnEntries m_columns;
    int m_checkWidth;
    int m_scrollerWidth;
    WebWatch::SiteItemGroup *m_group;
    std::auto_ptr<Colors> m_colors;
    CMenu m_menuNoItem;
    CMenu m_menu;
    int m_sortColumn;
    SortOrder m_sortOrder;
    bool m_addSiteEnabled;
    Hooks m_hooks;
    SiteListFn m_itemActivationAction;
    bool m_viewUpdateDifference;
    bool m_orderByImportance;
    CSiteCheckDlg::Hook *m_checkHook;

    typedef std::deque<WebWatch::SiteItem *> Items;
    Items m_items;
};
