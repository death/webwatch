// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteMUPage.h $ - Site MU property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 16/04/04 17:25 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

namespace WebWatch
{
    class SiteItem;
}

class CSitePropertiesDlg;

class CSiteMUPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSiteMUPage)

public:
    CSiteMUPage(CSitePropertiesDlg & owner, WebWatch::SiteItem & site);
	virtual ~CSiteMUPage();

	enum { IDD = IDD_SITE_MU_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnSessionButtonClicked();
    
    void ShowInfo();

    DECLARE_MESSAGE_MAP()

private:
    CSitePropertiesDlg *m_owner;
    WebWatch::SiteItem & m_site;
};
