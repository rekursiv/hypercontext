#include "stdwx.h"
#include "hctTextWindow.h"
#include "hctEditManager.h"
#include "hctTextManager.h"
#include "hctTextBlock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void hctCursorBlinker::Notify() {
//	 wxLogDebug("*blink*");
	 if (_em) _em->paintCursor();
 }

hctEditManager::hctEditManager(hctTextWindow* ownerWin) {
	_ownerWin = ownerWin;
	_cursorPos = text()->end();
	_cursorShowing = false;
	_cursorEnabled = false;
	_cursorBlinker.SetOwner(this);
}

hctEditManager::~hctEditManager() {
}

text_t* hctEditManager::text(){
	return _ownerWin->getText();
}

void hctEditManager::paintCursor() {
	if (!_cursorEnabled) return;

	wxRect rect;
	if (_cursorPos == text()->end()) {
		if (_cursorPos == text()->begin()) {
			rect.SetPosition(_ownerWin->getTextManager()->getTextStartPt());
			rect.width = 24;
			rect.height = 24;
		}
		else {
			txtitr_t pos = _cursorPos;
			--pos;
			rect = pos->getRect();
			if (pos->getCtrlChar() == hctCTRL_BREAK) {
				wxPoint stpt = _ownerWin->getTextManager()->getTextStartPt();
				rect.x = stpt.x;
				rect.y += rect.height;
			}
			else rect.x+=rect.width;
		}
	}
	else {
		rect = _cursorPos->getRect();
	}
	wxClientDC dc(_ownerWin);
//	dc.BeginDrawing();
	dc.SetLogicalFunction(wxXOR);
	dc.SetPen(*wxTRANSPARENT_PEN);
	_ownerWin->PrepareDC(dc);
	dc.DrawRectangle(rect.x, rect.y, 2, rect.height);
//	dc.EndDrawing();
	_cursorShowing = !_cursorShowing;
}


void hctEditManager::autoLinguify() {  // FINISHME
	for (txtitr_t i= text()->begin(); i!= text()->end(); ++i) {
		// TODO:  find Tibetan text marked as hctLANG_NONE, cut it, then paste it as Tibetan
	}
}


void hctEditManager::moveLineUp() {
	if (_cursorPos == text()->end()) return;
	txtitr_t to = _cursorPos;
	while (to->getCtrlChar() != hctCTRL_BREAK) {
		if (to == text()->begin()) return;
		--to;
	}

	txtitr_t eol = _cursorPos;
	while (eol->getCtrlChar() != hctCTRL_BREAK) {
		if (eol == text()->end()) return;
		++eol;
	}

	text()->splice(to, *text(), _cursorPos, eol);
}

void hctEditManager::setCursor(txtitr_t newPos, bool refresh, bool updateScreenPosX, bool updateScreenPosY) {
	if (_cursorPos == newPos) return;
	if (_cursorShowing) paintCursor();
	txtitr_t prevPos = _cursorPos;
	_cursorPos = newPos;
	resetCursorBlink();  // draws cursor
	if (_cursorPos != text()->end()) {
		_cursorPos->debug();
		if (updateScreenPosX) _cursorScreenPos.x = _cursorPos->getPos().x;
		if (updateScreenPosY) _cursorScreenPos.y = _cursorPos->getPos().y;
	}
}


void hctEditManager::moveCursorRight() {
	txtitr_t pos = _cursorPos;
	if (pos == text()->end()) return;
	++pos;
	setCursor(pos);
}

void hctEditManager::moveCursorLeft() {
	txtitr_t pos = _cursorPos;
	if (pos == text()->begin()) return;  
	--pos;
	setCursor(pos);
}



void hctEditManager::moveCursorUp() {
	rowitr_t row;
	if (_cursorPos == text()->end()) row = _ownerWin->getTextManager()->getRows()->end();
	else row = _cursorPos->getRow();
	if (row == _ownerWin->getTextManager()->getRows()->begin()) return;
	if (_cursorPos == text()->end()) --row;
	--row;
	wxPoint pnt;
	pnt.y = row->getRect().GetTop()+1;
	pnt.x = _cursorScreenPos.x;

	setCursor(_ownerWin->getTextManager()->findTextAt(pnt), true, false);
}

void hctEditManager::moveCursorDown() {
	if (_cursorPos == text()->end()) return;
	rowitr_t row = _cursorPos->getRow();
	++row;
	if (row == _ownerWin->getTextManager()->getRows()->end()) {
		wxLogDebug("dn  ***");
		return;
	}
	wxPoint pnt;
	pnt.y =  row->getRect().GetTop()+1;
	pnt.x = _cursorScreenPos.x;

	wxLogDebug("dn  %i", _ownerWin->getTextManager()->findTextAt(pnt)->getIndex());

	setCursor(_ownerWin->getTextManager()->findTextAt(pnt), true, false);
}


void hctEditManager::insertNewline() {
	hctTextAtom atom;
	if (_cursorPos != text()->end()) {
		if (_cursorPos == text()->begin()) atom = *_cursorPos;
		else {
			txtitr_t pos = _cursorPos;
			--pos;
			atom = *pos;
		}
	}
	atom.resetDefaults();
	atom.setLinkBlock(NULL);
	atom.detachSelectBlock(NULL);
	atom.setCtrlChar(hctCTRL_BREAK);
	_ownerWin->getTextManager()->addTextAtomAt(_cursorPos, atom);
}


void hctEditManager::insertChar(char c) {
	wxString letters;
	letters << c;

	hctTextAtom atom;
	if (_cursorPos != text()->end()) {
		if (_cursorPos == text()->begin()) atom = *_cursorPos;
		else {
			txtitr_t pos = _cursorPos;
			--pos;
			atom = *pos;

			hctTextBlock *block = atom.getBlock();
			if (block) {
				if (block->getEnd() == pos) {
	//				block->incEnd();  // new char inherits the link to the cursor's left - causes problems

					// don't continue the link - this seems to work better
					atom.setLinkBlock(NULL);
					atom.detachSelectBlock(NULL);
				}
			}
		}
	}

	atom.resetDefaults();
	atom.setLetters(letters);
	_ownerWin->getTextManager()->addTextAtomAt(_cursorPos, atom);
}

void hctEditManager::deleteChar(bool delPrev) {
	txtitr_t pos = _cursorPos;

	if (delPrev) {
		if (_cursorPos == text()->begin()) return;
		--pos;
	}
	else 
	{
		if (_cursorPos == text()->end()) return;
		++_cursorPos;
	}
//	wxLogDebug("---cp=%i   pos=%i",_cursorPos->getIndex(), pos->getIndex());
	_ownerWin->getTextManager()->deleteTextAtomAt(pos);
}

