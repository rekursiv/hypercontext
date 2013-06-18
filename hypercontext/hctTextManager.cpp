#include "stdwx.h"
#include "hctTextManager.h"
#include "hctTextWindow.h"
#include "hctApp.h"
#include "hctRootWindow.h"
#include "hctFontManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



hctTextManager::hctTextManager(hctTextWindow* ownerWin) : _glyphStacker(this)
{
	_ownerWin = ownerWin;
	if (hctFontManager::getInstance()->areTibetanFontsInstalled()) {
		_noTibetanFonts = false;
		_textInWylieMode = false;
	}
	else {
		wxLogDebug("Tibetan fonts not found!");
		_noTibetanFonts = true;
		_textInWylieMode = true;
	}
	hctTextManager::init();
}



hctTextManager::~hctTextManager() {
}


void hctTextManager::init() {
	//  text layout options
	_margin = 21;
	_spaceBetweenParagraphs = 50;  // obsolete
	///////////////////////

	_visTxtRange_start = _text.begin();
	_visTxtRange_end = _text.end();
	_visRowRange_start = _row.begin();
	_visRowRange_end = _row.end();

	_textSwitchModes = true;
	_textWasFound = false;
    
	_needsFullRefresh = true;
}


void hctTextManager::clear() {
	_text.clear();
	_row.clear();
	hctTextManager::init();
}


// this function is just an experiment - will be replaced by "paste"
void hctTextManager::makeTibetan(txtitr_t begin, txtitr_t end) {
	for (txtitr_t i=begin; i!=end;) {
		i->setLanguage(hctLANG_TIBETAN);
		_glyphStacker.addTextAtom(*i);
		_text.erase(i++);
	}
	_glyphStacker.stack();
	_ownerWin->update();
}


void hctTextManager::addTextAtom(hctTextAtom &atom) {
	atom.setOwnerWin(_ownerWin);
	atom.setWylieDisplayMode(_textInWylieMode);
	_text.push_back(atom);
}
void hctTextManager::addTextAtomAt(txtitr_t pos, hctTextAtom &atom) {
	atom.setOwnerWin(_ownerWin);
	atom.setWylieDisplayMode(_textInWylieMode);
	_text.insert(pos, atom);
	wxGetApp().getRootWindow()->setHasBeenEdited(true);
}

bool hctTextManager::deleteTextAtomAt(txtitr_t pos) {
	if (pos == _text.end()) return false;
	_ownerWin->getSelectionManager()->clear();
	pos->prepareForDelete();
	_text.erase(pos);
	_ownerWin->update();  // can't call setNeedsUpdate() here, itrs need immediate reassignment
	wxGetApp().getRootWindow()->setHasBeenEdited(true);
	return true;
}

int hctTextManager::arrangeText(int clientWidth) {
//	wxLogDebug("hctTextManager::arrangeText()");
	int textAreaWidth = clientWidth - _margin;
	if (textAreaWidth < 50) return 0;

	_row.clear();

	int rowHeight = 0;
	int rowTop = 0; 

	wxPoint curPos(_margin, _margin/2);
	int count = 0;
	int column = 0;

	rowitr_t curRow = _row.end();

	bool lineBreak = false;
	bool endOfRow = true;
	bool rowStarted = true;
	txtitr_t breakpnt = _text.begin();
	txtitr_t prevBreakpnt = _text.end();
	txtitr_t i;
	for (i=_text.begin(); i!=_text.end(); ++i) {
		i->setNeedsRefresh(true);
		if (_textSwitchModes) i->setWylieDisplayMode(_textInWylieMode);
		i->setPos(curPos);
		i->setIndex(count++);
		if (i->getHeight() > rowHeight) rowHeight = i->getHeight();
		rowTop = i->getRect().GetTop();  // FIXME:  can we just derive this from curPos.y  ??

		if (i->getCtrlChar() == hctCTRL_BREAK || i->getCtrlChar() == hctCTRL_EOF) {
			lineBreak = true;
		}
		else if (i->getCtrlChar() == hctCTRL_PARAGRAPH) {
			lineBreak = true;
			rowHeight += _spaceBetweenParagraphs;
		}

		if (i->getRect().GetRight() > textAreaWidth && breakpnt != prevBreakpnt) {
			lineBreak = true;
			prevBreakpnt = breakpnt;
			i = breakpnt;
			count = breakpnt->getIndex()+1;
		}
		if (i->isWordWrapPnt()) breakpnt = i;

		if (endOfRow) {
			endOfRow = false;
			startRow(i);
			rowStarted = true;
			++curRow;
			column = 0;
		}
		if (lineBreak) {
			lineBreak = false;
			endOfRow = true;
			endRow(i, curRow, rowHeight, rowTop);
			rowStarted = false;
			curPos.x = _margin;
			curPos.y += rowHeight;
			rowHeight = 0;
		}
		else curPos.x+=i->getWidth();
		i->setRow(curRow);
		i->setColumn(column++);
	}
	_textSwitchModes = false;
	if (rowStarted) endRow(--i, curRow, rowHeight, rowTop);
	return curPos.y+rowHeight+80; 
}

