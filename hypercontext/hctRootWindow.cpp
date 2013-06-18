#include "stdwx.h"

#include "hctRootWindow.h"

//#include "wx/config.h"
#include "wx/clipbrd.h"
#include "hctColors.h"

#include "hctTextWindow.h"
#include "hctParseHyp.h"
#include "hctSaveHyp.h"
#include "hctParseACIP.h"
#include "hctDictTibEng.h"   
#include "hctFontManager.h" 


#ifdef WIN32
	#include <windows.h>
#endif

const long   ID_TEXTWIN   = wxNewId();
const long   ID_DEFWIN   = wxNewId();


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define HCT_DICTIONARY "tibengdict_RY"
//#define HCT_DICTIONARY "tibengdict_THDL"

BEGIN_EVENT_TABLE(hctRootWindow, wxSplitterWindow)
	EVT_SIZE(hctRootWindow::OnSize)
END_EVENT_TABLE()


hctRootWindow::hctRootWindow(wxWindow *parent)
	: wxSplitterWindow(parent, -1) {
	_mainWin = NULL;
	_defWin = NULL;
	_hasBeenEdited = false;
	_autoSearch = true;   //  TODO:  let user toggle this on/off

	_mainWin = new hctTextWindow(this, ID_TEXTWIN);
	_defWin = new wxTextCtrl(this, ID_DEFWIN, "", wxDefaultPosition, wxDefaultSize, 
		wxTE_MULTILINE|wxTE_WORDWRAP|wxTE_READONLY|wxTE_RICH );

	_defWin->SetDefaultStyle(wxTextAttr(*wxBLACK, *wxWHITE, wxFont(12, wxSWISS , wxNORMAL, wxNORMAL)));
	_defWin->SetBackgroundColour(HCT_COLOR_WBG_LIGHT_GRAY);

	SplitHorizontally(_mainWin, _defWin, -108);
//	SplitHorizontally(_mainWin, _defWin, -400);
	SetMinimumPaneSize(40);
//	SetMinimumPaneSize(108);

	_prevWinSize = GetSize();
	_pathRunFrom = wxGetCwd();
}

hctRootWindow::~hctRootWindow() {
	if (_mainWin) delete _mainWin;
	if (_defWin) delete _defWin;
	hctFontManager::release();
}


void hctRootWindow::init() {
	_mainWin->SetFocus();
	if (!_mainWin->getGlyphStacker()->loadKerningTable("ACIP.kt")) {
		wxMessageBox("Cannot load kerning table 'ACIP.kt'\nTibetan will not be displayed properly.",
			"Error initializing Glyph Stacker", wxOK|wxICON_EXCLAMATION, this);
	}
    
	loadTutorial();   //////////////////////////   TEST    /////////////////
    
}

void hctRootWindow::deleteAllLinks() {
	if (wxMessageBox("Really delete all links in this document?", "Delete All Links",
		wxICON_QUESTION|wxYES_NO, this) == wxYES) {
			_mainWin->getLinkManager()->deleteAllLinks();
			_mainWin->Refresh();
			setHasBeenEdited(true);
		}
}

void hctRootWindow::loadTutorial() {
	load(_pathRunFrom+"/tutorial.hyp");
}

bool hctRootWindow::canWrite() {
	return (wxFile().Access(_curFileName, wxFile::write));
}

void hctRootWindow::updateWindowTitle() {
	wxString appName = "Hypercontext";
	wxString title;
	if (_curFileName.IsEmpty()) title = appName;
	else {
		title = _curFileName+"  -  "+appName;
	}
	if (_hasBeenEdited) title += " *";
	((wxTopLevelWindow*)GetParent())->SetTitle(title);
}

void hctRootWindow::newDoc() {
	_curFileName.Empty();
	clear();
	_mainWin->setEditMode(hctEM_TEXT);
}

void hctRootWindow::clear() {
	_mainWin->clear();
	_mainWin->setNeedsUpdate();
	setHasBeenEdited(false);
	updateWindowTitle();
	SetSashPosition(-108, true);   // comment out for wx242

}

void hctRootWindow::load() {
	if (_curFileName.IsEmpty()) {
		newDoc();
		return;
	}
	clear();

	if (!insert(_curFileName)) _curFileName.Empty();

	setHasBeenEdited(false);
	updateWindowTitle();
}


void hctRootWindow::load(wxString fileName) {
	_curFileName = fileName;
	load();
}

bool hctRootWindow::insert(wxString fileName) {
	hctParser *parser;
	wxString extension = fileName.AfterLast('.');
	if (extension.CmpNoCase("ACT") == 0 ||
		extension.CmpNoCase("AET") == 0 ||
		extension.CmpNoCase("INC") == 0 ||
		extension.CmpNoCase("INL") == 0) parser = new hctParseACIP;
	else
		parser = new hctParseHyp;

	if (!parser->parse(fileName, *_mainWin)) {
		_defWin->Clear();
		_defWin->SetBackgroundColour(HCT_COLOR_WBG_DISASTER_RED);
		*_defWin << parser->getLastError();
	}
	bool success = !parser->getLastErrorWasFatal();
	delete parser;
	return success;
}


void hctRootWindow::launchTextEdit() {//////////////////////////   TEST    /////////////////
#ifdef __WXMSW__
	wxExecute("notepad " + _curFileName);
#endif
#ifdef __WXMAC__
//	wxExecute("/Applications/TextEdit.app/Contents/MacOS/TextEdit \"" + _curFileName + "\"");
   	wxExecute("open -t \"" + _curFileName + "\"");
#endif
}

