// WebWatch UI - by DEATH, 2004
//
// $Workfile: SitePropertiesDlg.h $ - Site properties dialog (property sheet)
//
// $Author: Death $
// $Revision: 3 $
// $Date: 25/05/04 19:38 $
// $NoKeywords: $
//
#pragma once

#include "SiteGeneralPage.h"
#include "SiteCheckPage.h"
#include "SiteMUPage.h"

namespace WebWatch
{
    class SiteItem;
}

class CSitePropertiesDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CSitePropertiesDlg)

public:
    CSitePropertiesDlg(WebWatch::SiteItem & site, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CSitePropertiesDlg(WebWatch::SiteItem & site, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CSitePropertiesDlg();

    void EnableOK(bool enable);

protected:
    DECLARE_MESSAGE_MAP()

private:
    void Initialize();

private:
    CSiteGeneralPage m_general;
    CSiteCheckPage m_check;
    CSiteMUPage m_mu;
};

