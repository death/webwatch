// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteCheckPage.cpp $ - Site Check property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/AddressParser.h"

#include "SiteCheckPage.h"
#include "SitePropertiesDlg.h"
#include "SiteItem.h"
#include "Common.h"
#include "HttpContentChild.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace {
    enum CheckMethodType {
        CHECK_METHOD_HTTP_TIME,
        CHECK_METHOD_HTTP_SIZE,
        CHECK_METHOD_HTTP_CONTENT,
        CHECK_METHOD_FTP,
        CHECK_METHOD_COUNT
    };

    typedef std::map<std::string, CheckMethodType> CheckMethodTypeMap;

    CheckMethodTypeMap checkMethodTypeMap;

    void InitializeCheckMethodTypeMap();
    CheckMethodType DetermineCheckMethodType(const std::string & type);
}

IMPLEMENT_DYNAMIC(CSiteCheckPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSiteCheckPage, CPropertyPage)
    ON_CBN_SELCHANGE(IDC_CHECK_METHOD, OnCheckMethodChanged)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

CSiteCheckPage::CSiteCheckPage(CSitePropertiesDlg & owner, WebWatch::SiteItem & site)
: CPropertyPage(CSiteCheckPage::IDD)
, m_owner(&owner)
, m_site(site)
, m_httpContentChild(0)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "Check";

    m_paramsVec.resize(CHECK_METHOD_COUNT);
}

CSiteCheckPage::~CSiteCheckPage()
{
}

void CSiteCheckPage::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_CHECK_METHOD, m_checkMethod);
    CPropertyPage::DoDataExchange(pDX);
}

BOOL CSiteCheckPage::OnSetActive()
{
    if (m_httpContentChild == 0) {
        m_httpContentChild = new CHttpContentChild(m_site);
        m_httpContentChild->Create(IDD_CHILD_HTTP_CONTENT, this);
    }

    std::string type = m_site.GetType();

    // Set site type static
    std::stringstream ss;
    ss << "Site type: " << type;
    SetDlgItemText(IDC_SITE_TYPE, ss.str().c_str());

    // Set available check methods and socket implementation
    m_socketType.clear();
    m_checkMethod.ResetContent();
    std::string defMethod;

    if (type == "http" || type == "https") {
        defMethod = "HttpTime";
        m_checkMethod.AddString("HttpTime");
        m_checkMethod.AddString("HttpSize");
        m_checkMethod.AddString("HttpContent");
    } else if (type == "ftp") {
        defMethod = "Ftp";
        m_checkMethod.AddString("Ftp");
    } else {
        m_checkMethod.EnableWindow(FALSE);
        OnCheckMethodChanged();
        return CPropertyPage::OnSetActive();
    }

    m_checkMethod.EnableWindow();

    // Set the check method
    XML::CXML params;
    m_site.GetCheckParameters(params);

    ASSERT(params.IsEmpty() == false);
    ASSERT(strcmp(params.GetTagName(), "CheckParameters") == 0);

    const char *method = params.GetParamText("Method", defMethod.c_str());

    try {
        CheckMethodType checkMethodType = DetermineCheckMethodType(method);

        if (!m_paramsVec[checkMethodType]) {
            m_paramsVec[checkMethodType] = XmlPtr(new XML::CXML(params));
        } else {
            *m_paramsVec[checkMethodType] = params;
        }
    }
    catch(const std::exception &) {
        // Ignore exception
    }

    int ret = m_checkMethod.SelectString(0, method);
    if (ret == CB_ERR) {
        ret = m_checkMethod.SelectString(0, defMethod.c_str());
        ASSERT(ret != CB_ERR);
    }

    OnCheckMethodChanged();

    return CPropertyPage::OnSetActive();
}

