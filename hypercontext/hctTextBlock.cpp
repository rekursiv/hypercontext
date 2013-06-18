#include "stdwx.h"
#include "hctTextBlock.h"
#include "hctTextAtom.h"
#include "hctApp.h"
#include "hctRootWindow.h"
//#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctTextBlock::hctTextBlock(text_t *text)
{
	_text = text;
	_empty = true;

	_start = _text->begin();
	_end = _text->begin();
	_start_prev = _text->begin();
	_end_prev = _text->begin();
	_collision = _text->begin();

	_size = 0;
	_marked = false;
}

hctTextBlock::~hctTextBlock()
{
//	if (_start < 0 || _end < 0) return;
//	detachText(_start, _end);
}

void hctTextBlock::define() {
	if (getLanguage() == hctLANG_TIBETAN) {
		wxGetApp().getRootWindow()->define(getText());
	}
}

void hctTextBlock::prepareForDelete(const hctTextAtom* const atom) {
	bool deletedFirst = false;

	if (atom == &(*_start)) {
//		wxLogDebug("deleting first");
		deletedFirst = true;
		++_start;
	}
	if (atom == &(*_end)) {
		if (deletedFirst) {  // deleting last atom in range
			_start = _text->begin();
			_end = _text->begin();
			_empty = true;
		}
		else {
			--_end;
		}
	}
}


void hctTextBlock::incEnd() {
	++_end;
}
void hctTextBlock::decEnd() {
	--_end;
}

txtitr_t hctTextBlock::getStart() {
	if (_empty) return _text->begin();
	else return _start;
}

txtitr_t hctTextBlock::getEnd() {
	if (_empty) return _text->begin();
	else return _end;
}

txtitr_t hctTextBlock::getCollision() {
	return _collision;
}

bool hctTextBlock::checkRange(txtitr_t &start, txtitr_t &end) {
	for (txtitr_t i=_text->begin(); i!=start; ++i) {
		if (i == end) {
			wxLogDebug("!!!!!! (hctTextBlock)  range check failed:  start ( %i ) is ahead of end ( %i ) !!!!!!", start->getIndex(), end->getIndex());
			start = _text->begin();
			return false;
		}
	}
	return true;
}

bool hctTextBlock::setRange(txtitr_t start, txtitr_t end)
{
//	wxLogDebug("hctTextBlock::setRange()  %i  %i", i1, i2);
#ifdef __WXDEBUG__
	checkRange(start, end);  /////////////////  SLOW!  (use for debugging only)
#endif
	
	_start = start;
	_end = end;
	_empty = false;

	// if nothing needs updating, we can bail here
	if (_start_prev == _start && _end_prev == _end) return true;

	bool success = updateTextAttachment();

	_start_prev = _start;
	_end_prev = _end;

//	return false;
	return success;  // returns "false" if range collided with another selection
}

void hctTextBlock::refresh()
{
	if (_empty) return;
	txtitr_t e = _end; ++e;
	for (txtitr_t i=_start; i!=e; ++i) {
		i->refresh();
	}
}

int hctTextBlock::getSize() {
	if (_empty) return 0;
	return distance(_start, _end);  // TODO: optimize
}


hctLanguage_t hctTextBlock::getLanguage()
{
	return _start->getLanguage();
}


wxString hctTextBlock::getText() {
	return getText(_end);
}

wxString hctTextBlock::getText(txtitr_t end) {
	if (_empty) return "";
	wxString txt;
	txtitr_t e = end; ++e;
	for (txtitr_t i=_start; i!=e; ++i) {
		txt << i->getLetters();
	}
	return txt;
}


void hctTextBlock::paint(wxDC &dc, wxRect rect) {

	// add underlining
	if (_marked) {
		wxPen *pen = wxThePenList->FindOrCreatePen(wxColour(233, 0, 0), 4, wxSOLID);
		pen->SetCap(wxCAP_BUTT);
		dc.SetLogicalFunction(wxCOPY);
		dc.SetPen(*pen);
		//		dc.DrawLine(rect.GetLeft(), rect.GetTop()+2, rect.GetRight()+1, rect.GetTop()+2);
		dc.DrawLine(rect.GetLeft(), rect.GetBottom()-1, rect.GetRight()+1, rect.GetBottom()-1);
		dc.SetPen(wxNullPen);
	}
}

/*
bool hctTextBlock::operator==(hctTextBlock& rhs) {
	if (getLanguage() == rhs.getLanguage()) {
		bool caseSensitive = (getLanguage() == hctLANG_TIBETAN);
		if (getText().IsSameAs(rhs.getText(), caseSensitive)) return true;
	}
	return false;
}
*/
bool hctTextBlock::textMatches(wxString text, hctLanguage_t language) {
	if (getLanguage() == language) {
		bool caseSensitive = (language == hctLANG_TIBETAN);
		if (getText().IsSameAs(text, caseSensitive)) return true;
	}
	return false;
}