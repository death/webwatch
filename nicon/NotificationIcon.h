/* $Workfile: NotificationIcon.h $
 *
 * Notification icon class header.
 *
 * $Author: Death $
 * $Revision: 1 $
 * $Date: 9/24/03 2:20 $
 *
 * $NoKeywords: $
 */
#pragma once

class CNotificationIcon
{
public:
    CNotificationIcon(HWND hwnd, UINT uId);
    ~CNotificationIcon(void);

    // Add to taskbar
    void Add(void) const;

    // Update notification icon
    void Update(UINT uFlags = 0) const;

    // Remove notification icon
    void Remove(void) const;

    // Are we running on a new shell?
    bool IsNewShell(void) const;

    // Callback message identifier handling
    void SetCallbackMessage(UINT uMsg);
    UINT GetCallbackMessage(void) const;

    // Icon handling
    void SetIcon(HICON hIcon);
    HICON GetIcon(void) const;

    // Tip handling
    void SetTip(LPCTSTR pszTip);
    LPCTSTR GetTip(void) const;

// New options
#if (_WIN32_IE >= 0x500)
    // Info handling
    void SetInfo(LPCTSTR pszInfo, DWORD dwFlags = NIIF_NONE, LPCTSTR pszInfoTitle = NULL, UINT uTimeout = 0);
    LPCTSTR GetInfo(void) const;
    LPCTSTR GetInfoTitle(void) const;
    UINT GetTimeout(void) const;

    // Hide/show notification icon
    void Hide(void); 
    void Show(void);
    bool IsHidden(void) const;

    // Share/unshare notification icon
    void Share(void);
    void Unshare(void);
    bool IsShared(void) const;
#endif

private:
    NOTIFYICONDATA m_nid;
    bool m_bNewShell;
    bool m_bIsHidden;
    bool m_bIsShared;
};
