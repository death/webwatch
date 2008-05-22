// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteCheckPage.h $ - Site Check property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

namespace WebWatch
{
    class SiteItem;
}

class CSitePropertiesDlg;
class CHttpContentChild;

class CSiteCheckPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CSiteCheckPage)

public:
    CSiteCheckPage(CSitePropertiesDlg & owner, WebWatch::SiteItem & site);
    virtual ~CSiteCheckPage();

    enum { IDD = IDD_SITE_CHECK_PAGE };

protected:
    virtual void DoDataExchange(CDataExchange *pDX);    // DDX/DDV support
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual void OnOK();

    afx_msg void OnCheckMethodChanged();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    void SetCheckMethodView(const char *method);
    void SetCheckParameters();

private:
    CSitePropertiesDlg *m_owner;
    WebWatch::SiteItem & m_site;
    CComboBox m_checkMethod;
    std::string m_socketType;
    CHttpContentChild *m_httpContentChild;

    typedef boost::shared_ptr<XML::CXML> XmlPtr;
    typedef std::vector<XmlPtr> ParametersVector;
    ParametersVector m_paramsVec;
};