void CSiteCheckPage::OnCheckMethodChanged()
{
    if (m_checkMethod.GetCount() == 0) {
        SetCheckMethodView(0);
        return;
    }

    int sel = m_checkMethod.GetCurSel();
    if (sel == CB_ERR) {
        SetCheckMethodView(0);
        return;
    }

    CString method;
    m_checkMethod.GetLBText(sel, method);
    SetCheckMethodView(method);
}

void CSiteCheckPage::SetCheckMethodView(const char *method)
{
    ASSERT(m_httpContentChild);

    if (method == 0) {
        SetDlgItemText(IDC_CHECK_METHOD_INFO, "No check method");
        m_httpContentChild->ShowWindow(SW_HIDE);
        return;
    }

    CheckMethodType checkMethodType = DetermineCheckMethodType(method);
    XmlPtr xml = m_paramsVec[checkMethodType];
    if (!xml) {
        xml.reset(new XML::CXML("CheckParameters"));
        m_paramsVec[checkMethodType] = xml;
        xml->SetParam("Method", method);
    }

    ASSERT(xml);
    ASSERT(strcmp(xml->GetParamText("Method"), method) == 0);

    bool showHttpContent = false;

    switch (checkMethodType) {
        case CHECK_METHOD_HTTP_CONTENT:
            SetDlgItemText(IDC_CHECK_METHOD_INFO, "Edit filters/regex");
            showHttpContent = true;
            m_httpContentChild->SetParamsToModify(*xml);
            break;
        case CHECK_METHOD_HTTP_TIME:
        case CHECK_METHOD_HTTP_SIZE:
        case CHECK_METHOD_FTP:
        default:
            SetDlgItemText(IDC_CHECK_METHOD_INFO, "No check method options");
            break;
    }

    WebWatch::EffectiveShowWindow(m_httpContentChild, showHttpContent);
}

BOOL CSiteCheckPage::OnKillActive()
{
    SetCheckParameters();
    return CPropertyPage::OnKillActive();
}

void CSiteCheckPage::OnOK()
{
    SetCheckParameters();
    CPropertyPage::OnOK();
}

void CSiteCheckPage::SetCheckParameters()
{
    if (m_checkMethod.GetCount() == 0) {
        XML::CXML xml;
        m_site.SetCheckParameters(xml);
        return;
    }

    int sel = m_checkMethod.GetCurSel();
    if (sel == CB_ERR) {
        XML::CXML xml;
        m_site.SetCheckParameters(xml);
        return;
    }

    CString method;
    m_checkMethod.GetLBText(sel, method);

    try {
        CheckMethodType checkMethodType = DetermineCheckMethodType(std::string(method));

        ASSERT(m_paramsVec[checkMethodType] != 0);
        const XML::CXML & params = *m_paramsVec[checkMethodType];
        m_site.SetCheckParameters(params);
    }
    catch(const std::exception &) {
        // Ignore exception
    }
}

void CSiteCheckPage::OnDestroy()
{
    if (m_httpContentChild) {
        m_httpContentChild->DestroyWindow();
        delete m_httpContentChild;
        m_httpContentChild = 0;
    }
}

namespace {

void InitializeCheckMethodTypeMap()
{
    checkMethodTypeMap["HttpTime"] = CHECK_METHOD_HTTP_TIME;
    checkMethodTypeMap["HttpSize"] = CHECK_METHOD_HTTP_SIZE;
    checkMethodTypeMap["HttpContent"] = CHECK_METHOD_HTTP_CONTENT;
    checkMethodTypeMap["Ftp"] = CHECK_METHOD_FTP;
}

CheckMethodType DetermineCheckMethodType(const std::string & type)
{
    if (checkMethodTypeMap.empty())
        InitializeCheckMethodTypeMap();

    CheckMethodTypeMap::const_iterator i = checkMethodTypeMap.find(type);
    if (i == checkMethodTypeMap.end()) {
        std::ostringstream ss;
        ss << "Unknown check method type: " << type;
        throw std::runtime_error(ss.str());
    }
    return i->second;
}

}
