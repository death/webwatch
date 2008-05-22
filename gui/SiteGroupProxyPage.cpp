// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupProxyPage.cpp $ - Site group Proxy property page
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "core/Core.h"
#include "core/HttpSocketFactory.h"

#include "SiteGroupProxyPage.h"
#include "SiteGroupPropertiesDlg.h"
#include "SiteItemGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace {
    struct UsageAndID
    {
        WebWatch::SiteItemGroup::ProxyData::Usage usage;
        UINT id;
    };

    const UsageAndID ProxyOptionTable[] = {
        {WebWatch::SiteItemGroup::ProxyData::noProxy, IDC_NO_PROXY},
        {WebWatch::SiteItemGroup::ProxyData::useProxy, IDC_USE_PROXY},
        {WebWatch::SiteItemGroup::ProxyData::inheritProxy, IDC_INHERIT_PROXY}
    };

    const int ProxyOptionTableEntries = sizeof(ProxyOptionTable) / sizeof(ProxyOptionTable[0]);

    WebWatch::SiteItemGroup::ProxyData::Usage GetUsageFromID(UINT id);
    UINT GetIDFromUsage(WebWatch::SiteItemGroup::ProxyData::Usage usage);
}

IMPLEMENT_DYNAMIC(CSiteGroupProxyPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSiteGroupProxyPage, CPropertyPage)
    ON_BN_CLICKED(IDC_NO_PROXY, OnProxyOption)
    ON_BN_CLICKED(IDC_INHERIT_PROXY, OnProxyOption)
    ON_BN_CLICKED(IDC_USE_PROXY, OnProxyOption)
    ON_CBN_SELCHANGE(IDC_PROXY_TYPE, SaveSettings)
    ON_EN_CHANGE(IDC_PROXY_ADDRESS, SaveSettings)
END_MESSAGE_MAP()

CSiteGroupProxyPage::CSiteGroupProxyPage(CSiteGroupPropertiesDlg & owner, WebWatch::SiteItemGroup & group, bool isRootGroup)
: CPropertyPage(CSiteGroupProxyPage::IDD)
, m_owner(&owner)
, m_group(group)
, m_isRootGroup(isRootGroup)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "Proxy";
}

CSiteGroupProxyPage::~CSiteGroupProxyPage()
{
}

void CSiteGroupProxyPage::DoDataExchange(CDataExchange *dx)
{
	CPropertyPage::DoDataExchange(dx);
    DDX_Control(dx, IDC_PROXY_TYPE, m_proxyType);
    DDX_Text(dx, IDC_PROXY_ADDRESS, m_proxyAddress);
}

BOOL CSiteGroupProxyPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    typedef Core::HttpSocketFactoryImpl::ProxyTypes ProxyTypes;
    const ProxyTypes & types = Core::HttpSocketFactory::Instance().GetProxyTypes();
    ASSERT(types.empty() == false);
    ProxyTypes::const_iterator i;
    for (i = types.begin(); i != types.end(); ++i)
        m_proxyType.AddString(i->c_str());

    return FALSE;
}

BOOL CSiteGroupProxyPage::OnSetActive()
{
    m_proxyData = m_group.GetProxyData();
    int sel = m_proxyType.FindString(-1, m_proxyData.type.c_str());
    if (sel >= 0)
        m_proxyType.SetCurSel(sel);
    m_proxyAddress = m_proxyData.address.c_str();
    UpdateData(FALSE);

    CheckDlgButton(GetIDFromUsage(m_proxyData.usage), BST_CHECKED);
    OnProxyOption();

    return CPropertyPage::OnSetActive();
}

void CSiteGroupProxyPage::OnOK()
{
    CPropertyPage::OnOK();
}

void CSiteGroupProxyPage::OnProxyOption()
{
    UINT id;
    if (IsDlgButtonChecked(IDC_NO_PROXY) == BST_CHECKED)
        id = IDC_NO_PROXY;
    else if (IsDlgButtonChecked(IDC_USE_PROXY) == BST_CHECKED)
        id = IDC_USE_PROXY;
    else if (IsDlgButtonChecked(IDC_INHERIT_PROXY) == BST_CHECKED)
        id = IDC_INHERIT_PROXY;

    m_proxyData.usage = GetUsageFromID(id);

    BOOL enable = m_proxyData.usage == WebWatch::SiteItemGroup::ProxyData::useProxy ? TRUE : FALSE;
    GetDlgItem(IDC_PROXY_ADDRESS)->EnableWindow(enable);
    m_proxyType.EnableWindow(enable);

    SaveSettings();
}

void CSiteGroupProxyPage::SaveSettings()
{
    UpdateData();
    m_proxyData.address = static_cast<const char *>(m_proxyAddress);
    int sel = m_proxyType.GetCurSel();
    if (sel != -1) {
        CString text;
        m_proxyType.GetLBText(sel, text);
        m_proxyData.type = static_cast<const char *>(text);
    }
    m_group.SetProxyData(m_proxyData);

    bool ok = m_proxyData.usage != WebWatch::SiteItemGroup::ProxyData::useProxy ||
              (m_proxyData.address.empty() == false && m_proxyData.type.empty() == false);
    m_owner->EnableOK(ok);
}

namespace {
    WebWatch::SiteItemGroup::ProxyData::Usage GetUsageFromID(UINT id)
    {
        for (int i = 0; i < ProxyOptionTableEntries; i++)
            if (ProxyOptionTable[i].id == id)
                return ProxyOptionTable[i].usage;
        return WebWatch::SiteItemGroup::ProxyData::noProxy;
    }

    UINT GetIDFromUsage(WebWatch::SiteItemGroup::ProxyData::Usage usage)
    {
        for (int i = 0; i < ProxyOptionTableEntries; i++)
            if (ProxyOptionTable[i].usage == usage)
                return ProxyOptionTable[i].id;
        return IDC_NO_PROXY;
    }
}
