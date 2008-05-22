// WebWatch UI - by DEATH, 2004
//
// $Workfile: SettingsSiteGroupsTreePage.cpp $ - Settings Site Groups Tree property page
//
// $Author: Death $
// $Revision: 1 $
// $Date: 11/09/04 20:36 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "SettingsSiteGroupsTreePage.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace {
    const char *ColorNames[] = {
        "Regular",
        "Updated",
        "MU",
        "Droppable",
        "Nondroppable",
        "Drag mask"
    };

    const std::size_t ColorOffsets[] = {
        offsetof(CSiteGroupsTree::Colors, regular),
        offsetof(CSiteGroupsTree::Colors, updated),
        offsetof(CSiteGroupsTree::Colors, mu),
        offsetof(CSiteGroupsTree::Colors, droppable),
        offsetof(CSiteGroupsTree::Colors, nondroppable),
        offsetof(CSiteGroupsTree::Colors, dragMask)
    };

    const std::size_t ColorCount = sizeof(ColorNames) / sizeof(ColorNames[0]);
}

IMPLEMENT_DYNAMIC(CSettingsSiteGroupsTreePage, CPropertyPage)

BEGIN_MESSAGE_MAP(CSettingsSiteGroupsTreePage, CPropertyPage)
    ON_WM_CTLCOLOR()
    ON_CBN_SELCHANGE(IDC_ITEM_COLOR_NAME, OnItemColorNameChanged)
    ON_COMMAND(IDC_COLOR_SET, OnSetItemColor)
    ON_COMMAND(ID_SET_TEXT_COLOR, OnSetItemTextColor)
    ON_COMMAND(ID_SET_DEFAULT_COLORS, OnSetItemDefaultColors)
END_MESSAGE_MAP()

CSettingsSiteGroupsTreePage::CSettingsSiteGroupsTreePage(CSiteGroupsTree & groupsTree, CSettingsDlg & owner)
: CPropertyPage(CSettingsSiteGroupsTreePage::IDD)
, m_owner(&owner)
, m_groupsTree(&groupsTree)
{
    m_pPSP->dwFlags &= ~PSP_HASHELP;
    m_pPSP->dwFlags |= PSP_USETITLE;
    m_pPSP->pszTitle = "Site Groups Tree";
    if (m_colorMenu.CreatePopupMenu()) {
        m_colorMenu.AppendMenu(MF_STRING, ID_SET_TEXT_COLOR, "Text color...");
        m_colorMenu.AppendMenu(MF_STRING, ID_SET_DEFAULT_COLORS, "Default color");
    }
}

CSettingsSiteGroupsTreePage::~CSettingsSiteGroupsTreePage()
{
}

void CSettingsSiteGroupsTreePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ITEM_COLOR_NAME, m_color);
}

BOOL CSettingsSiteGroupsTreePage::OnSetActive()
{
    if (m_color.GetCount() == 0) {
        m_color.ResetContent();
        m_colors = m_groupsTree->GetColors();
        for (int i = 0; i < ColorCount; i++)
            m_color.AddString(ColorNames[i]);
        m_color.SetCurSel(0);
        OnItemColorNameChanged();
    }

    return CPropertyPage::OnSetActive();
}

void CSettingsSiteGroupsTreePage::OnOK()
{
    m_groupsTree->SetColors(m_colors);
    CPropertyPage::OnOK();
}

std::string CSettingsSiteGroupsTreePage::GetCurrentColorName() const
{
    int sel = m_color.GetCurSel();
    CString colorName;
    m_color.GetLBText(sel, colorName);
    return std::string(colorName);
}

void CSettingsSiteGroupsTreePage::OnItemColorNameChanged()
{
    GetDlgItem(IDC_COLOR_SAMPLE_TEXT)->Invalidate();
}

HBRUSH CSettingsSiteGroupsTreePage::OnCtlColor(CDC *dc, CWnd *wnd, UINT type)
{
    HBRUSH brush = CPropertyPage::OnCtlColor(dc, wnd, type);

    if (type == CTLCOLOR_STATIC && wnd->GetDlgCtrlID() == IDC_COLOR_SAMPLE_TEXT) {
        const WebWatch::Color & col = GetCurrentColor();
        dc->SetTextColor(col.color);
    }

    return brush;
}

void CSettingsSiteGroupsTreePage::OnSetItemColor()
{
    CPoint pos;
    GetCursorPos(&pos);
    if (m_colorMenu.m_hMenu)
        m_colorMenu.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
}

WebWatch::Color & CSettingsSiteGroupsTreePage::GetCurrentColor(CSiteGroupsTree::Colors *colors)
{
    if (colors == 0)
        colors = &m_colors;
    const std::string & name = GetCurrentColorName();
    for (int i = 0; i < ColorCount; i++) {
        if (name == ColorNames[i])
            return *reinterpret_cast<WebWatch::Color *>(reinterpret_cast<char *>(colors) + ColorOffsets[i]);
    }
    throw std::logic_error("Logic error in groups tree page's GetCurrentColor()");
}

void CSettingsSiteGroupsTreePage::OnSetItemTextColor()
{
    WebWatch::Color & col = GetCurrentColor();
    CColorDialog dlg(col.color, 0, this);
    switch (dlg.DoModal()) {
        case IDOK:
            {
            col.color = dlg.GetColor();
            GetDlgItem(IDC_COLOR_SAMPLE_TEXT)->Invalidate();
            }
            break;
        default:
            break;
    }
}

void CSettingsSiteGroupsTreePage::OnSetItemDefaultColors()
{
    CSiteGroupsTree::Colors defaultColors;
    GetCurrentColor() = GetCurrentColor(&defaultColors);
    GetDlgItem(IDC_COLOR_SAMPLE_TEXT)->Invalidate();
}
