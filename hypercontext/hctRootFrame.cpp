// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#include "stdwx.h"

#include <wx/image.h>

#include "hctApp.h"
#include "hctRootFrame.h"
#include "hctTextWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------
// the application icon
#include "hypercontext.xpm"

// toolbar icons
#include "open.xpm"
#include "reopen.xpm"
#include "save.xpm"
#include "edit.xpm"


// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    hctMenuID_file_new = 1,
	hctMenuID_file_open,
	hctMenuID_file_insert,
    hctMenuID_file_reopen,
    hctMenuID_file_save,
    hctMenuID_file_backup_save,
    hctMenuID_file_save_as,
    hctMenuID_file_quit,
	hctMenuID_edit_launch_text_edit,
	hctMenuID_edit_delete_all_links,
	hctMenuID_edit_copy_links_from_template,
	hctMenuID_edit_cut,
	hctMenuID_edit_copy,
	hctMenuID_edit_paste,
	hctMenuID_help_show,
    hctMenuID_help_load_tutorial,
    hctMenuID_help_about,
	hctMenuID_help_test,

	// search UI
	hctToolbarID_search_btn,
	hctToolbarID_search_txb,
	hctToolbarID_search_lang
};

// ----------------------------------------------------------------------------
// event tables and other macros for wxWindows
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
IMPLEMENT_CLASS(hctRootFrame, wxFrame)
BEGIN_EVENT_TABLE(hctRootFrame, wxFrame)    
	EVT_MENU(hctMenuID_file_new,  hctRootFrame::OnMenuFileNew)
	EVT_MENU(hctMenuID_file_open,  hctRootFrame::OnMenuFileOpen)
	EVT_MENU(hctMenuID_file_insert,  hctRootFrame::OnMenuFileInsert)
	EVT_MENU(hctMenuID_file_reopen,  hctRootFrame::OnMenuFileReopen)
	EVT_MENU(hctMenuID_file_save,  hctRootFrame::OnMenuFileSave)
	EVT_MENU(hctMenuID_file_backup_save,  hctRootFrame::OnMenuFileBackupSave)
	EVT_MENU(hctMenuID_file_save_as,  hctRootFrame::OnMenuFileSaveAs)
    EVT_MENU(hctMenuID_file_quit,  hctRootFrame::OnMenuFileQuit)
    EVT_MENU(hctMenuID_edit_launch_text_edit,  hctRootFrame::OnMenuEditLaunchTextEdit)
	EVT_MENU(hctMenuID_edit_delete_all_links, hctRootFrame::OnMenuEditDeleteAllLinks)
	EVT_MENU(hctMenuID_edit_copy_links_from_template, hctRootFrame::OnMenuEditCopyLinksFromTemplate)
	EVT_MENU(hctMenuID_edit_cut, hctRootFrame::OnMenuEditCut)
	EVT_MENU(hctMenuID_edit_copy, hctRootFrame::OnMenuEditCopy)
	EVT_MENU(hctMenuID_edit_paste, hctRootFrame::OnMenuEditPaste)
    EVT_MENU(hctMenuID_help_show, hctRootFrame::OnMenuHelpShow)
	EVT_MENU(hctMenuID_help_load_tutorial,  hctRootFrame::OnMenuHelpLoadTutorial)
    EVT_MENU(hctMenuID_help_about, hctRootFrame::OnMenuHelpAbout)
    EVT_MENU(hctMenuID_help_test, hctRootFrame::OnMenuHelpTest)
	EVT_CLOSE(hctRootFrame::OnCloseWindow)
	EVT_BUTTON(hctToolbarID_search_btn, hctRootFrame::onToolbarSearchBtn)
	EVT_TEXT(hctToolbarID_search_txb, hctRootFrame::onToolbarSearchTxbUpdate)
	EVT_TEXT_ENTER(hctToolbarID_search_txb, hctRootFrame::onToolbarSearchTxbEnter)
	EVT_CHOICE(hctToolbarID_search_lang, hctRootFrame::onToolbarSearchLangSelect)