void hctRootWindow::loadDictionary() {
    wxLogDebug(" Loading Dictionary ");
	_defWin->Clear();
	_defWin->SetBackgroundColour(HCT_COLOR_WBG_POSTITNOTE_YELLOW);
	*_defWin << "\n\n                Loading Dictionary...";
	wxSafeYield(this->MacGetTopLevelWindow());  //  TODO:  multiplatformizeme
	_dictTibEng.load(HCT_DICTIONARY);
	clearDefText();
	wxSafeYield(this->MacGetTopLevelWindow());  //  TODO:  multiplatformizeme
}


void hctRootWindow::save() {
	if (_curFileName.IsEmpty()) return;
	wxLogDebug(" document SAVE ");
	hctSaveHyp saver;
	saver.save(_curFileName, *_mainWin);
}

void hctRootWindow::save(wxString fileName) {
	_curFileName = fileName;
	save();
	updateWindowTitle();
}

//			=== dictionary utils ===  
wxString hctRootWindow::ACIP2Wylie(wxString word)
{
	char prevChar = '\0';
	for (size_t i=0; i<word.size(); ++i) {
		// swap case
		if (isupper(word[i])) word[i] = tolower(word[i]);
		else if (islower(word[i])) word[i] = toupper(word[i]);
		if (word[i] == '-') word[i] = '.';  //   convert '-' -> '.'
		else if (prevChar == 't') {
			if (word[i] == 'z') word[i] = 's';  //   convert tza -> tsa
			else if (word[i] == 's') {  //  convert tsa -> tsha
				++i;
				word.insert(i, 'h');
			}
		}
		prevChar = word[i];
	}

	return word;
}

void hctRootWindow::cut() {
	_mainWin->cut();
}
void hctRootWindow::copy() {
	_mainWin->copy();
}
void hctRootWindow::paste() {
	_mainWin->paste();
}
void hctRootWindow::copyLinksFromTemplate() {
	_mainWin->copyLinksFromTemplate();
}


void hctRootWindow::wordToClipboard() {  // TODO:  use "copy" instead???
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(_prevLookup) );
		wxTheClipboard->Close();
	}
	illuminatorLink();
}

void hctRootWindow::illuminatorLink() {
#ifdef WIN32
	HWND hwnd = ::FindWindow("TibetDRO", NULL);
//	wxLogDebug("\n\nhctDictTibEng_JV::lookup()   word=%s   hwnd=%x", _prevLookup.c_str(), hwnd);

	if (hwnd) {
		SendMessage(hwnd, WM_COMMAND, 324, 0);  // go forward through prev (to select lookup editbox)
//		::wxMilliSleep(20);  // use this for wx242
		::wxSleep(20);
		SendMessage(hwnd, WM_COMMAND, 268, 0);  // select all
//		::wxMilliSleep(10);
		::wxSleep(20);
		SendMessage(hwnd, WM_COMMAND, 267, 0);  // delete
//		::wxMilliSleep(10);
		::wxSleep(20);
		SendMessage(hwnd, WM_COMMAND, 264, 0);  // paste
	}
#endif
}


hctDictFindInfo hctRootWindow::dictFind(wxString word) {
	hctDictFindInfo info = _dictTibEng.find(ACIP2Wylie(word).ToStdString());
/*
	wxString temp;
	temp.Clear();
	if (info.found) temp << "found";
	if (info.exact) temp << " | exact";
	if (info.unique) temp << " | unique";
	temp << "     r=" << info.remainder;
	wxLogDebug(temp);
	wxLogDebug("");
*/
	return info;
}



//			=== definition and note display ===  
void hctRootWindow::define(wxString word, bool isLink) {
	wxString prettyDef;
	_defWin->Clear();
	if (isLink)	_defWin->SetBackgroundColour(HCT_COLOR_WBG_COOL_CYAN);
	else _defWin->SetBackgroundColour(HCT_COLOR_WBG_PASTEL_PURPLE);

	if (word.size() == 0) return;

	if (_autoSearch) {
		int found = _mainWin->getLinkManager()->searchLinks(word);
		if (found > 0) {
			prettyDef << "(" << found << ") ";
		}
	}

	_prevLookup = ACIP2Wylie(word);
	wxString def = _dictTibEng.define(_prevLookup.ToStdString()).c_str();
	for (size_t i=0; i < def.size(); ++i) {
		if (def[i] == '\t') {
			prettyDef << "      >> ";
		}
		else prettyDef << def[i];
	}
	*_defWin << prettyDef;
}

void hctRootWindow::clearDefText() {
	_defWin->SetBackgroundColour(HCT_COLOR_WBG_LIGHT_GRAY);
	_defWin->Clear();
}


wxString hctRootWindow::getNoteText() {
	return _defWin->GetValue();
}

void hctRootWindow::setNoteText(wxString note) {
	_defWin->Clear();
	_defWin->SetBackgroundColour(HCT_COLOR_WBG_POSTITNOTE_YELLOW);
	*_defWin << note;
}

void hctRootWindow::beginNoteEdit() {
	_defWin->SetBackgroundColour(HCT_COLOR_WBG_BANDAID_PINK);
	_defWin->SetEditable(true);
	_defWin->SetFocus();
}

void hctRootWindow::endNoteEdit() {
	_defWin->SetBackgroundColour(HCT_COLOR_WBG_POSTITNOTE_YELLOW);
	_defWin->SetEditable(false);
}
//   ^^^ (end definition and note display) ^^^


//			=== event handlers  ===   
void hctRootWindow::OnSize(wxSizeEvent& event) {
	int newSashPos = GetSashPosition() + event.GetSize().GetHeight() - _prevWinSize.GetHeight();
	SetSashPosition(newSashPos, FALSE);
	_prevWinSize = GetSize();

	event.Skip();
}
//  ^^^  (end event handlers)  ^^^

