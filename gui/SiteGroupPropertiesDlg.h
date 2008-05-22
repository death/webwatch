// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupPropertiesDlg.h $ - Site group properties dialog (property sheet)
//
// $Author: Death $
// $Revision: 2 $
// $Date: 11/10/04 23:01 $
// $NoKeywords: $
//
#pragma once

#include "SiteGroupGeneralPage.h"
#include "SiteGroupAutoCheckPage.h"
#include "SiteGroupProxyPage.h"

namespace WebWatch
{
    class SiteItemGroup;
}

class CSiteGroupPropertiesDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CSiteGroupPropertiesDlg)

public:
    CSiteGroupPropertiesDlg(WebWatch::SiteItemGroup & group, bool isRootGroup, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CSiteGroupPropertiesDlg(WebWatch::SiteItemGroup & group, bool isRootGroup, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CSiteGroupPropertiesDlg();

    void EnableOK(bool enable);

protected:
    DECLARE_MESSAGE_MAP()

private:
    void Initialize();

private:
    CSiteGroupGeneralPage m_general;
    CSiteGroupAutoCheckPage m_autoCheck;
    CSiteGroupProxyPage m_proxy;
};

