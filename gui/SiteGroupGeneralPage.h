// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupGeneralPage.h $ - Site group General property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 25/05/04 23:11 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

namespace WebWatch
{
    class SiteItemGroup;
}

class CSiteGroupPropertiesDlg;

class CSiteGroupGeneralPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSiteGroupGeneralPage)

public:
    CSiteGroupGeneralPage(CSiteGroupPropertiesDlg & owner, WebWatch::SiteItemGroup & group, bool isRootGroup);
	virtual ~CSiteGroupGeneralPage();

	enum { IDD = IDD_SITEGROUP_GENERAL_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnChangedName();

	DECLARE_MESSAGE_MAP()

private:
    void CheckValid();

private:
    CSiteGroupPropertiesDlg *m_owner;
    WebWatch::SiteItemGroup & m_group;
    bool m_isRootGroup;
};
