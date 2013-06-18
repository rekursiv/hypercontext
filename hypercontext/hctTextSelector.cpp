#include "stdwx.h"
#include "hctTextSelector.h"
#include "hctTextAtom.h"
#include "hctApp.h"
#include "hctRootWindow.h"
#include "hctDictTibEng.h"
#include "hctTextManager.h"  // for rows

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctTextSelector::hctTextSelector(text_t *text) {
	_text = text;
	_curSel = NULL;
	_selecting = false;
	_selSimple = false;
}

hctTextSelector::~hctTextSelector() {
	reset();
}


hctLanguage_t hctTextSelector::getLanguage() {
	if (_curSel) return _curSel->getLanguage();
	return hctLANG_NONE;
}


void hctTextSelector::reset() {
	if (_curSel) {
		delete _curSel;
		_curSel = NULL;
	}
}


bool hctTextSelector::isBlank() {
	if (!_curSel) return true;
	if (_curSel->isEmpty()) return true;
	return false;
}


void hctTextSelector::beginSimple(txtitr_t atom, wxPoint pos) {
	if (atom == _text->end()) return;
	_selAnchor = atom;
	if (_selAnchor->isRightSide(pos)) ++_selAnchor;
	if (_selAnchor == _text->end()) --_selAnchor;
	reset();
	_selecting = true;
	_selSimple = true;
}

bool hctTextSelector::begin(txtitr_t atom) {
	int index = -1;
	if (atom!=_text->end()) index = atom->getIndex();
//	wxLogDebug("hctTextSelector::begin()   i=%i  ", index);

	_selecting = false;
	_selSimple = false;
	if (_curSel) {
		if (_curSel->isReselect(atom)) {
			if (_curSel->getNumLayers() > 0) {
//				wxLogDebug("      cycle layers");
				_curSel->cycleLayers();
				wxGetApp().getRootWindow()->define(_curSel->getLayerText());
			}
			return true;
		}
	}
	return selectWord(atom);
}

void hctTextSelector::cycleLayers() {
	if (_curSel) {
		if (_curSel->getNumLayers() == 0) {
			txtitr_t start = _curSel->getStart();
			reset();
			selectWord(start);
		}
		else {
			_curSel->cycleLayers();
			wxGetApp().getRootWindow()->define(_curSel->getLayerText());
		}
	}
}

void hctTextSelector::nextWord() {
	if (_curSel) {
		txtitr_t end = _curSel->getEnd();
		if (end == _text->end()) return;
		++end;
		if (end == _text->end()) return;
		while (checkForBreakPoint(end)) {
			++end;
			if (end == _text->end()) return;
		}
		selectWord(end);
	}
}

void hctTextSelector::prevWord() {
	if (_curSel) {
		txtitr_t start = _curSel->getStart();
		if (start == _text->begin()) return;
		--start;
		while (start != _text->begin() && checkForBreakPoint(start)) --start;
		selectWord(start);
	}
}

