#ifndef hypercontextFrame_H
#define hypercontextFrame_H

#include <wx/dnd.h>
#include "hctRootWindow.h"

class wxTextCtrl;
class wxButton;
class wxChoice;

class hctFileDropTarget : public wxFileDropTarget
{
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
};

/// Define a new frame type: this is going to be our main frame
class hctRootFrame : public wxFrame
{
public:
    hctRootFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	virtual ~hctRootFrame();

	//@{@name- event handlers
	/// menu commands
    void OnMenuFileNew(wxCommandEvent& event);
	void OnMenuFileOpen(wxCommandEvent& event);
	void OnMenuFileInsert(wxCommandEvent& event);
	void OnMenuFileReopen(wxCommandEvent& event);
    void OnMenuFileSave(wxCommandEvent& event);
    void OnMenuFileBackupSave(wxCommandEvent& event);
    void OnMenuFileSaveAs(wxCommandEvent& event);
    void OnMenuFileQuit(wxCommandEvent& event);
	void OnMenuEditLaunchTextEdit(wxCommandEvent& event);
	void OnMenuEditDeleteAllLinks(wxCommandEvent& event);
	void OnMenuEditCopyLinksFromTemplate(wxCommandEvent& event);
	void OnMenuEditCut(wxCommandEvent& event);
	void OnMenuEditCopy(wxCommandEvent& event);
	void OnMenuEditPaste(wxCommandEvent& event);
    void OnMenuHelpAbout(wxCommandEvent& event);
    void OnMenuHelpLoadTutorial(wxCommandEvent& event);
	void OnMenuHelpShow(wxCommandEvent& event);
	void OnMenuHelpTest(wxCommandEvent& event);

	/// other
	void OnCloseWindow(wxCloseEvent& event);

	void onToolbarSearchBtn(wxCommandEvent& event);
	void onToolbarSearchTxbUpdate(wxCommandEvent& event);
	void onToolbarSearchTxbEnter(wxCommandEvent& event);
	void onToolbarSearchLangSelect(wxCommandEvent& event);
	//@}

	void init();

	hctRootWindow* getRootWindow() {
		return _rootWin;
	}

	bool loadFile(wxString fname);
	bool insertFile(wxString fname);




private:
	bool warnUserAboutLoosingChanges();
	hctRootWindow *_rootWin;

	// // //
	wxTextCtrl *_txc_search;
	wxButton *_btn_search;
	wxChoice *_chc_lang;
	// // //

    DECLARE_CLASS(hctRootFrame)
    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()
};

#endif
