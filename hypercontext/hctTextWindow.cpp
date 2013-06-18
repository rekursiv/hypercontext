#include "stdwx.h"

#include <wx/dcclient.h>

#include "hctTextWindow.h"
#include "hctHyperlink.h"
#include "hctTextBlock_link.h"
#include "hctApp.h"
#include "hctRootFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_EVENT_TABLE(hctTextWindow, wxScrolledWindow)
    EVT_PAINT  (hctTextWindow::OnPaint)
	EVT_SIZE(hctTextWindow::OnSize)
	EVT_SCROLLWIN(hctTextWindow::OnScroll)
	EVT_CHAR(hctTextWindow::OnKeyChar)
	EVT_KEY_DOWN(hctTextWindow::OnKeyDown)
	EVT_KEY_UP(hctTextWindow::OnKeyUp)
    EVT_MOTION (hctTextWindow::OnMouseMove)
	EVT_LEFT_DOWN (hctTextWindow::OnMouseLeftBtnDown)
	EVT_LEFT_DCLICK (hctTextWindow::OnMouseLeftBtnDblClk)
	EVT_LEFT_UP (hctTextWindow::OnMouseLeftBtnUp)
	EVT_RIGHT_DOWN (hctTextWindow::OnMouseRightBtnDown)
	EVT_RIGHT_DCLICK (hctTextWindow::OnMouseRightBtnDblClk)
	EVT_RIGHT_UP (hctTextWindow::OnMouseRightBtnUp)
	EVT_IDLE (hctTextWindow::OnIdle)
END_EVENT_TABLE()


hctTextWindow::hctTextWindow(wxWindow *parent, long id)
	: wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize,
                       wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)  //   | wxWANTS_CHARS)
	,_textManager(this)
	,_editManager(this)
	,_linkManager(_textManager.getText())
	,_selectionManager(_textManager.getText())
	,_searchManager(_textManager.getText())
{

	_editMode = hctEM_LINK;

	_atomClosestToMouse = _atomUnderMouse = getText()->end();

	_updating = false;
	_scrolledPage = false;
	_prevTopOfWin = 0;
	_editingNote = false;
	_scrollRate = 20;
	_keyIsDown = false;
	_shiftKeyIsDown = false;
	_firstUpdate = true;

//	_searchBegin = NULL;
    _searchNeedsReset = true;
	_searchLang = hctLANG_TIBETAN;
	_searchNum = 0;
    
//    wxLog::SetVerbose(true);   //   TEST
//    wxLog *logger=new wxLogStream(&cout);
//    wxLog::SetActiveTarget(logger);
}



hctTextWindow::~hctTextWindow() {
}



void hctTextWindow::clear() {
	_selectionManager.clear();
	_editManager.hideCursor();
	_linkManager.deleteAllLinks();
	_textManager.clear();
//	_searchBegin = NULL;
    _searchNeedsReset = true;
	wxLogDebug("* CLEAR *");
//	_scrollRate = 20;
//	_keyIsDown = false;
//	_shiftKeyIsDown = false;
}

void hctTextWindow::update() {
	_needsUpdate = false;
	int clientWidth, clientHeight;
	GetClientSize(&clientWidth, &clientHeight);
	int txtHeight = _textManager.arrangeText(clientWidth);
	SetVirtualSize(clientWidth, txtHeight);
	SetScrollRate(0, _scrollRate);
	//  FIXME:  I think this used to nail down the position of the text on resize
	//	if (_visTxtRange_start < _text.size()) Scroll(0, _text[_visTxtRange_start].getRect().GetTop()/_scrollRate);
	_textManager.calcVisibleTextRange(getViewRect());
	updateAtomUnderMouse();
	_updating = false;
	Refresh();
}

wxRect hctTextWindow::getViewRect() {
	wxRect rect;
	int xs, ys, cw, ch, top, bottom, left, right;
	GetViewStart(&xs, &ys);
	top = ys*_scrollRate;
	left = xs*_scrollRate;
	GetClientSize(&cw, &ch);
	bottom = top+ch;
	right = left+cw;
	rect.SetLeft(left);
	rect.SetTop(top);
	rect.SetBottom(bottom);
	rect.SetRight(right);
	return rect;
}