void hctTextManager::endRow(txtitr_t atom, rowitr_t row, int rowHeight, int rowTop) {
	if (row ==_row.end()) return;
	wxRect rect;
	rect.x = _margin;
	rect.y = rowTop;
	rect.width = atom->getRect().GetRight() - _margin;
	rect.height = rowHeight;
	row->setRect(rect);
	row->setAtomEnd(atom);
//	wxLogDebug("endRow:  %i    %i %i", _row.size()-1, row->getRect().GetTop(), row->getRect().GetBottom());
}

void hctTextManager::startRow(txtitr_t atom) {
//	wxLogDebug("startRow:  %i", _row.size());
	hctRow row;
	row.setIndex(_row.size());
	row.setAtomBegin(atom);
	_row.push_back(row);
}


void hctTextManager::calcVisibleTextRange(wxRect view) {
	int top = view.GetTop();
	int bottom = view.GetBottom();

	_visTxtRange_start = _text.end();
	_visTxtRange_end = _text.end();
	_visRowRange_start = _row.end();
	_visRowRange_end = _row.end();

	bool foundStart = false;
	for (rowitr_t i=_row.begin(); i!=_row.end(); ++i) {
		if (!foundStart) {
			if (top < i->getRect().GetBottom()) {
				_visRowRange_start = i;
				_visTxtRange_start = i->getAtomBegin();
				foundStart = true;
			}
		}
		else if (bottom < i->getRect().GetBottom()) {
			_visRowRange_end = i;
			_visTxtRange_end = i->getAtomEnd();
			++_visTxtRange_end;
			break;
		}
	}
//	wxLogDebug("---cvtr  top=%i  topRow=%i  btmRow=%i", top, distance(_row.begin(), _visRowRange_start), distance(_row.begin(), _visRowRange_end));
//	int dbgEnd = -1;
//	if (_visTxtRange_end != _text.end()) dbgEnd = _visTxtRange_end->getIndex();
//	wxLogDebug("start = %i   end = %i", _visTxtRange_start->getIndex(), dbgEnd);
}


txtitr_t hctTextManager::findTextAt(wxPoint pos) {
	_textWasFound = false;

	txtitr_t start = _text.begin();
	if (pos.y < _visRowRange_start->getRect().GetTop()) return start;
	txtitr_t end = _text.end();
	--end;
	rowitr_t endRow = _visRowRange_end;
	if (endRow == _row.end()) endRow--;
	if (pos.y > endRow->getRect().GetBottom()) return end;


	// search rows
	float top_y, btm_y, prev_btm_y = 0.0f;
	rowitr_t e = _visRowRange_end;
	if (_visRowRange_end != _row.end()) ++e;
	for (rowitr_t row=_visRowRange_start; row!=e; ++row) {
		top_y = row->getRect().GetTop();
		btm_y = row->getRect().GetBottom();
		if (pos.y >= top_y && pos.y <= btm_y) {
			//	wxLogDebug("findTextAt:   %i %i    %i", row->getRect().GetTop(), row->getRect().GetBottom(), row->getRect().GetHeight());
			start = row->getAtomBegin();
			end = row->getAtomEnd();
			for (txtitr_t i=start; i!=end; ++i) {
				if (i->isInsideX(pos.x)) {
					_textWasFound = true;
					return i;
				}
			}
			if (pos.x < _margin) return start;
			else return end;
		}
		else if (pos.y >= prev_btm_y && pos.y <= top_y) {
			//			wxLogDebug("between");
			return row->getAtomBegin();
		}
		prev_btm_y = btm_y;
	}

	//	wxLogDebug("###########################################################################");
	return _text.begin();  // should never get here...
}


void hctTextManager::paint(wxPaintDC &dc) {
//	wxLogDebug("paint()");
	_ownerWin->getSelectionManager()->paint();
	for (txtitr_t i=_visTxtRange_start; i!=_visTxtRange_end; ++i) {
		i->paint(dc);
	}
//	if (!_needsFullRefresh) wxLogDebug("     (partial)");
	_needsFullRefresh = true;

//	for (txtitr_t i=_text.begin(); i!=_text.end(); ++i) {  //  DEBUG   (SLOW!!!)
//		i->paint(dc);
//	}
}
