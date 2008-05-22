// WebWatch UI - by DEATH, 2004
//
// $Workfile: ChooseGroupDlg.cpp $ - Choose group dialog
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "ChooseGroupDlg.h"
#include "SiteItemGroup.h"

namespace 
{
    class GroupAppender
    {
    public:
        GroupAppender(CTreeCtrl & tree, HTREEITEM parentItem);
        HTREEITEM operator() (const WebWatch::SiteItemGroup *group);

    private:
        CTreeCtrl & m_tree;
        HTREEITEM m_parentItem;
    };
}

BEGIN_MESSAGE_MAP(CChooseGroupDlg, CDialog)
END_MESSAGE_MAP()

CChooseGroupDlg::CChooseGroupDlg(WebWatch::SiteItemGroup & rootGroup, Hook *hook, CWnd *parentWnd)
: CDialog(IDD, parentWnd)
, m_rootGroup(rootGroup)
, m_chosenGroup(0)
, m_hook(hook)
{
}

void CChooseGroupDlg::DoDataExchange(CDataExchange *pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_GROUP_TREE, m_tree);
}

BOOL CChooseGroupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    PopulateTree();
    return TRUE;
}

void CChooseGroupDlg::OnOK()
{
    HTREEITEM item = m_tree.GetSelectedItem();
    
    ASSERT(m_chosenGroup == 0);

    if (item)
        m_chosenGroup = reinterpret_cast<WebWatch::SiteItemGroup *>(m_tree.GetItemData(item));

    if (m_hook && m_hook->CanChoose(m_chosenGroup) == false) {
        MessageBox("The selected group cannot be chosen, select another group.", 
                   "Select another group", MB_OK | MB_ICONINFORMATION);
        m_chosenGroup = 0;
        return;
    }

    CDialog::OnOK();
}

WebWatch::SiteItemGroup *CChooseGroupDlg::GetChosenGroup() const
{
    return m_chosenGroup;
}

void CChooseGroupDlg::PopulateTree()
{
    m_tree.SetRedraw(FALSE);
    m_tree.DeleteAllItems();
    
    GroupAppender appender(m_tree, 0);
    HTREEITEM rootItem = appender(&m_rootGroup);

    if (rootItem) {
        m_tree.Expand(rootItem, TVE_EXPAND);
        m_tree.SelectItem(rootItem);
    }

    m_tree.SetRedraw();
}

namespace 
{
    GroupAppender::GroupAppender(CTreeCtrl & tree, HTREEITEM parentItem)
    : m_tree(tree)
    , m_parentItem(parentItem)
    {
    }

    HTREEITEM GroupAppender::operator() (const WebWatch::SiteItemGroup *group)
    {
        ASSERT(group);
        const std::string & name = group->GetName();
        
        HTREEITEM item = m_tree.InsertItem(name.c_str(), m_parentItem);
        if (item) {
            m_tree.SetItemData(item, reinterpret_cast<DWORD_PTR>(group));

            // Add child groups

            if (group->GetGroupCount() > 0) {
                GroupAppender appender(m_tree, item);
                std::for_each(group->GetGroupsBegin(), group->GetGroupsEnd(), appender);
            }
        }

        return item;
    }
}