END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
hctRootFrame::hctRootFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
       : wxFrame((wxFrame *)NULL, -1, title, pos, size, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	_rootWin = NULL;

    // set the frame icon
    SetIcon(wxICON(hypercontext));

#ifdef __WXMAC__
	wxApp::s_macAboutMenuItemId = hctMenuID_help_about;
#endif 

    // create menu bars
    wxMenu *fileMenu = new wxMenu("", wxMENU_TEAROFF);
    wxMenu *editMenu = new wxMenu("", wxMENU_TEAROFF);
    wxMenu *helpMenu = new wxMenu("", wxMENU_TEAROFF);

    fileMenu->Append(hctMenuID_file_new, _("New\tCtrl-N"), _("Create a new document"));
    fileMenu->Append(hctMenuID_file_open, _("Open...\tCtrl-O"), _("Open a file"));
    fileMenu->Append(hctMenuID_file_insert, _("Insert...\tCtrl-I"), _("Insert a file into current document"));
	fileMenu->Append(hctMenuID_file_reopen, _("Reopen\tCtrl-R"), _("Re-open last file opened or saved"));
	fileMenu->AppendSeparator();
    fileMenu->Append(hctMenuID_file_save, _("Save\tCtrl-S"), _("Save current file"));
//    fileMenu->Append(hctMenuID_file_backup_save, _("Backup, Save\tCtrl-B"), 
//		_("Create a backup of the last file saved, then save current file"));
	fileMenu->Append(hctMenuID_file_save_as, _("Save As..."), _("Save current file to specific location"));
	fileMenu->AppendSeparator();
    fileMenu->Append(hctMenuID_file_quit, _("Exit\tAlt-F4"), _("Quit Hypercontext"));

//	editMenu->Append(hctMenuID_edit_cut, _("Cut (NYI)\tCtrl-X"), _("Move text to clipboard (not yet implemented)"));
	editMenu->Append(hctMenuID_edit_copy, _("Copy\tCtrl-C"), _("Copy text to clipboard"));
//	editMenu->Append(hctMenuID_edit_paste, _("Paste (NYI)\tCtrl-V"), _("Copy text into document (not yet implemented)"));
	editMenu->AppendSeparator();
	editMenu->Append(hctMenuID_edit_launch_text_edit, _("Open in External Editor\tCtrl-E"), 
		_("Open currently loaded file in an external text editor"));
	editMenu->Append(hctMenuID_edit_delete_all_links, _("Delete All Links"), 
		_("Delete all the links in this document"));
	editMenu->Append(hctMenuID_edit_copy_links_from_template, _("Copy Links From Template"), 
		_("Uses currently selected text as template"));
	helpMenu->Append(hctMenuID_help_show, _("Show Help\tF1"), _("Basic help using program"));
    helpMenu->Append(hctMenuID_help_load_tutorial, _("Load Tutorial"), _("Basic help using program"));
#ifndef __WXMAC__	
	helpMenu->AppendSeparator();
#endif	
	helpMenu->Append(hctMenuID_help_about, _("About"), _("About Hypercontext"));
#ifdef __WXDEBUG__
	helpMenu->AppendSeparator();
	helpMenu->Append(hctMenuID_help_test, _("Test\tF9"), _("test"));
#endif

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, _("&File"));
    menuBar->Append(editMenu, _("&Edit"));
    menuBar->Append(helpMenu, _("&Help"));
    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);

    // create the toolbar and add tools
    wxToolBar* toolbar = CreateToolBar();
    toolbar->AddTool(hctMenuID_file_open, _("Open"), wxBitmap(open_xpm), _("Open a file"));
	toolbar->AddTool(hctMenuID_file_save, _("Save"), wxBitmap(save_xpm), _("Save current file"));
	toolbar->AddSeparator();
    toolbar->AddTool(hctMenuID_edit_launch_text_edit, _("Edit"), wxBitmap(edit_xpm),
		_("Open current file in text editor"));
	toolbar->AddTool(hctMenuID_file_reopen, _("Reopen"), wxBitmap(reopen_xpm),
		_("Re-open last file opened or saved"));



	// // // search tool

	toolbar->AddSeparator();

	wxString lang_choices[2];
	lang_choices[0]="Tibetan";
	lang_choices[1]="English";
	_chc_lang = new wxChoice(toolbar, hctToolbarID_search_lang, wxPoint(-1, -1), wxSize(-1, -1), 2, lang_choices);
	_chc_lang->SetSelection(0);
	toolbar->AddControl(_chc_lang);

	_txc_search = new wxTextCtrl(toolbar, hctToolbarID_search_txb, wxEmptyString, wxDefaultPosition, wxSize(300, wxDefaultSize.GetHeight()), wxTE_PROCESS_ENTER);
	toolbar->AddControl(_txc_search);

	_btn_search = new wxButton(toolbar, hctToolbarID_search_btn, "Search");
	toolbar->AddControl(_btn_search);

	// // // (end search tool)

	toolbar->Realize();

    CreateStatusBar(3);

