// WebWatch UI - by DEATH, 2004
//
// $Workfile: AboutDlg.cpp $ - About dialog
//
// $Author: Death $
// $Revision: 5 $
// $Date: 4/02/05 6:22 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace 
{
    std::ostream & Version(std::ostream & os);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg()
: CDialog(CAboutDlg::IDD)
{
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_pic = Util::Picture::FromResource(m_hWnd, 
                                        AfxGetInstanceHandle(), 
                                        "JPEG", 
                                        MAKEINTRESOURCE(IDJ_ABOUT));

    std::ostringstream text;
    text << "WebWatch " << Version << " (compiled " << __DATE__ << ")\n"
            "Coded by DEATH in 2002-2004\n"
            "WebWatch logo by CodeSafe Hawk\n"
            "Just another ACME tool!";

    SetDlgItemText(IDC_ABOUT_TEXT, text.str().c_str());

    return FALSE;
}

void CAboutDlg::OnPaint()
{
    CPaintDC dc(this);
    if (m_pic) {
        CRect rect;
        GetClientRect(&rect);
        int sx = (rect.Width() - m_pic->GetWidth()) / 2;
        int sy = 0;
        CRect picRect(sx, sy, sx + m_pic->GetWidth(), sy + m_pic->GetHeight());
        m_pic->Draw(dc, picRect);
    }
}

BOOL CAboutDlg::OnEraseBkgnd(CDC *dc)
{
    ASSERT(dc);
    CRect rect;
    GetClientRect(&rect);
    dc->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), WHITENESS);
    return TRUE;
}

HBRUSH CAboutDlg::OnCtlColor(CDC *dc, CWnd *wnd, UINT type)
{
    HBRUSH brush = CDialog::OnCtlColor(dc, wnd, type);

    ASSERT(wnd);
    if (wnd->GetDlgCtrlID() == IDC_ABOUT_TEXT) {
        ASSERT(dc);
        dc->SetBkMode(TRANSPARENT);
        brush = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    }

    return brush;
}

namespace 
{
    std::ostream & Version(std::ostream & os) 
    {
        bool written = false;
        char filename[_MAX_PATH];
        GetModuleFileName(AfxGetInstanceHandle(), filename, _MAX_PATH);
        if (DWORD infoSize = GetFileVersionInfoSize(filename, &infoSize)) {
            std::vector<char> info(infoSize);
            if (GetFileVersionInfo(filename, 0, infoSize, &info[0])) {
                VS_FIXEDFILEINFO *fileInfo;
                UINT fileInfoSize;
                void **ugly = reinterpret_cast<void **>(&fileInfo);
                if (VerQueryValue(&info[0], "\\", ugly, &fileInfoSize)) {
                    UINT major = HIWORD(fileInfo->dwProductVersionMS);
                    UINT minor = LOWORD(fileInfo->dwProductVersionMS);
                    os << "version " << major << "." << minor;
                    written = true;
                }
            }
        }
        
        if (written == false)
            os << "(unknown version)";

        return os;
    }
}