void hctTextWindow::onAtomRefresh(wxRect rect, int index) {
	wxRect dcrect = rect;
	CalcScrolledPosition(rect.x, rect.y, &dcrect.x, &dcrect.y);
	RefreshRect(dcrect);
}

void hctTextWindow::setEditMode(hctEditMode_t mode) {
	_editMode = mode;
	if (_editMode == hctEM_LINK) {
		((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText("Link Edit Mode", 1);
		this->SetCursor(wxCursor(wxCURSOR_ARROW));
		_editManager.hideCursor();
	}
	else if (_editMode == hctEM_TEXT) {
		((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText("Text Edit Mode", 1);
		this->SetCursor(wxCursor(wxCURSOR_IBEAM));
		_selectionManager.clear();
		_editManager.showCursor();
	}
}

void hctTextWindow::cut() {

}

void hctTextWindow::copy() {
	_selectionManager.copy();
}

void hctTextWindow::paste() {
}

// // // search

void hctTextWindow::resetSearch() {
	_searchBegin = _textManager.getTopVisibleAtom();
	_searchNum = 0;
	((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText("", 2);
    _searchNeedsReset = false;
	wxLogDebug("resetSearch");
}

void hctTextWindow::setSearchLang(hctLanguage_t lang) {
	wxLogDebug("lang=%i", lang);
	_searchLang = lang;
}

void hctTextWindow::setSearchTerm(wxString term) {
	_searchTerm = term;
	wxLogDebug("set search term to: '%s'", _searchTerm.c_str());
}

bool hctTextWindow::doSearch() {
	if (_searchNeedsReset) resetSearch();
	if (_searchTerm.IsNull()) return false;
	wxLogDebug("finding '%s'", _searchTerm.c_str());

	bool found = false;
	_selectionManager.clear();
	_searchManager.setLanguage(_searchLang);
	_searchManager.setRangeToAll();
	_searchManager.setRangeBegin(_searchBegin);
	_searchManager.setText(_searchTerm);
	_searchManager.setCaseSensitive(false);
	if (_searchManager.find()) {
		found = true;
		++_searchNum;
		txtitr_t begin = _searchManager.getFoundBegin();
		txtitr_t end = _searchManager.getFoundEnd();
		_searchBegin = end;
		wxLogDebug("found   %i  %i", begin->getIndex(), end->getIndex());
		hctTextBlock_link *sel = new hctTextBlock_link(_textManager.getText());
		sel->setRange(begin, --end);
		_selectionManager.addSelector(sel);

		// scroll to found text
		_editManager.setCursor(begin, true, false, true);
		scrollToCursor();
	}

	wxString status;
	if (found) status.Printf("Found '%s' #%i", _searchTerm.c_str(), _searchNum);
	else status.Printf("'%s' not found", _searchTerm.c_str());
	((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText(status, 2);
	return found;
}
// // // (end search)


void hctTextWindow::copyLinksFromTemplate() {
   // gather list of all links in selected region
	txtitr_t srcBegin, srcEnd;
	_selectionManager.getRange(srcBegin, srcEnd);
//	wxLogDebug("s=%i  e=%i", srcBegin->getIndex(), srcEnd->getIndex());
	if (srcBegin == srcEnd || srcBegin == _textManager.getText()->end()) return;

	wxString tibText;
	std::set<hctHyperlink*> linkset;
	for (txtitr_t pos = srcBegin; pos != srcEnd; ++pos) {
		tibText << pos->getLetters();
		hctTextBlock_link *block = pos->getLinkBlock();
		while (block) {
			linkset.insert(block->getLink());
			block =  block->getNext();
		}
	}

	// pass #1:  get range of English text linked to the selected Tibetan
	txtitr_t minPos = srcBegin;
	txtitr_t maxPos = getText()->end();
	--maxPos;
	int minIdx=maxPos->getIndex();
	int maxIdx=0;
	for (std::set<hctHyperlink*>::iterator pos = linkset.begin(); pos!=linkset.end(); ++pos) {
		int n = (*pos)->getNumBlocks();
		if (n == 0) continue;
		for (int i=0; i<n; ++i) {
			hctTextBlock_link *block;
			block = (*pos)->getBlock(i);
			if (block->getLanguage() == hctLANG_ENGLISH) {
				if (block->getStart()->getIndex() < minIdx) {
					minPos = block->getStart();
					minIdx = minPos->getIndex();
				}
				if (block->getEnd()->getIndex() > maxIdx) {
					maxPos = block->getEnd();
					maxIdx = maxPos->getIndex();
				}
			}
		}
	}
	++maxIdx;
	++maxPos;
//	wxLogDebug("min=%i   max=%i  txt=%s", minIdx, maxIdx, tibText.c_str());


	// find where to copy the links to
	_searchManager.setCaseSensitive(true);
	txtitr_t startSearchFrom = maxPos;
	for (;;){
		// find Tibetan destination
		_searchManager.setRange(startSearchFrom, getText()->end());
		_searchManager.setLanguage(hctLANG_TIBETAN);
		_searchManager.setText(tibText);
		if (!_searchManager.find()) return;
//		else wxLogDebug("TIB found   %i  %i", _searchManager.getFoundBegin()->getIndex(), _searchManager.getFoundEnd()->getIndex());
		// make sure there are no links in our Tibetan destination text
		for (txtitr_t pos = _searchManager.getFoundBegin(); pos != _searchManager.getFoundEnd(); ++pos) {
			if (pos->getLinkBlock()!=NULL) return;
		}
		txtitr_t tibBegin = _searchManager.getFoundBegin();

		// find English destination
		wxString engText;
		for (txtitr_t pos = minPos; pos != maxPos; ++pos) {
			engText << pos->getLetters();
		}
		_searchManager.setRange(_searchManager.getFoundEnd(), getText()->end());
		_searchManager.setLanguage(hctLANG_ENGLISH);
		_searchManager.setText(engText);
		if (!_searchManager.find()) return;
//		else wxLogDebug("ENG found   %i  %i", _searchManager.getFoundBegin()->getIndex(), _searchManager.getFoundEnd()->getIndex());
		// make sure there are no links in our English destination text
		for (txtitr_t pos = _searchManager.getFoundBegin(); pos != _searchManager.getFoundEnd(); ++pos) {
			if (pos->getLinkBlock()!=NULL) return;
		}
		txtitr_t engBegin = _searchManager.getFoundBegin();

//		wxLogDebug("tibBegin=%i     engBegin=%i", tibBegin->getIndex(), engBegin->getIndex());
		startSearchFrom = _searchManager.getFoundEnd();

		wxGetApp().getRootWindow()->setHasBeenEdited(true);

		// pass #2:  copy the links
		for (std::set<hctHyperlink*>::iterator pos = linkset.begin(); pos!=linkset.end(); ++pos) {
			int n = (*pos)->getNumBlocks();
			if (n == 0) continue;
			hctHyperlink *link = NULL;//new hctHyperlink;
			for (int i=0; i<n; ++i) {
				hctTextBlock_link *block;
				block = (*pos)->getBlock(i);
				wxString txt = block->getText().c_str();
//				wxLogDebug("linking %s", txt.c_str());
				txtitr_t src_offset, dest_offset;
				if (block->getLanguage() == hctLANG_ENGLISH) {
					src_offset = minPos;
					dest_offset = engBegin;
				}
				else {
					src_offset = srcBegin;
					dest_offset = tibBegin;
				}
				--dest_offset;

				hctTempLink tlink;
				tlink.offset = dest_offset->getIndex();
				tlink.start = block->getStart()->getIndex();
				tlink.start-=src_offset->getIndex();
				tlink.start+=tlink.offset;
				tlink.end = block->getEnd()->getIndex();
				tlink.end-=src_offset->getIndex();
				tlink.end+=tlink.offset;
				tlink.search_from = dest_offset;

//				wxLogDebug("o=%i  s=%i  e=%i", tlink.offset, tlink.start, tlink.end);
				link = _linkManager.addLink(link, tlink);

			} // end block loop
		}  // end linkset loop
	}  // end find loop
}  // end func

//			=== event handlers  ===   
//				---  keyboard events  ---
void hctTextWindow::OnKeyDown(wxKeyEvent &event) {
	if (_updating) return;
	int key = event.GetKeyCode();


	if (event.ShiftDown()) {
		_shiftKeyIsDown = true;
		if (key != WXK_SHIFT) _keyIsDown = false;  // FIXME:  (this was to fix a "window focus" issue, is probably obsolete)  
	}
	else _shiftKeyIsDown = false;
	if (!_keyIsDown) {  // ignore key repeat
	switch (key) {
		case WXK_F6:
			_textManager.toggleWylieMode();
			setNeedsUpdate();
			break;
		case WXK_F7:  // TODO:  toggle language insert mode
			break;
		case WXK_F8:
			toggleEditMode();
			break;
		case WXK_ESCAPE:
			_selectionManager.clear();
			_linkManager.clearMarked();
			break;
		}


	//////////////////////////////////////////////////////////////////////////
//		wxLogDebug("hctTextWindow  key: %c", key);

		doCommonEditKeys(key);
		if (_editMode==hctEM_LINK) {
			doLinkEditKeys(key);
		} // else doFreeEditKeys(key);
	}
	_keyIsDown = true;
	event.Skip();
}



void hctTextWindow::doCommonEditKeys(int key) {
	switch (key) {
		case WXK_RETURN:
			_selectionManager.cycleLayers();
			break;
		case WXK_F3:
			// TODO:  search entire document for text
			doSearch();
			break;
	}
}


void hctTextWindow::doLinkEditKeys(int key) {  
	switch (key) {
		case WXK_RETURN:
			_selectionManager.cycleLayers();
			break;
		case WXK_F4:
			wxGetApp().getRootWindow()->wordToClipboard();
			break;
		case ' ':
			linkSelectedText();
			break;
		case 'X':
			_linkManager.unlink(_selectionManager);
			break;
		case 'D':
			_linkManager.deleteFocusedLink();
			break;
		case WXK_LEFT:
			_selectionManager.prevWord();
			break;
		case WXK_RIGHT:
			_selectionManager.nextWord();
			break;
	}
}



void hctTextWindow::OnKeyUp(wxKeyEvent &event) {
	if (_updating) return;
	_keyIsDown = false;
	_shiftKeyIsDown = false;
	event.Skip();
}


void hctTextWindow::OnKeyChar(wxKeyEvent &event) {
	if (_updating) return;
	int key = event.GetKeyCode();
	//wxLogDebug("key: %c", key);

	if (_editMode == hctEM_LINK) {
		switch (key) {
			case WXK_LEFT:
				scrollToFound(false);
				return;
			case WXK_RIGHT:
				scrollToFound(true);
				return;
		}
		event.Skip();
		return;
	}
	_atomUnderMouse = _textManager.getText()->end();

//	if (key != WXK_NEXT && key != WXK_PRIOR) scrollToCursor();
	if (key == WXK_PAGEDOWN || key == WXK_PAGEUP) {
		_scrolledPage = true;
		_prevTopOfWin = getViewRect().GetTop();
	}
//	else scrollToCursor();

	if (key < 256 && isprint(key)) {
//		wxLogDebug("key: %c", key);
		_selectionManager.clear();
		_linkManager.clearMarked();
		_editManager.insertChar((char)key);
		scrollToCursor();
		setNeedsUpdate();
	}
	else {
		switch (key) {
		case WXK_DELETE:
			_editManager.deleteChar(false);
			scrollToCursor();
			break;
		case WXK_BACK:
			_editManager.deleteChar(true);
			scrollToCursor();
			break;
		case WXK_LEFT:
			_editManager.moveCursorLeft();
			scrollToCursor();
			break;
		case WXK_RIGHT:
			_editManager.moveCursorRight();
			scrollToCursor();
			break;
		case WXK_UP:
			scrollToCursor();
			_editManager.moveCursorUp();
			break;
		case WXK_DOWN:
			scrollToCursor();
			_editManager.moveCursorDown();
			break;
		case WXK_RETURN:
			if (_shiftKeyIsDown) {
				_editManager.moveLineUp();
			}
			else {
				_editManager.insertNewline();
				scrollToCursor();
			}
			setNeedsUpdate();
			break;
		default:
			event.Skip();
		}
	}

//	event.Skip();
}
//  ^^^  (end keyboard events)  ^^^



void hctTextWindow::updateAtomUnderMouse() {
//    wxLogVerbose("hctTextWindow::updateAtomUnderMouse()");   //  TEST
	if (_editingNote) return;
	_atomClosestToMouse = _textManager.findTextAt(_mousePos);
	if (_textManager.getTextWasFound())  _atomUnderMouse = _atomClosestToMouse;
	else _atomUnderMouse = getText()->end();
}


//					 ---  mouse events  ---
void hctTextWindow::OnMouseMove(wxMouseEvent &event) {
	if (_updating || _editingNote) return;
	wxPoint devPos = event.GetPosition();
	CalcUnscrolledPosition(devPos.x, devPos.y, &_mousePos.x, &_mousePos.y);
	txtitr_t prevACTM = _atomClosestToMouse;
	updateAtomUnderMouse();

	if (_atomClosestToMouse->isNote() && !_editingNote && prevACTM != _atomClosestToMouse) {
		wxGetApp().getRootWindow()->setNoteText(_atomClosestToMouse->getLetters());
	}

	bool doLookup = !_selectionManager.tibetanSelectorInUse();
	if (!_selectionManager.isSelecting()) _linkManager.refocus(_atomUnderMouse, doLookup);

	_selectionManager.drag(_atomClosestToMouse, _mousePos);
	if (_editMode == hctEM_TEXT) {
		if (_selectionManager.isSelecting()) {
			txtitr_t cp = _atomClosestToMouse;
			if (cp->isRightSide(_mousePos) && !cp->isCtrl()) ++cp;
			_editManager.setCursor(cp);
		}
	}

#ifdef __WXDEBUG__
//#if 0
	////   temp debugging code
	wxString msg;
	msg << _mousePos.x << " " << _mousePos.y;
	((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText(msg, 1);

	if (_atomUnderMouse != getText()->end()) _atomUnderMouse->debug();
	else {
		((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText("", 0);
	}
#else
	//  put wylie in status bar
		if (_atomClosestToMouse != getText()->end())
			((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText(_atomClosestToMouse->getLetters(), 0);
#endif

	//	if (!_textManager.getTextWasFound()){   /////////  TEST
	//		((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText("", 0);
	//	}
	event.Skip();
}


void hctTextWindow::OnMouseLeftBtnDown(wxMouseEvent &event) {
	if (_updating) return;

	wxPoint devPos = event.GetPosition();
	wxPoint logPos;
	CalcUnscrolledPosition(devPos.x, devPos.y, &logPos.x, &logPos.y);

	_linkManager.click(_atomClosestToMouse);

	txtitr_t cp = _atomClosestToMouse;
	switch (_editMode) {
		case hctEM_TEXT:
			_selectionManager.beginSimple();
			if (cp->isRightSide(_mousePos) && !cp->isCtrl()) ++cp;
			_editManager.setCursor(cp);
            _searchNeedsReset = true;
//			_searchBegin = NULL;
		break;
		case hctEM_LINK:
			if (_editingNote) {
				if (_atomUnderMouse != getText()->end()) {
					wxString note = wxGetApp().getRootWindow()->getNoteText();
					if (note.size() > 0) {
						_atomUnderMouse->setLetters(note);
					}
					else {  // note is empty, so delete it
						txtitr_t pos = _atomUnderMouse;
						++pos;
						_textManager.deleteTextAtomAt(_atomUnderMouse);
						_atomUnderMouse = getText()->end();
						if (pos != getText()->end() || pos->getCtrlChar() == hctCTRL_BREAK)
							_textManager.deleteTextAtomAt(pos);
					}
					_editingNote = false;
					wxGetApp().getRootWindow()->setHasBeenEdited(true);
				}
				wxGetApp().getRootWindow()->endNoteEdit();
				_selectionManager.clear();
				_linkManager.clearMarked();
				return;
			}
			if (_atomUnderMouse == getText()->end()) {   // outside of text area
				_selectionManager.clear();
				_linkManager.clearMarked();
			}
			else {
#ifdef __WXMAC__
				_selectionManager.begin(_atomUnderMouse, event.MetaDown());
#else
				_selectionManager.begin(_atomUnderMouse, event.ControlDown());
#endif
			}
		break;
	}
	event.Skip();
}

void hctTextWindow::OnMouseLeftBtnDblClk(wxMouseEvent &event) {
	if (_updating) return;
	if (_editMode != hctEM_LINK) {
		OnMouseLeftBtnDown(event);  // ignore
		return;
	}

	if (_atomClosestToMouse->getCtrlChar()==hctCTRL_BREAK) {
		txtitr_t pos = _atomClosestToMouse;
		bool addNote = true;
		if (pos != getText()->begin()) {
			--pos;
			if (pos->getCtrlChar()!=hctCTRL_BREAK) addNote = false;
		}
		if (addNote) {
			_editingNote = true;
			hctTextAtom noteAtom;
			noteAtom.setIsNote(true);
			_textManager.addTextAtomAt(_atomClosestToMouse, noteAtom);
			pos = _atomClosestToMouse;
			_atomUnderMouse = --pos;
			hctTextAtom newlineAtom;
			newlineAtom.setCtrlChar(hctCTRL_BREAK);
			newlineAtom.setLanguage(_atomClosestToMouse->getLanguage());
			_textManager.addTextAtomAt(_atomClosestToMouse, newlineAtom);
			wxGetApp().getRootWindow()->beginNoteEdit();
			setNeedsUpdate();
			wxGetApp().getRootWindow()->setHasBeenEdited(true);
		}
	}
	else if (_atomUnderMouse != getText()->end() && 
		_atomUnderMouse->isNote()) {
			_editingNote = true;
			wxGetApp().getRootWindow()->beginNoteEdit();
	}
	else OnMouseLeftBtnDown(event);  // ignore
	event.Skip();
}

void hctTextWindow::OnMouseLeftBtnUp(wxMouseEvent &event) {
	if (_updating) return;
	wxPoint devPos = event.GetPosition();
	wxPoint logPos;
	CalcUnscrolledPosition(devPos.x, devPos.y, &logPos.x, &logPos.y);
	_selectionManager.end();
//	_selectingText = false;
	event.Skip();
}



void hctTextWindow::OnMouseRightBtnDown(wxMouseEvent &event) {
	if (_updating) return;
	wxPoint devPos = event.GetPosition();
	wxPoint logPos;
	CalcUnscrolledPosition(devPos.x, devPos.y, &logPos.x, &logPos.y);

	switch (_editMode) {
		case hctEM_TEXT:
			//	setCursor(_atomUnderMouse);
			//	hideCursor();
		break;
		case hctEM_LINK:
				if (_selectionManager.getNumSelectors() > 0) linkSelectedText();
				else _linkManager.unlink(_selectionManager);
		break;
	}


	//	if (_txtIndexUnderMouse >= 0 && _text[_txtIndexUnderMouse].getSelectBlock() != NULL) linkSelectedText();
	//	else if (_curLinkBlockFocused) unlinkFocusedText();
	event.Skip();
}

void hctTextWindow::OnMouseRightBtnDblClk(wxMouseEvent &event) {
	if (_updating) return;
	OnMouseRightBtnDown(event);  //	disable for now...				//////////////////////////
	event.Skip();
}

void hctTextWindow::OnMouseRightBtnUp(wxMouseEvent &event) {
	if (_updating) return;
	wxPoint devPos = event.GetPosition();
	wxPoint logPos;
	CalcUnscrolledPosition(devPos.x, devPos.y, &logPos.x, &logPos.y);
	event.Skip();
}
//  ^^^  (end mouse events)  ^^^




void hctTextWindow::OnSize(wxSizeEvent& event) {
//	wxLogDebug("OnSize");
	if (_updating) {
		event.Skip();
		return;
	}
	setNeedsUpdate();
//	update();
	event.Skip();
}

void hctTextWindow::OnScroll(wxScrollWinEvent& event) {
	wxScrolledWindow::OnScroll(event);
	_textManager.calcVisibleTextRange(getViewRect());
    _searchNeedsReset = true;

//	wxLogDebug("OnScroll  %i", _prevTopOfWin - getViewRect().GetTop());

	// FIXME
	if (_scrolledPage) {
		_scrolledPage = false;
		wxPoint csp = _editManager.getCursorScreenPos();
		wxLogDebug("  %i %i ", csp.x, csp.y);
		csp.y += getViewRect().GetTop() - _prevTopOfWin + 27;
		wxLogDebug("             %i %i ", csp.x, csp.y);
		_editManager.setCursor(_textManager.findTextAt(csp), true, false, true);
	}
//	event.Skip();
}

void hctTextWindow::OnPaint(wxPaintEvent& event) {
	_editManager.beginSystemPaint();
	wxPaintDC dc(this);
//	dc.BeginDrawing();
	dc.SetBackground(*wxWHITE_BRUSH);
	dc.Clear();  
	PrepareDC(dc);
	_textManager.paint(dc);
//	dc.EndDrawing();
	_editManager.endSystemPaint();
}

void hctTextWindow::OnIdle(wxIdleEvent& event) {
//	wxLogDebug("OnIdle");
	if (_needsUpdate) update();

	if (_firstUpdate) {
		_firstUpdate = false;
		wxGetApp().onFirstUpdate();
	}

	event.Skip();
}

//  ^^^  (end event handlers)  ^^^



void hctTextWindow::scrollToFound(bool next) {
	hctTextBlock_link *block = NULL;
	if (next) block = _linkManager.getNextFound();
	else block = _linkManager.getPrevFound();
	if (block) {
		float top = block->getStart()->getRect().GetTop();
		Scroll(0, top/_scrollRate);
		setNeedsUpdate();
	}
}

void hctTextWindow::linkSelectedText() {
	// check for invalid links
	//   (This is very important to do - linking an invalid
	//   text selector will result in a crash!)
	if (!_selectionManager.isValidForLinking()) {
		wxString txt = "Cannot link selected text:  Links would overlap without full containment.";
		wxGetApp().getRootWindow()->setNoteText(txt);
		return;
	}
	else wxGetApp().getRootWindow()->clearDefText();

	// do the link
	hctHyperlink *link = new hctHyperlink;
	_selectionManager.linkSelected(link);
	_linkManager.addLink(link);

	// update focus
	//	refocus();
}


void hctTextWindow::scrollToCursor() {
	if (_editManager.getCursorPos() == _textManager.getText()->end()) {
		Scroll(0, 0);
		return;
	}

	wxRect view = getViewRect();
	wxRect cursor = _editManager.getCursorPos()->getRect();

	int top = cursor.GetTop() - cursor.GetHeight();
	if (top < 0) top = 0;

	if (top < view.GetTop()) {
		wxLogDebug("need to go up");
		Scroll(0, top/_scrollRate);
		_textManager.calcVisibleTextRange(getViewRect());
		return;
	}

	int bottom = cursor.GetBottom() + cursor.GetHeight();
	if (bottom > view.GetBottom()) {
		wxLogDebug("need to go down");
		Scroll(0, (bottom-view.GetHeight())/_scrollRate+1);
		_textManager.calcVisibleTextRange(getViewRect());
	}

}


void hctTextWindow::test() {   //////////////////////////////////////////////////////////////////////////
	wxLogDebug("test");
    
    clear();
    getGlyphStacker()->testKerningTable();
//    getGlyphStacker()->testFont();
    setNeedsUpdate();
}
////////////////////////////////////////////////////////////////////////////// 


