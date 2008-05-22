// WebWatch UI - by DEATH, 2004
//
// $Workfile: Common.cpp $ - Common utilities
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/TimeUtil.h"

#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace WebWatch
{
    // NOTE: This function doesn't do file existence check
    bool MakeBackup(const char *filename, BackupType type, HWND hwndParent)
    {
        std::string backupType;
        switch (type) {
            case perRun:
                backupType = "per-run";
                break;
            case perSave:
                backupType = "per-save";
                break;
            default:
                return false; // Unknown backup type
        }
        
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        _splitpath(filename, drive, dir, fname, ext);

        std::ostringstream backupFileName;
        backupFileName << fname << "-" << backupType << "-backup";

        char path[_MAX_PATH];
        _makepath(path, drive, dir, backupFileName.str().c_str(), ext);

        bool backedup = !!CopyFile(filename, path, FALSE);

        if (backedup == false) {
            std::string msg("WebWatch was unable to make a ");
            msg += backupType;
            msg += " backup of your configuration. now would be a good time"
                   " to make one yourself";
            MessageBox(hwndParent, msg.c_str(), "Warning", MB_OK | MB_ICONEXCLAMATION);
        }

        return backedup;
    }

    bool IsFileExistent(const char *filename)
    {
        return std::ifstream(filename).good();
    }

    bool EffectiveShowWindow(CWnd *wnd, bool show)
    {
        bool result(false);
        
        if (wnd) {
            bool visible = !!wnd->IsWindowVisible();
            if (show != visible) {
                wnd->ShowWindow(show ? SW_SHOW : SW_HIDE);
                result = true;
            }
        }
        
        return result;
    }

    CTime ConvertTimeToCTime(time_t t)
    {
        if (struct tm *gmt = gmtime(&t))
            return CTime(gmt->tm_year + 1900, 
                         gmt->tm_mon + 1, 
                         gmt->tm_mday, 
                         gmt->tm_hour, 
                         gmt->tm_min, 
                         gmt->tm_sec, 
                         gmt->tm_isdst ? 1 : 0);
        else
            throw std::logic_error("Invalid time passed to ConvertTimeToCTime");
    }
}