// WebWatch UI - by DEATH, 2004
//
// $Workfile: SetMUDlg.h $ - Set MU dialog
//
// $Author: Death $
// $Revision: 1 $
// $Date: 16/04/04 17:25 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

class CSetMUDlg : public CDialog
{
public:
    CSetMUDlg(time_t now);

    enum { IDD = IDD_SET_MU };

    bool IsExpirySet() const;
    time_t GetExpiry() const;

protected:
    virtual void DoDataExchange(CDataExchange *pDX);
    virtual void OnOK();
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    time_t m_now;
    time_t m_expiry;
    CDateTimeCtrl m_expiryDate;
};
