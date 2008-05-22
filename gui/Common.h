// WebWatch UI - by DEATH, 2004
//
// $Workfile: Common.h $ - Common utilities
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_COMMON_H
#define WEBWATCH_INC_COMMON_H

namespace WebWatch
{

enum BackupType
{
    perRun, perSave
};

bool MakeBackup(const char *filename, BackupType type, HWND hwndParent);
bool IsFileExistent(const char *filename);
bool EffectiveShowWindow(CWnd *wnd, bool show);
CTime ConvertTimeToCTime(time_t t);

} 

#endif // WEBWATCH_INC_COMMON_H