// WebWatch UI - by DEATH, 2004
//
// $Workfile: SetMUDlg.cpp $ - Set MU dialog
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/TimeUtil.h"

#include "SetMUDlg.h"
#include "Store.h"
#include "MU.h"
#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

BEGIN_MESSAGE_MAP(CSetMUDlg, CDialog)
END_MESSAGE_MAP()

CSetMUDlg::CSetMUDlg(time_t now)
: CDialog(CSetMUDlg::IDD)
, m_now(now)
, m_expiry(0)
{
}

void CSetMUDlg::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_EXPIRY_DATE, m_expiryDate);
    CDialog::DoDataExchange(pDX);
}

bool CSetMUDlg::IsExpirySet() const
{
    return m_expiry != 0;
}

time_t CSetMUDlg::GetExpiry() const
{
    ASSERT(IsExpirySet());
    return m_expiry;
}

void CSetMUDlg::OnOK()
{
    // Grab time from control
    SYSTEMTIME sysTime;
    DWORD res = m_expiryDate.GetTime(&sysTime);
    if (res != GDT_VALID) {
        CDialog::OnOK();
        return;
    }

    // MU always expires at midnight
    sysTime.wHour = 0;
    sysTime.wMinute = 0;
    sysTime.wSecond = 0;
    sysTime.wMilliseconds = 0;

    m_expiry = Util::SystemTimeToTime(sysTime, true);
    
    CDialog::OnOK();
}

BOOL CSetMUDlg::OnInitDialog()
{
    try {
        WebWatch::MUSettings settings;
        WebWatch::Store::Instance().GetMUSettings(settings);
        
        CTime tm = WebWatch::ConvertTimeToCTime(m_now);
        CTimeSpan span(settings.GetSpan(), -tm.GetHour(), -tm.GetMinute(), -tm.GetSecond());
        tm += span;

        UpdateData(FALSE);

        m_expiryDate.SetTime(&tm);
    }
    catch (const std::exception & ex) {
        MessageBox(ex.what(), "Exception", MB_OK | MB_ICONERROR);
    }

    return TRUE;
}
