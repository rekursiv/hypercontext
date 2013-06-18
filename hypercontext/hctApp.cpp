// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#include "stdwx.h"

#include "hctApp.h"
#include "hctRootFrame.h"
#include <wx/file.h>
#include <wx/filename.h>

//#ifndef __WXDEBUG__
#define hctLOAD_DICTIONARY
//#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
//#endif

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. hctApp and
// not wxApp)
IMPLEMENT_APP(hctApp)


hctApp::hctApp()
{
}

hctApp::~hctApp()
{
}

// 'Main program' equivalent: the program execution "starts" here
bool hctApp::OnInit()
{
	wxLogDebug("hctApp::OnInit()");
//	wxLogDebug(argv[0]);
//	wxLogDebug("wd1=%s",wxGetWorkingDirectory());

//#ifndef __WXDEBUG__
//#ifdef __APPLE__
    // set "working directory" to Resources folder inside the .app bundle
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);
    
    wxSetWorkingDirectory(path);
    
//    wxFileName here(path);
//	wxSetWorkingDirectory(here.GetPath(wxPATH_GET_VOLUME));

/*
#else
	// make sure our "working directory" is the one we were loaded from
	wxFileName here(argv[0]);
	wxSetWorkingDirectory(here.GetPath(wxPATH_GET_VOLUME));
#endif
#endif
  */
//   	wxLogDebug("wd2=%s",wxGetWorkingDirectory());

    // create the main application window
    _mainFrame = new hctRootFrame(_("Hypercontext"),
                                 wxPoint(50, 50), wxSize(800, 600));

    // and show it (the frames, unlike simple controls, are not shown when created initially)
    _mainFrame->Show(TRUE);
	SetTopWindow(_mainFrame);

	_mainFrame->SetCursor(wxCursor(wxCURSOR_WAIT));
	_mainFrame->init();

	if (argc > 1) getRootWindow()->setFileName(argv[1]);

	_mainFrame->SetCursor(wxCursor(wxCURSOR_ARROW));

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
}

void hctApp::onFirstUpdate() {
	wxLogDebug("hctApp::onFirstUpdate()");

#ifdef __WXMAC__
	// work around wxMAC init bug
	_mainFrame->SetSize(780, 580);
#endif

	getRootWindow()->init();
	getRootWindow()->load();

#ifdef hctLOAD_DICTIONARY
	getRootWindow()->loadDictionary();
#endif

}


hctRootWindow* hctApp::getRootWindow() {
	return _mainFrame->getRootWindow();
}