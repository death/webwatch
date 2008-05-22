// WebWatch UI - by DEATH, 2004
//
// $Workfile: webwatch.cpp $ - WebWatch application
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "openssl/ssl.h"

#include "core/Core.h"
#include "core/CheckMethod.h"

#include "WebWatch.h"
#include "Common.h"
#include "MainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

CWebWatchApp theApp;

BEGIN_MESSAGE_MAP(CWebWatchApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CWebWatchApp::CWebWatchApp()
{
}

BOOL CWebWatchApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

    SSL_library_init();
    SSL_load_error_strings();

    AfxEnableControlContainer();

    // Register check methods
    Core::CheckMethod::RegisterAllTypes();

    // Make a per-run backup of configuration file
    if (WebWatch::IsFileExistent("WebWatch.xml"))
        WebWatch::MakeBackup("WebWatch.xml", WebWatch::perRun, 0);

    try {
        CMainDlg dlg;
	    m_pMainWnd = &dlg;
	    dlg.DoModal();
    }
    catch (const std::exception & ex) {
        std::string msg = "WebWatch raised an unexpected exception "
                          "and will now terminate: ";
        msg += ex.what();
        MessageBox(NULL, msg.c_str(), "Unexpected exception", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    catch (...) {
        MessageBox(0, "WebWatch raised an unknown exception "
                      "and will now terminate.", "Unknown exception",
                      MB_OK | MB_ICONERROR);
    }

    return TRUE;
}
