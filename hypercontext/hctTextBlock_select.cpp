#include "stdwx.h"
#include "hctTextBlock_select.h"
#include "hctTextAtom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctTextBlock_select::hctTextBlock_select(text_t *text) : hctTextBlock(text) {

}
hctTextBlock_select::~hctTextBlock_select() {
//	wxLogDebug("~hctTextBlock_select()");
	detachText(_start, _end);
}


bool hctTextBlock_select::updateTextAttachment() {

//	detachText(_start_prev, _end_prev);
//	return attachText(_start, _end);//////////////////////////////////////////////////////////////////////////
	

	if (_empty) return true;
	if (_start_prev == _start && _start_prev != _end_prev) {
		bool found = false;
		txtitr_t overlap;
		for (txtitr_t i=_end; i!=_start; --i) {
			if (i == _end_prev) {
				overlap = i;
				found = true;
				break;
			}
		}
		if (found) {
			++overlap;
			if (overlap != _text->end()) return attachText(overlap, _end);
		}
		else {
			txtitr_t s = _end;
			txtitr_t e = _end_prev;
			++s;
			++e;
			if (s == _text->end()) --s;
			if (e == _text->end()) --e;
			detachText(s, e);
		}
	}
	else if (_end_prev == _end && _start_prev != _end_prev) {
		bool found = false;
		txtitr_t overlap;
		for (txtitr_t i=_start; i!=_end; ++i) {
			if (i == _start_prev) {
				overlap = i;
				found = true;
				break;
			}
		}
		if (found) {
			--overlap;
			return attachText(_start, overlap);
		}
		else {
			txtitr_t s = _start_prev;
			txtitr_t e = _start;
			if (s != _text->begin()) --s;
			if (e != _text->begin()) --e;
			detachText(s, e);
		}
	}
	else {
		detachText(_start_prev, _end_prev);
		return attachText(_start, _end);
	}

	return true;
}

bool hctTextBlock_select::attachText(txtitr_t start, txtitr_t end) {
	if (_empty) return true;
	txtitr_t e = end; ++e;
	for (txtitr_t i=start; i!=e; ++i) {
		if (!i->attachSelectBlock(this)) {
			_collision = i;
			return false;
		}
		i->refresh();
	}
	return true;
}

void hctTextBlock_select::detachText(txtitr_t start, txtitr_t end) {
	if (_empty) return;
	txtitr_t e = end; ++e;
	for (txtitr_t i=start; i!=e; ++i) {
		i->detachSelectBlock(this);
		i->refresh();
	}
}

void hctTextBlock_select::paint(wxDC &dc, wxRect rect, bool startsLayer) {

	// let parent class paint
	hctTextBlock::paint(dc, rect);

	// set background colors
//	dc.SetLogicalFunction(wxAND);
	dc.SetPen(*wxTRANSPARENT_PEN);

	dc.SetBrush(wxBrush(wxColor(200, 200, 234, 200), wxSOLID));
	dc.DrawRectangle(rect);
	
//	dc.SetLogicalFunction(wxCOPY);

}