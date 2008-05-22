// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGeneralPage.h $ - Site General property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 16/04/04 17:27 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

namespace WebWatch
{
    class SiteItem;
}

class CSitePropertiesDlg;

class CSiteGeneralPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSiteGeneralPage)

public:
    CSiteGeneralPage(CSitePropertiesDlg & owner, WebWatch::SiteItem & site);
	virtual ~CSiteGeneralPage();

	enum { IDD = IDD_SITE_GENERAL_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnChangedName();
    afx_msg void OnChangedAddress();
    afx_msg void OnChangedNotes();

	DECLARE_MESSAGE_MAP()

private:
    void CheckValid();

private:
    CSitePropertiesDlg *m_owner;
    WebWatch::SiteItem & m_site;
};
