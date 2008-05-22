// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupProxyPage.h $ - Site group Proxy property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 11/10/04 23:01 $
// $NoKeywords: $
//
#pragma once

#include "SiteItemGroup.h"
#include "Resource.h"

class CSiteGroupPropertiesDlg;

class CSiteGroupProxyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSiteGroupProxyPage)

public:
    CSiteGroupProxyPage(CSiteGroupPropertiesDlg & owner, WebWatch::SiteItemGroup & group, bool isRootGroup);
	virtual ~CSiteGroupProxyPage();

	enum { IDD = IDD_SITEGROUP_PROXY_PAGE };

protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange *dx);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual void OnOK();

    afx_msg void OnProxyOption();
    afx_msg void SaveSettings();

	DECLARE_MESSAGE_MAP()

private:
    CSiteGroupPropertiesDlg *m_owner;
    WebWatch::SiteItemGroup & m_group;
    bool m_isRootGroup;
    CComboBox m_proxyType;
    CString m_proxyAddress;
    WebWatch::SiteItemGroup::ProxyData m_proxyData;
};
