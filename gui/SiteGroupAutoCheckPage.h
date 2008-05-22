// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupAutoCheckPage.h $ - Site group Auto Check property page
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

class CSiteGroupAutoCheckPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSiteGroupAutoCheckPage)

public:
    CSiteGroupAutoCheckPage(CSiteGroupPropertiesDlg & owner, WebWatch::SiteItemGroup & group, bool isRootGroup);
	virtual ~CSiteGroupAutoCheckPage();

	enum { IDD = IDD_SITEGROUP_AUTO_CHECK_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();
    
    afx_msg void OnHScroll(UINT sbCode, UINT pos, CScrollBar *scrollBar);
    afx_msg void OnIncludedInParentAutoCheckClicked();

	DECLARE_MESSAGE_MAP()

private:
    void UpdateText();
    void UpdateProperties();

private:
    CSliderCtrl m_slider;
    CSiteGroupPropertiesDlg *m_owner;
    WebWatch::SiteItemGroup & m_group;
    bool m_isRootGroup;
};
