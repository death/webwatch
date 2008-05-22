// WebWatch UI - by DEATH, 2004
//
// $Workfile: Clipboard.cpp $ - Clipboard utilities
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "Clipboard.h"

namespace Util_DE050401
{
    bool CopyTextToClipboard(HWND hwnd, const std::string & text)
    {
        bool copied = false;
        
        if (OpenClipboard(hwnd)) {
            if (EmptyClipboard()) {
                if (text.empty() == false) {
                    HGLOBAL hBuf = GlobalAlloc(GHND, text.length() + 1);
                    if (hBuf) {
                        char *buf = static_cast<char *>(GlobalLock(hBuf));
                        if (buf) {
                            lstrcpy(buf, text.c_str());
                            if (GlobalUnlock(hBuf) == 0 && GetLastError() == NO_ERROR) {
                                if (SetClipboardData(CF_TEXT, hBuf))
                                    copied = true;
                            }
                        }
                        if (copied == false)
                            GlobalFree(hBuf);
                    }
                } else {
                    copied = true;
                }
            }
            CloseClipboard();
        }

        return copied;
    }
}
