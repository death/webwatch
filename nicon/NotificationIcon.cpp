/* $Workfile: NotificationIcon.cpp $
 *
 * Notification icon class.
 *
 * $Author: Death $
 * $Revision: 1 $
 * $Date: 9/24/03 2:20 $
 *
 * $NoKeywords: $
 */
#include "StdAfx.h"
#include "notificationicon.h"

// Constructor
CNotificationIcon::CNotificationIcon(HWND hwnd, UINT uId)
: m_bNewShell(true)
, m_bIsHidden(false)
, m_bIsShared(false)
{
    ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
    m_nid.cbSize = sizeof(NOTIFYICONDATA);

#if (_WIN32_IE >= 0x500)
    DLLGETVERSIONPROC DllGetVersion = reinterpret_cast<DLLGETVERSIONPROC>(GetProcAddress(GetModuleHandle(_T("shell32.dll")), "DllGetVersion"));
    if (DllGetVersion) {
        DLLVERSIONINFO dvi;
        if (SUCCEEDED(DllGetVersion(&dvi))) {
            if (dvi.dwMajorVersion < 5) {
                m_nid.cbSize = NOTIFYICONDATA_V2_SIZE;
                m_bNewShell = false;
            }
        }
    }
#else
    m_bNewShell = false;
#endif

    m_nid.hWnd = hwnd;
    m_nid.uID = uId;
}

// Destructor
CNotificationIcon::~CNotificationIcon(void)
{
}

// Add to taskbar
void CNotificationIcon::Add(void) const
{
    Shell_NotifyIcon(NIM_ADD, const_cast<PNOTIFYICONDATA>(&m_nid));
}

// Update notification icon
void CNotificationIcon::Update(UINT uFlags /* = 0 */) const
{
    NOTIFYICONDATA nid;
    CopyMemory(&nid, &m_nid, sizeof(NOTIFYICONDATA));

    if (uFlags != 0) {
        nid.uFlags = uFlags;
    }
    
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

// Remove notification icon
void CNotificationIcon::Remove(void) const
{
    Shell_NotifyIcon(NIM_DELETE, const_cast<PNOTIFYICONDATA>(&m_nid));
}

// Set callback message identifier, if 0, there is none
void CNotificationIcon::SetCallbackMessage(UINT uMsg)
{
    if (uMsg == 0) {
        m_nid.uFlags &= ~NIF_MESSAGE;
    } else {
        m_nid.uFlags |= NIF_MESSAGE;
    }
    m_nid.uCallbackMessage = uMsg;
}

// Get callback message identifier
UINT CNotificationIcon::GetCallbackMessage(void) const
{
    return(m_nid.uCallbackMessage);
}

// Set icon, if NULL, there is none
void CNotificationIcon::SetIcon(HICON hIcon)
{
    if (hIcon == NULL) {
        m_nid.uFlags &= ~NIF_ICON;
    } else {
        m_nid.uFlags |= NIF_ICON;
    }
    m_nid.hIcon = hIcon;
}

// Get icon
HICON CNotificationIcon::GetIcon(void) const
{
    return(m_nid.hIcon);
}

// Set notification icon tip, if NULL, there is none
void CNotificationIcon::SetTip(LPCTSTR pszTip)
{
    if (pszTip == NULL) {
        m_nid.uFlags &= ~NIF_TIP;
    } else {
        m_nid.uFlags |= NIF_TIP;
    }
    lstrcpyn(m_nid.szTip, pszTip, (sizeof(m_nid.szTip) / sizeof(TCHAR)) - 1);
}

// Get tip
LPCTSTR CNotificationIcon::GetTip(void) const
{
    return(static_cast<LPCTSTR>(m_nid.szTip));
}

// Are we running on a new shell
bool CNotificationIcon::IsNewShell(void) const
{
    return(m_bNewShell);
}

// New options
#if (_WIN32_IE >= 0x500)

// Set info/info title, if NULL, there is none, also sets timeout
void CNotificationIcon::SetInfo(LPCTSTR pszInfo, DWORD dwFlags /* = NIIF_NONE */, LPCTSTR pszInfoTitle /* = NULL */, UINT uTimeout /* = 0 */)
{
    if (pszInfo == NULL) {
        m_nid.uFlags &= ~NIF_INFO;
        m_nid.uTimeout = 0;
        m_nid.szInfo[0] = '\0';
        m_nid.szInfoTitle[0] = '\0';
        m_nid.dwInfoFlags = 0;
    } else {
        m_nid.uFlags |= NIF_INFO;
        m_nid.uTimeout = uTimeout;
        m_nid.dwInfoFlags = dwFlags;
        lstrcpyn(m_nid.szInfo, pszInfo, (sizeof(m_nid.szInfo) / sizeof(TCHAR)) - 1);

        if (pszInfoTitle == NULL) {
            m_nid.szInfoTitle[0] = '\0';
        } else {
            lstrcpyn(m_nid.szInfoTitle, pszInfoTitle, (sizeof(m_nid.szInfoTitle) / sizeof(TCHAR)) - 1);
        }
    }
}

// Get info
LPCTSTR CNotificationIcon::GetInfo(void) const
{
    return(static_cast<LPCTSTR>(m_nid.szInfo));
}

// Get info title
LPCTSTR CNotificationIcon::GetInfoTitle(void) const
{
    return(static_cast<LPCTSTR>(m_nid.szInfoTitle));
}

// Get timeout value
UINT CNotificationIcon::GetTimeout(void) const
{
    return(m_nid.uTimeout);
}

// Hide/show notification icon
void CNotificationIcon::Hide(void)
{
    NOTIFYICONDATA nid;
    CopyMemory(&nid, &m_nid, sizeof(NOTIFYICONDATA));

    nid.dwState = NIS_HIDDEN;
    nid.dwStateMask = NIS_HIDDEN;
    nid.uFlags = NIF_STATE;

    Shell_NotifyIcon(NIM_MODIFY, &nid);

    m_bIsHidden = true;
}

void CNotificationIcon::Show(void) 
{
    NOTIFYICONDATA nid;
    CopyMemory(&nid, &m_nid, sizeof(NOTIFYICONDATA));

    nid.dwState = 0;
    nid.dwStateMask = NIS_HIDDEN;
    nid.uFlags = NIF_STATE;

    Shell_NotifyIcon(NIM_MODIFY, &nid);

    m_bIsHidden = false;
}

// Share/unshare notification icon
void CNotificationIcon::Share(void) 
{
    NOTIFYICONDATA nid;
    CopyMemory(&nid, &m_nid, sizeof(NOTIFYICONDATA));

    nid.dwState = NIS_SHAREDICON;
    nid.dwStateMask = NIS_SHAREDICON;
    nid.uFlags = NIF_STATE;

    Shell_NotifyIcon(NIM_MODIFY, &nid);

    m_bIsShared = true;
}

void CNotificationIcon::Unshare(void) 
{
    NOTIFYICONDATA nid;
    CopyMemory(&nid, &m_nid, sizeof(NOTIFYICONDATA));

    nid.dwState = 0;
    nid.dwStateMask = NIS_SHAREDICON;
    nid.uFlags = NIF_STATE;

    Shell_NotifyIcon(NIM_MODIFY, &nid);

    m_bIsShared = false;
}

// Determine wether the icon is hidden/shared
bool CNotificationIcon::IsHidden(void) const
{
    return(m_bIsHidden);
}

bool CNotificationIcon::IsShared(void) const
{
    return(m_bIsShared);
}

#endif