//	OnMenuHelpShow(wxCommandEvent());

}

// frame destructor
hctRootFrame::~hctRootFrame()
{
	if (_rootWin) delete _rootWin;
}

void hctRootFrame::init() {
	_rootWin = new hctRootWindow(this);
	SetDropTarget(new hctFileDropTarget);
}

// returns "true" if user wants to cancel (hit "Cancel" btn)
bool hctRootFrame::warnUserAboutLoosingChanges() {
	if (_rootWin->getHasBeenEdited())
	{
		int userChoice = wxMessageBox("It looks like you have edited the current file.\nWould you like to save your changes?\n",
			"Last chance to save...", wxYES_DEFAULT| wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
		if (userChoice == wxYES){
//			OnMenuFileSave(wxCommandEvent());  // FIXME:  GCC doesn't like this...
			wxCommandEvent e;
			OnMenuFileSave(e);
		}
		else if (userChoice == wxCANCEL) return true;
	}
	return false;
}

//  menu commands
void hctRootFrame::OnMenuFileNew(wxCommandEvent& event)
{
	if (warnUserAboutLoosingChanges()) return;
	_rootWin->newDoc();
}
void hctRootFrame::OnMenuFileOpen(wxCommandEvent& event)
{
	if (warnUserAboutLoosingChanges()) return;
	wxFileDialog fileDlg(this, "Choose a file to open...", "", "",
//		"Hyperlink Files (*.hyp;*.txt)|*.hyp;*.txt|ACIP Texts (*.act;*.inc)|*.act;*.inc|All Files (*.*)|*.*",
//		"ACIP Texts (*.act;*.inc)|*.act;*.inc|Hyperlink Files (*.hyp;*.txt)|*.hyp;*.txt|All Files (*.*)|*.*",
		"All Files (*.*)|*.*|ACIP Texts (*.act;*.inc)|*.act;*.inc|Hyperlink Files (*.hyp;*.txt)|*.hyp;*.txt",
		wxFD_OPEN|wxFD_CHANGE_DIR);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		SetCursor(wxCursor(wxCURSOR_WAIT));
		_rootWin->load(fileDlg.GetPath());
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void hctRootFrame::OnMenuFileInsert(wxCommandEvent& event)
{
	wxFileDialog fileDlg(this, "Choose a file to insert...", "", "",
		//		"Hyperlink Files (*.hyp;*.txt)|*.hyp;*.txt|ACIP Texts (*.act;*.inc)|*.act;*.inc|All Files (*.*)|*.*",
		//		"ACIP Texts (*.act;*.inc)|*.act;*.inc|Hyperlink Files (*.hyp;*.txt)|*.hyp;*.txt|All Files (*.*)|*.*",
		"All Files (*.*)|*.*|ACIP Texts (*.act;*.inc)|*.act;*.inc|Hyperlink Files (*.hyp;*.txt)|*.hyp;*.txt",
		wxFD_OPEN|wxFD_CHANGE_DIR);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		SetCursor(wxCursor(wxCURSOR_WAIT));
		_rootWin->insert(fileDlg.GetPath());
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void hctRootFrame::OnMenuFileReopen(wxCommandEvent& event) {
	if (!_rootWin->getHasFile()) {
		return;
	}
	if (_rootWin->getHasBeenEdited())
	{
		int userChoice = wxMessageBox("It looks like you have edited the current file.\nDo you really want to loose all these changes and reload your last saved file?\n",
			"Really revert to last saved?", wxNO_DEFAULT| wxYES_NO | wxICON_QUESTION, this);
		if (userChoice == wxNO) return;
	}
	SetCursor(wxCursor(wxCURSOR_WAIT));
	_rootWin->load();
	SetCursor(wxCursor(wxCURSOR_ARROW));
}

void hctRootFrame::OnMenuFileSave(wxCommandEvent& event)
{
	if (!_rootWin->getHasFile() || !_rootWin->canWrite()) {
		OnMenuFileSaveAs(event);
		return;
	}
	SetCursor(wxCursor(wxCURSOR_WAIT));
	_rootWin->save();
	SetCursor(wxCursor(wxCURSOR_ARROW));
}

void hctRootFrame::OnMenuFileBackupSave(wxCommandEvent& event)
{
}

void hctRootFrame::OnMenuFileSaveAs(wxCommandEvent& event)
{
	wxFileDialog fileDlg(this, "Save as...", "", "", "*.hyp", wxFD_SAVE|wxFD_CHANGE_DIR|wxFD_OVERWRITE_PROMPT);
	if (fileDlg.ShowModal() == wxID_OK)
	{
		SetCursor(wxCursor(wxCURSOR_WAIT));
		_rootWin->save(fileDlg.GetPath());
		SetCursor(wxCursor(wxCURSOR_ARROW));
	}
}

void hctRootFrame::OnMenuFileQuit(wxCommandEvent& event)
{
	if (warnUserAboutLoosingChanges()) return;
    // TRUE is to force the frame to close
    Close(TRUE);
}
void hctRootFrame::OnMenuHelpLoadTutorial(wxCommandEvent& event)
{
	if (_rootWin->getHasBeenEdited())
	{
		int userChoice = wxMessageBox("It looks like you have edited the current file.\nDo you really want to loose all these changes and load the tutorial?\n",
			"Really loose all changes?", wxNO_DEFAULT| wxYES_NO | wxICON_QUESTION, this);
		if (userChoice == wxNO) return;
	}
	SetCursor(wxCursor(wxCURSOR_WAIT));
	_rootWin->loadTutorial();
	SetCursor(wxCursor(wxCURSOR_ARROW));
}

void hctRootFrame::OnMenuHelpAbout(wxCommandEvent& event)
{
	wxWindow *win = FindWindow("aboutWindow"); 
	if (win) {
		win->Show();  // if "about" window already exists, show it
		return;
	}

	wxDialog* dlg = new wxDialog(this, -1, (wxString)"Hypercontext Version 0.9", 
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, "aboutWindow");
	wxTextCtrl* txt = new wxTextCtrl(dlg, -1, "", wxDefaultPosition, wxSize(300, 80), 
		wxTE_MULTILINE|wxTE_WORDWRAP|wxTE_READONLY|wxTE_RICH );
	txt->SetBackgroundColour(wxColor(162, 220, 234));  // sky blue

	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(10, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << " Many thanks to Dan Stivers for donating\n";
	*txt << " a Mac Mini to get Hypercontext updated\n";
  	*txt << " and working on modern Macs!\n";

	dlg->Fit();
	dlg->Center();
	dlg->Show();
}

void hctRootFrame::OnMenuHelpShow(wxCommandEvent& event)
{
	wxWindow *win = FindWindow("helpWindow"); 
	if (win) {
		win->Show();  // if help window already exists, show it
		return;
	}
#ifdef __WXMAC__
	wxString ctrlKey = "<Command>";
#else
	wxString ctrlKey = "<Ctrl>";
#endif

	wxDialog* dlg = new wxDialog(this, -1, (wxString)"Hypercontext Help", 
		wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, "helpWindow");
	wxTextCtrl* txt = new wxTextCtrl(dlg, -1, "", wxDefaultPosition, wxSize(300, 560), 
		wxTE_MULTILINE|wxTE_WORDWRAP|wxTE_READONLY|wxTE_RICH );
	txt->SetBackgroundColour(wxColor(162, 220, 234));  // sky blue

	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(12, wxSWISS , wxNORMAL, wxBOLD)));
	*txt << "Keyboard Map:";
	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(10, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << "   (for \"Link Edit\" mode)\n";
	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(12, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << "  F1 - Help\n";
	*txt << "  F3 - Search\n";
	*txt << "  F4 - Copy word to clipboard\n";
	*txt << "  F6 - Toggle Wylie/Native display mode\n";
	*txt << "  F8 - Toggle Link/Text edit mode\n";
	*txt << "  Esc - Cancel selection\n";
	*txt << "  Spacebar - Link selected text\n";
	*txt << "  X - Unlink focused text\n";
	*txt << "  D - Delete focused link\n";
	*txt << "  Enter - Cycle Tibetan words\n";
	*txt << "  Left/Right Arrows - Prev/Next word\n";

	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(12, wxSWISS , wxNORMAL, wxBOLD)));
	*txt << "\nUsing the Mouse:\n";
	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(12, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << "Left Button: Select text  --  ";
	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(10, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << "Click to select words, click-and-drag-outside to select areas. ";
	*txt << "Multiple-selection is enabled by default across different languages. ";
	*txt << "Hold down "<< ctrlKey << " and click to select multiple words in the same language. ";
	*txt << "Click on non-text area to deselect all.  ";
	*txt << "Tibetan syllables are combined into the longest word possible and defined in the window ";
	*txt << "below.  If \"subwords\" are also found within this selection, black vertical lines will ";
	*txt << "separate each word, and clicking on the far left syllable will step through each one.\n";
	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(12, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << "Right Button: Link/Unlink text --  ";
	txt->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(10, wxSWISS , wxNORMAL, wxNORMAL)));
	*txt << "Right-clicking (anywhere in main window) links together all selected text. Right-clicking on any ";
	*txt << "focused text unlinks all text within that link, leaving it selected.\n";

	dlg->Fit();
	dlg->Center();
	dlg->Show();
}

void hctRootFrame::OnMenuEditLaunchTextEdit(wxCommandEvent& event) {
	if (_rootWin->getHasBeenEdited() && _rootWin->canWrite()) OnMenuFileSave(event);
	_rootWin->launchTextEdit();
}

void hctRootFrame::OnMenuEditDeleteAllLinks(wxCommandEvent& event) {
	_rootWin->deleteAllLinks();
}

void hctRootFrame::OnMenuEditCut(wxCommandEvent& event) {
	_rootWin->cut();
}

void hctRootFrame::OnMenuEditCopy(wxCommandEvent& event) {
	_rootWin->copy();
}

void hctRootFrame::OnMenuEditPaste(wxCommandEvent& event) {
	_rootWin->paste();
}

void hctRootFrame::OnMenuEditCopyLinksFromTemplate(wxCommandEvent& event) {
	_rootWin->copyLinksFromTemplate();
}

// end menu commands

void hctRootFrame::OnCloseWindow(wxCloseEvent& event) {
	if (event.CanVeto()) {
		if (warnUserAboutLoosingChanges()) {
			event.Veto();
			return;
		}
	}
	Destroy();
}

bool hctRootFrame::loadFile(wxString fname) {
	if (warnUserAboutLoosingChanges()) return false;
	SetCursor(wxCursor(wxCURSOR_WAIT));
	_rootWin->load(fname);
	SetCursor(wxCursor(wxCURSOR_ARROW));
	return true;
}

bool hctRootFrame::insertFile(wxString fname) {
	SetCursor(wxCursor(wxCURSOR_WAIT));
	bool success = _rootWin->insert(fname);
	SetCursor(wxCursor(wxCURSOR_ARROW));
	return success;
}

bool hctFileDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
	int numFiles = filenames.GetCount();
	if (numFiles == 0) return false;
	if (!((hctRootFrame *)(wxGetApp().GetTopWindow()))->loadFile(filenames[0])) return false;
	for (int i=1; i<numFiles; ++i) {
		if (!((hctRootFrame *)(wxGetApp().GetTopWindow()))->insertFile(filenames[i])) return false;
	}
    return true;   //  TESTME
}

void hctRootFrame::OnMenuHelpTest(wxCommandEvent& event) {
	_rootWin->getMainWin()->test();
}

void hctRootFrame::onToolbarSearchBtn(wxCommandEvent& event) {
	wxLogDebug("hctRootFrame::onToolbarSearchBtn()");
	_rootWin->getMainWin()->doSearch();
}

void hctRootFrame::onToolbarSearchTxbUpdate(wxCommandEvent& event) {
	wxLogDebug("hctRootFrame::onToolbarSearchTxbUpdate()");
	_rootWin->getMainWin()->setSearchTerm(_txc_search->GetValue());
	_rootWin->getMainWin()->resetSearch();
}

void hctRootFrame::onToolbarSearchTxbEnter(wxCommandEvent& event) {
	wxLogDebug("hctRootFrame::onToolbarSearchTxbEnter()");
	_rootWin->getMainWin()->doSearch();
}

void hctRootFrame::onToolbarSearchLangSelect(wxCommandEvent& event) {
	wxLogDebug("hctRootFrame::onToolbarSearchLangSelect()    %i", event.GetSelection());
	_rootWin->getMainWin()->resetSearch();
	int sel = event.GetSelection();
	if (sel==0) _rootWin->getMainWin()->setSearchLang(hctLANG_TIBETAN);
	else _rootWin->getMainWin()->setSearchLang(hctLANG_ENGLISH);
}

