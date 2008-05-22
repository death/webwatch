// WebWatch UI - by DEATH, 2004
//
// $Workfile: WebWatch.h $ - Application header
//
// $Author: Death $
// $Revision: 1 $
// $Date: 16/04/04 14:14 $
// $NoKeywords: $
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

class CWebWatchApp : public CWinApp
{
public:
	CWebWatchApp();

public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CWebWatchApp theApp;