// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsSiteListPage.cpp $ - Settings Site List property page
//
// $Author: Death $
// $Revision: 5 $
// $Date: 11/09/04 20:37 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SettingsSiteListPage.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace {
    class StringAppender : public std::unary_function<std::string, void>
    {
    public:
        StringAppender(CComboBox & box) 
        : m_box(box) 
        {
        }
        
        void operator() (const std::string & str)
        {
            m_box.AddString(str.c_str());
        }

    private:
        CComboBox & m_box;
    };
}

IMPLEMENT_DYNAMIC(CSettingsSiteListPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSettingsSiteListPage, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_CBN_SELCHANGE(IDC_ITEM_ACTIVATION_ACTION, OnItemActivationActionChanged)
    ON_CBN_SELCHANGE(IDC_ITEM_COLOR_NAME, OnItemColorNameChanged)
    ON_COMMAND(IDC_COLOR_SET, OnSetItemColor)
    ON_COMMAND(ID_SET_TEXT_COLOR, OnSetItemTextColor)
    ON_COMMAND(ID_SET_BACKGROUND_COLOR, OnSetItemBackgroundColor)
    ON_COMMAND(ID_SET_DEFAULT_COLORS, OnSetItemDefaultColors)
END_MESSAGE_MAP()

CSettingsSiteListPage::CSettingsSiteListPage(CSiteList & siteList, CSettingsDlg & owner)
: CPropertyPage(CSettingsSiteListPage::IDD)
, m_owner(&owner)
, m_siteList(&siteList)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "Site List";
    if (m_colorMenu.CreatePopupMenu()) {
        m_colorMenu.AppendMenu(MF_STRING, ID_SET_TEXT_COLOR, "Text color...");
        m_colorMenu.AppendMenu(MF_STRING, ID_SET_BACKGROUND_COLOR, "Background color...");
        m_colorMenu.AppendMenu(MF_STRING, ID_SET_DEFAULT_COLORS, "Default colors");
    }
}

CSettingsSiteListPage::~CSettingsSiteListPage()
{
}

void CSettingsSiteListPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ITEM_ACTIVATION_ACTION, m_action);
    DDX_Control(pDX, IDC_ITEM_COLOR_NAME, m_color);
}

BOOL CSettingsSiteListPage::OnSetActive()
{
    if (m_action.GetCount() == 0) {
        m_action.ResetContent();
        const CSiteList::ActionNames & actionNames = CSiteList::GetItemActivationActions();
        std::for_each(actionNames.begin(), actionNames.end(), StringAppender(m_action));
        const std::string & actionName = m_siteList->GetItemActivationAction();
        m_action.SelectString(-1, actionName.c_str());
    }

    if (m_color.GetCount() == 0) {
        m_color.ResetContent();
        m_colors = m_siteList->GetColors();
        const CSiteList::Colors::ColorPairNames & cpairNames = CSiteList::Colors::GetColorPairNames();
        std::for_each(cpairNames.begin(), cpairNames.end(), StringAppender(m_color));
        m_color.SetCurSel(0);
        OnItemColorNameChanged();
    }

    return CPropertyPage::OnSetActive();
}

void CSettingsSiteListPage::OnOK()
{
    m_siteList->SetColors(m_colors);
    CPropertyPage::OnOK();
}

void CSettingsSiteListPage::OnItemActivationActionChanged()
{
    int sel = m_action.GetCurSel();
    CString actionName;
    m_action.GetLBText(sel, actionName);
    m_siteList->SetItemActivationAction(std::string(actionName));
}

std::string CSettingsSiteListPage::GetCurrentColorName() const
{
    int sel = m_color.GetCurSel();
    CString colorName;
    m_color.GetLBText(sel, colorName);
    return std::string(colorName);
}

void CSettingsSiteListPage::OnItemColorNameChanged()
{
    GetDlgItem(IDC_COLOR_SAMPLE_TEXT)->Invalidate();
}

HBRUSH CSettingsSiteListPage::OnCtlColor(CDC *dc, CWnd *wnd, UINT type)
{
    HBRUSH brush = CPropertyPage::OnCtlColor(dc, wnd, type);

    if (type == CTLCOLOR_STATIC && wnd->GetDlgCtrlID() == IDC_COLOR_SAMPLE_TEXT) {
        const WebWatch::ColorPair & cpair = GetCurrentColorPair();
        dc->SetTextColor(cpair.text.color);
        dc->SetBkMode(TRANSPARENT);
        if (static_cast<HBRUSH>(m_brush))
            m_brush.DeleteObject();
        m_brush.CreateSolidBrush(cpair.background.color);
        return m_brush;
    } else {
        return brush;
    }
}

void CSettingsSiteListPage::OnSetItemColor()
{
    CPoint pos;
    GetCursorPos(&pos);
    if (m_colorMenu.m_hMenu)
        m_colorMenu.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
}

void CSettingsSiteListPage::OnSetItemTextColor()
{
    SetNewColor(text);
}

void CSettingsSiteListPage::OnSetItemBackgroundColor()
{
    SetNewColor(background);
}

void CSettingsSiteListPage::OnSetItemDefaultColors()
{
    const std::string & colorName = GetCurrentColorName();
    CSiteList::Colors defaultColors;
    m_colors.SetColorPair(colorName.c_str(), defaultColors.GetColorPair(colorName.c_str()));
    GetDlgItem(IDC_COLOR_SAMPLE_TEXT)->Invalidate();
}

void CSettingsSiteListPage::SetNewColor(ColorElement el)
{
    WebWatch::ColorPair cpair = GetCurrentColorPair();
    WebWatch::Color & col = el == text ? cpair.text : cpair.background;
    CColorDialog dlg(col.color, 0, this);
    switch (dlg.DoModal()) {
        case IDOK:
            {
            col.color = dlg.GetColor();

            const std::string & colorName = GetCurrentColorName();
            m_colors.SetColorPair(colorName.c_str(), cpair);

            GetDlgItem(IDC_COLOR_SAMPLE_TEXT)->Invalidate();
            }
            break;
        default:
            break;
    }
}

const WebWatch::ColorPair & CSettingsSiteListPage::GetCurrentColorPair() const
{
    const std::string & colorName = GetCurrentColorName();
    return m_colors.GetColorPair(colorName.c_str());
}