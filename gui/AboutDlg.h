// WebWatch UI - by DEATH, 2004
//
// $Workfile: AboutDlg.h $ - About dialog
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 6:07 $
// $NoKeywords: $
//
#pragma once

#include "util/Util.h"
#include "util/Picture.h"

#include "Resource.h"

class CAboutDlg : public CDialog
{
    DECLARE_MESSAGE_MAP()

public:
    CAboutDlg();

    enum { IDD = IDD_ABOUT };

private:
    virtual BOOL OnInitDialog();

    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC *dc);
    afx_msg HBRUSH OnCtlColor(CDC *dc, CWnd *wnd, UINT type);

private:
    Util::PicturePtr m_pic;
};