bool hctTextSelector::selectWord(txtitr_t atom) {
	reset();
	if (atom->isCtrl()) return false;
	if (atom->isNote()) {
		_curSel = new hctTextBlock_select(_text);
		_curSel->setRange(--atom, ++atom);
		return true;
	}

	wxString strUnderMouse = atom->getLetters();
//	wxLogDebug("  sum:  '%s'", strUnderMouse);

	// find start of word
	txtitr_t start;
	txtitr_t end;
	if (checkForBreakPoint(atom)) {
//		wxLogDebug("****  bp");
		start = end = atom;
		if (strUnderMouse == " " || (strUnderMouse == "," && atom->getLanguage() == hctLANG_TIBETAN)) {
			// don't select spaces, tseks, or shays
			return false;
		}
		else {
			++end;
		}
	}
	else {
		start = findStartPt(atom);
		end = findEndPt(atom);
		if (atom != _text->begin()) --atom;
		if (!checkForBreakPoint(atom)) {
			++atom;
			// allow selection of ending particles on Tibetan syllables
			if (atom->getLanguage() == hctLANG_TIBETAN && isParticle(atom)) {
//				wxLogDebug("   selecting ending particle");
				_curSel = new hctTextBlock_select(_text);
				_curSel->setRange(atom, atom);
				if (strUnderMouse.Length() < 2) strUnderMouse << "A";
				wxGetApp().getRootWindow()->define(strUnderMouse);
				return true;
			}
		}
	}

//	wxLogDebug("----  selectWord:  %i -> %i", start->getIndex(), end->getIndex());  ///

	if (start->getLanguage() == hctLANG_TIBETAN) {
		_curSel = new hctTextBlock_select_layered(_text);
		_curSel->setRange(start, end);
		findLayers();
		wxGetApp().getRootWindow()->define(_curSel->getLayerText());
	}
	else {
		_curSel = new hctTextBlock_select(_text);
		_curSel->setRange(start, end);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

void hctTextSelector::findLayers() {
	txtitr_t pos = _curSel->getEnd();
//	wxLogDebug("-findLayers:  %i ", pos->getIndex());  ///

	hctDictFindInfo fi;
	txtitr_t prevLayer = pos;
	for (int c=0; c<100; ++c) {
		if (pos == _text->end()) break;
		if (pos->getLanguage() != hctLANG_TIBETAN) break;

		if (isParticle(pos)) {
			--pos;
//			wxLogDebug("   found particle");
			fi = wxGetApp().getRootWindow()->dictFind(_curSel->getText(pos));
			if (fi.found && fi.exact) {
				_curSel->addLayer(pos);
				prevLayer = pos;
			}
			++pos;
		}

		fi = wxGetApp().getRootWindow()->dictFind(_curSel->getText(pos));
		if (fi.found && fi.exact) {
			_curSel->addLayer(pos);
			prevLayer = pos;
		}
		if (!fi.found) break;
		if (fi.exact && fi.unique) break;

		if (fi.unique) {

//wxString s;  s << "  unique -  '" << _curSel->getText(pos) << "'";  wxLogDebug(s);


			for (int i=0; i<fi.remainder;) {
//				wxLogDebug("loop");
				++pos;
				if (pos == _text->end()) break;
				i+=pos->getLetters().size();
//s.clear(); s << "   '" << _curSel->getText(pos) << "'";  wxLogDebug(s);

			}
			if (pos == _text->end()) break;

			fi = wxGetApp().getRootWindow()->dictFind(_curSel->getText(pos));
			if (fi.found && fi.exact) {
				_curSel->addLayer(pos);
				prevLayer = pos;
			}
			break;
		}

		pos = findEndPt(++pos);

	}
	_curSel->setEnd(prevLayer);
}


bool hctTextSelector::checkForBreakPoint(txtitr_t atom) {
	if (atom->isWordWrapPnt() || 
		atom->isCtrl() ||
		atom->getLetters().at(0) == '\"' ||
		atom->getLetters().at(0) == ',' ||
		atom->getLetters().at(0) == '.' ||
		atom->getLetters().at(0) == ';' ||
		atom->getLetters().at(0) == ':' ||
		atom->getLetters().at(0) == '{' ||
		atom->getLetters().at(0) == '}' ||
		atom->getLetters().at(0) == '[' ||
		atom->getLetters().at(0) == ']') return true;
	return false;
}

txtitr_t hctTextSelector::findStartPt(txtitr_t atom) {
	if (atom == _text->begin()) return atom;
	while (!checkForBreakPoint(atom)) {
		if (atom == _text->begin()) return atom;
		--atom;
	}
	return ++atom;
}

txtitr_t hctTextSelector::findEndPt(txtitr_t atom) {
	if (atom == _text->end()) return atom;
	++atom;
	if (atom == _text->end()) return atom;
	while (!checkForBreakPoint(atom)) {
		atom->setStartsLayer(false);
		++atom;
		if (atom == _text->end()) return atom;
	}
	return --atom;
}

txtitr_t hctTextSelector::findNextEndPt(txtitr_t atom) {
	++atom;
	if (checkForBreakPoint(atom)) ++atom;
	return findEndPt(atom);
}

bool hctTextSelector::isParticle(txtitr_t atom) {
	if (atom->getLanguage() == hctLANG_TIBETAN) {
		if (atom->getLetters() == "'I" ||
			atom->getLetters() == "'O" ||
			atom->getLetters() == "R" ||
			atom->getLetters() == "S" ) {
				++atom;
				if (atom != _text->end() && checkForBreakPoint(atom))	return true;
				else return false;
			}
	}
	return false;
}



void hctTextSelector::freeze() {
	if (!_curSel) return;
	if (_curSel->getNumLayers() > 0) {
		_curSel->unlayer();
	}
}


void hctTextSelector::setLinkBlock(hctTextBlock_link *block) {
	if (!block) return;
	reset();
	if (block->isEmpty()) {
		delete block;
		return;
	}
	txtitr_t start = block->getStart();
	txtitr_t end = block->getEnd();
	delete block;

	_curSel = new hctTextBlock_select(_text);
	_curSel->setRange(start, end);
}

hctTextBlock_link *hctTextSelector::getLinkBlock() {
	// manually copy text block
	if (!_curSel || _curSel->isEmpty()) return NULL;

	txtitr_t start = _curSel->getStart();
	txtitr_t end = _curSel->getEnd();
	// check for valid range???

	delete _curSel;
	_curSel = NULL;

	hctTextBlock_link *sel = new hctTextBlock_link(_text);
	sel->setRange(start, end);
	return sel;
}

//  TODO:  rewrite so atom index not used
bool hctTextSelector::isValidForLinking() {
	if (!_curSel) return false;
	txtitr_t start = _curSel->getStart();
	txtitr_t end = _curSel->getEnd();

	hctTextBlock_link* startBlock = start->getLinkBlock();
	hctTextBlock_link* endBlock = end->getLinkBlock();
	if (startBlock == endBlock) {  // inside
		if (startBlock == NULL) return true;
		if (duplicates(startBlock)) return false;
		else return true;
	}
	while (startBlock) {
		if (duplicates(startBlock)) return false;
		if (startBlock->getStart()->getIndex() < start->getIndex()) {
//			wxLogDebug("start 1:  %i  %i", startBlock->getStart()->getIndex(), start->getIndex());
			if (startBlock->getEnd()->getIndex() != end->getIndex()) {
//				wxLogDebug("start 2:  %i  %i", startBlock->getEnd()->getIndex(), end->getIndex());
				return false;
			}
		}
//		txtitr_t sbe = startBlock->getEnd();
//		for (txtitr_t i = start; i!=end; ++i) {
//			if (i == sbe) return false;
//		}
		startBlock=startBlock->getNext();
	}
	while (endBlock) {
		if (endBlock->getEnd()->getIndex() > end->getIndex()) {
//			wxLogDebug("end 1:  %i  %i", endBlock->getEnd()->getIndex(), end->getIndex());
			if (endBlock->getStart()->getIndex() != start->getIndex()) {
//				wxLogDebug("end 2:  %i  %i", endBlock->getStart()->getIndex(), start->getIndex());
				return false;
			}


		}
//		txtitr_t ebs = endBlock->getStart();
//		txtitr_t e = end; ++e;
//		for (txtitr_t i = start; i!=e; ++i) {
//			if (i == ebs) return false;
//		}
		endBlock=endBlock->getNext();
	}

	return true;
}


bool hctTextSelector::duplicates(hctTextBlock *block)
{
	if (!_curSel) return false;
	if (!block) return false;
	return (_curSel->getStart() == block->getStart() && _curSel->getEnd() == block->getEnd());
}


void hctTextSelector::drag(txtitr_t atom, wxPoint pos) {
	if (atom == _text->end()) {
		_selecting = false;
		_selSimple = false;
		return;
	} 

	if (!_selecting) {
		if (atom->getSelectBlock() != _curSel) {
			_selecting = true;
			if (_curSel) {
				freeze();

				// figure out which side of the selected word to "anchor"
				txtitr_t begin = _curSel->getStart();
				txtitr_t end = _curSel->getEnd();
				for (;;) {
					if (begin == _text->begin() && end == _text->end()) break;
					else if (begin == atom) {
						_selAnchor = _curSel->getEnd();
						break;
					}
					else if (end == atom) {
						_selAnchor = _curSel->getStart();
						break;
					}
					if (begin != _text->begin()) --begin;
					if (end != _text->end()) ++end;
				}
			}
		}
		return;
	}
	else {
//		wxLogDebug("hctTextSelector::drag()   atom=%i  anchor=%i", atom->getIndex(), _selAnchor->getIndex());

		// figure out which side of the "anchor point" we are selecting text on
		int anchorX = _selAnchor->getRect().GetLeft();
		int anchorTop = _selAnchor->getRow()->getRect().GetTop();
		int anchorBottom = _selAnchor->getRow()->getRect().GetBottom();
		bool right = true;
		if (pos.x >= anchorX) {
			if (pos.y < anchorTop) right = false;
		}
		else {
			if (pos.y <= anchorBottom) right = false;
		}

		// do the selection
		bool selChanged = false;
		if (right) {
			txtitr_t end = atom;
			if (!atom->isRightSide(pos)) {
				if (end == _selAnchor) {
					reset();
					wxGetApp().getRootWindow()->clearDefText();
					return;
				}
				else --end;
			}
			if (!_curSel) _curSel = new hctTextBlock_select(_text);
			if (_curSel->getEnd() != end) selChanged = true;
			if (!_curSel->setRange(_selAnchor, end)) {
				// detected collision
//				wxLogDebug("       detected collision right");
				txtitr_t col = _curSel->getCollision();
				--col;
				_curSel->setRange(_selAnchor, col);
				return;
			}
//			wxLogDebug("right");
		}
		else {
			if (_selAnchor == atom) return;
			txtitr_t begin = atom;
			txtitr_t end = _selAnchor;
			if (_selSimple) --end;
			if (atom->isRightSide(pos)) {
				if (begin == end) {
					reset();
					return;
				}
				else ++begin;
			}
			if (!_curSel) _curSel = new hctTextBlock_select(_text);
			if (_curSel->getStart() != begin) selChanged = true;
			if (!_curSel->setRange(begin, end)) {
				// detected collision
//				wxLogDebug("       detected collision left");
				txtitr_t col = _curSel->getCollision();
				++col;
				_curSel->setRange(col, end);
				return;
			}
//			wxLogDebug("left");
		}
		if (_curSel && selChanged && !_selSimple) _curSel->define();
	}
}

