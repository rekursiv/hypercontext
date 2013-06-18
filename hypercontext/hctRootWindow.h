#ifndef hctDocument_H
#define hctDocument_H

#include <wx/splitter.h>

class hctTextWindow;

#include "hctDictTibEng.h"

///  Contains everything needed to work with hypertexted files
/**
	The RootWindow class wraps around the main text window and holds any
	utility classes (parser, saver, dictionary, etc.) that may be needed.
*/
class hctRootWindow : public wxSplitterWindow
{
public:
	hctRootWindow(wxWindow *parent);
	~hctRootWindow();

	void init();
	void newDoc();
	void clear();
	void load();
	void load(wxString fileName);
	bool insert(wxString fileName);
	void save();
	void save(wxString fileName);
	/// returns true if user has changed the document
	bool getHasBeenEdited() {
		return _hasBeenEdited;
	}
	void setHasBeenEdited(bool hbe) {
		if (hbe != _hasBeenEdited) {
			_hasBeenEdited = hbe;
			updateWindowTitle();
		}
	}

	bool getHasFile() {
		return !_curFileName.IsEmpty();
	}

	void setFileName(wxString fileName) {
		_curFileName = fileName;
	}

	void loadDictionary();

	/// lookup a text in the dictionary, display in "definition window."
	void define(wxString word, bool isLink = false);

	void cut();
	void copy();
	void paste();
	void copyLinksFromTemplate();

	wxString ACIP2Wylie(wxString word);
	void illuminatorLink();
	void wordToClipboard();
	hctDictFindInfo dictFind(wxString word);
//	hctDictTibEng* getTibEngDict() {
//		return &_dictTibEng;
//	}

	/// clear text in definition window, reset color
	void clearDefText();

	/// get the text for user notes
	wxString getNoteText();

	/// set the text for user notes
	/**
		This is currently uses the same window as the dictionary
		definitions (_defWin) which should probably change someday...
	*/
	void setNoteText(wxString note);

	/// lets the user edit the note
	void beginNoteEdit();

	/// called when the user is done editing
	void endNoteEdit();

	void launchTextEdit();
	void updateWindowTitle();
	bool canWrite();
	void loadTutorial();
	void deleteAllLinks();

	hctTextWindow *getMainWin() {
		return _mainWin;
	}

private:
	void OnSize(wxSizeEvent& event);

	bool _hasBeenEdited;
	bool _autoSearch;
	hctTextWindow *_mainWin;
	wxTextCtrl *_defWin;

	wxString _prevLookup;
	wxString _curFileName;
	wxSize _prevWinSize;

	hctDictTibEng _dictTibEng;

	wxString _pathRunFrom;

	DECLARE_EVENT_TABLE()
};

#endif