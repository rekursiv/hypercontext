#ifndef hctTextWindow_H
#define hctTextWindow_H

#ifndef hctTextManager_H
#include "hctTextManager.h"
#endif

#ifndef hctLinkManager_H
#include "hctLinkManager.h"
#endif

#ifndef hctSelectionManager_H
#include "hctSelectionManager.h"
#endif

#ifndef hctEditManager_H
#include "hctEditManager.h"
#endif

#ifndef hctSearchManager_H
#include "hctSearchManager.h"
#endif

#include <wx/scrolwin.h>

//#ifndef hctGlyphStackerACIP_H
//#include "hctGlyphStackerACIP.h"
//#endif

#include <vector>
#include <list>
using namespace std;

#define CURSOR_BLINK_TIMER 0

enum hctEditMode_t
{
	hctEM_TEXT,
	hctEM_LINK
};

class hctGlyphStackerACIP;

/// A scrollable window for displaying text in multiple languages
class hctTextWindow : public wxScrolledWindow
{

public:
	hctTextWindow(wxWindow *parent, long id);
	virtual ~hctTextWindow();

	/// deletes all text and re-initializes variables
	virtual void clear();

	/// updates text manager
	/** 
	Update obviously needs to be called after loading a file, but
	also on a resize or whenever the layout of the text atoms in
	the window changes.  (e.g. switching Wylie mode)
	*/
	virtual void update();

	//@{@name- these provide access to text and links in the window (used mostly during load/save)
	text_t* getText() {
		return _textManager.getText();
	}

	//@}


	hctLinkManager* getLinkManager() {
		return &_linkManager;
	}
	hctSelectionManager* getSelectionManager() {
		return &_selectionManager;
	}
	hctTextManager* getTextManager() {
		return &_textManager;
	}
	hctGlyphStackerACIP* getGlyphStacker() {
//		return &_glyphStacker;
		return _textManager.getGlyphStacker();
	}
	hctEditManager* getEditManager() {
		return &_editManager;
	}

	void updateAtomUnderMouse();

	wxRect getViewRect();

	void setNeedsUpdate() {
		_updating = true;
		_needsUpdate = true;
	}

	void scrollToFound(bool next);

	void setEditMode(hctEditMode_t mode);
	void toggleEditMode() {
		if (_editMode == hctEM_LINK) setEditMode(hctEM_TEXT);
		else setEditMode(hctEM_LINK);
	}

	void cut();
	void copy();
	void paste();
	void copyLinksFromTemplate();

	void setSearchTerm(wxString term);
	void setSearchLang(hctLanguage_t lang);
	void resetSearch();
	bool doSearch();

	void test();

	void onAtomRefresh(wxRect rect, int index);

protected:

	void linkSelectedText();
	void scrollToCursor();

	//@{@name- event handlers
	void OnPaint(wxPaintEvent &event);
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnKeyChar(wxKeyEvent &event);
	void OnKeyDown(wxKeyEvent &event);
	void OnKeyUp(wxKeyEvent &event);
	void OnMouseMove(wxMouseEvent &event);
	void OnMouseLeftBtnDown(wxMouseEvent &event);
	void OnMouseLeftBtnUp(wxMouseEvent &event);
	void OnMouseLeftBtnDblClk(wxMouseEvent &event);
	void OnMouseRightBtnDown(wxMouseEvent &event);
	void OnMouseRightBtnUp(wxMouseEvent &event);
	void OnMouseRightBtnDblClk(wxMouseEvent &event);
	void OnIdle(wxIdleEvent &event);
	//@}

	void doCommonEditKeys(int key);
	void doLinkEditKeys(int key);

	//@{@name- managers
	hctTextManager _textManager;
	hctEditManager _editManager;
	hctLinkManager _linkManager;
	hctSelectionManager _selectionManager;
	hctSearchManager _searchManager;
//	hctGlyphStackerACIP _glyphStacker;
	//@}

	hctEditMode_t _editMode;

	bool _keyIsDown;
	bool _shiftKeyIsDown;

	int _margin;
	int _spaceBetweenParagraphs;

	int _scrollRate;
	bool _scrolledPage;
	int _prevTopOfWin;   //??

	bool _updating;
	bool _needsUpdate;

	bool _editingNote;
	bool _firstUpdate;

	txtitr_t _atomUnderMouse;
	txtitr_t _atomClosestToMouse;
	wxPoint _mousePos;

	txtitr_t _searchBegin;
	wxString _searchTerm;
	hctLanguage_t _searchLang;
	int _searchNum;
    bool _searchNeedsReset;


	DECLARE_EVENT_TABLE()

};

#endif

