// WebWatch UI - by DEATH, 2004
//
// $Workfile: ChooseGroupDlg.h $ - Choose group dialog
//
// $Author: Death $
// $Revision: 2 $
// $Date: 2/07/04 17:00 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

namespace WebWatch
{
    class SiteItemGroup;
}

class CChooseGroupDlg : public CDialog
{
    DECLARE_MESSAGE_MAP()

public:
    class Hook
    {
    public:
        virtual ~Hook() {}
        virtual bool CanChoose(const WebWatch::SiteItemGroup *group) const = 0;
    };

public:
    enum { IDD = IDD_CHOOSE_GROUP };

    explicit CChooseGroupDlg(WebWatch::SiteItemGroup & rootGroup, Hook *hook = 0, CWnd *parentWnd = 0);

    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange *pDX);
    virtual void OnOK();

    WebWatch::SiteItemGroup *GetChosenGroup() const;

private:
    void PopulateTree();

private:
    WebWatch::SiteItemGroup & m_rootGroup;
    WebWatch::SiteItemGroup *m_chosenGroup;
    CTreeCtrl m_tree;
    Hook *m_hook;
};