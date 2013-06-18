#include "stdwx.h"
#include "hctTextBlock_link.h"
#include "hctTextAtom.h"
#include "hctHyperlink.h"
#include "hctApp.h"
#include "hctRootWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctTextBlock_link::hctTextBlock_link(text_t *text) : hctTextBlock(text) {
	_link = NULL;
	_next = NULL;
	_styleSelect = false;
	_styleFocus = false;

	_wordCont = false;
	_wordInfreq = false;
	_wordSyn = false;;
}

hctTextBlock_link::~hctTextBlock_link() {
	detachText(_start, _end);
}

void hctTextBlock_link::define() {
	if (getLanguage() == hctLANG_TIBETAN) {
		wxGetApp().getRootWindow()->define(getText(), true);
	}
}

bool hctTextBlock_link::updateTextAttachment() {
	detachText(_start_prev, _end_prev);
	attachText(_start, _end);
	return true;  //??
}


void hctTextBlock_link::attachText(txtitr_t start, txtitr_t end) {
	if (_empty) return;
//	wxLogDebug("hctTextBlock_link::attachText  %i  ->  %i", start, end);
//	if (start < 0 || end < 0) return;
	hctTextBlock_link* block;
	txtitr_t e = end; ++e;
	for (txtitr_t i=start; i!=e; ++i)
	{
		block = addBlock(i->getLinkBlock());
		i->setLinkBlock(block);
		i->refresh();
	}
//	_start->setStartsLink(true);
//	if (_end != _text->end()) _end->setEndsLink(true);
}


hctTextBlock_link* hctTextBlock_link::addBlock(hctTextBlock_link* topBlock) {
	if (topBlock == NULL) return this;   // I will become new top block
	hctTextBlock_link* prevBlock = NULL;
	hctTextBlock_link* nextBlock = topBlock;
	while (nextBlock) {
		if (nextBlock == this) return topBlock;  // found duplicate block, leave the list alone
		if (nextBlock->getSize() > getSize()) break;
		prevBlock = nextBlock;
		nextBlock = nextBlock->_next;
	}
	this->_next = nextBlock;  // link me to the next block
	if (prevBlock) {  // if I'm not already on top,
		prevBlock->_next = this;   // link block above me to myself
		return topBlock;    // head of list hasn't changed
	}
	else return this;  // I will become new head of list
}


void hctTextBlock_link::detachText(txtitr_t start, txtitr_t end) {
	if (_empty) return;
//	wxLogDebug("hctTextBlock_link::detachText  %i  ->  %i", start, end);
//	if (start < 0 || end < 0) return;
	hctTextBlock_link *block, *aboveMe;
	bool loop = true;

	txtitr_t e = end; ++e;
	for (txtitr_t i=start; i!=e; ++i) {
		if (i == end) loop = false;
		block = i->getLinkBlock();
		if (block == this) {  // atom has my pointer
			i->setLinkBlock(_next);
		}
		else {  // atom has someone else's pointer
			//  find block directly on top of this one
			aboveMe = block;
			int loopCount = 0;
			while (aboveMe && aboveMe->_next != this) {
				aboveMe = aboveMe->_next;
				loopCount++;
				if (loopCount > 100) {
					wxLogDebug("    !!!!!  WARNING:   hctTextBlock_link::detachText():  Circular linking found!  i=%i", i->getIndex());
					return;
				}
			}
			// now "bypass" this block by linking around it
			if (aboveMe) aboveMe->_next = _next;
		}
		i->refresh();
	}

}

//void hctTextBlock_link::move(txtitr_t dest) {
//	txtitr_t end = _end;  // TODO: handle special case of EOF
//	_text->splice(dest, *_text, _start, ++end);
//}

void hctTextBlock_link::paint(wxDC &dc, wxRect rect) {
	// the correct block to paint may be further down the list
	if (!_styleFocus || !_styleSelect) {
		if (_next) _next->paint(dc, rect);
	}

	// let parent class paint
	hctTextBlock::paint(dc, rect);


	// set font colors
	if (_link && _link->getNumBlocks() == 1) {
		dc.SetTextForeground(wxColor(100, 100, 100));
//		dc.SetTextForeground(wxColor(170, 20, 0));
	}
//	else dc.SetTextForeground(wxColor(0, 0, 0));
	else dc.SetTextForeground(wxColor(0, 70, 150));


	// set background colors
//	dc.SetBackgroundMode(wxTRANSPARENT);
//	dc.SetLogicalFunction(wxAND);
	dc.SetPen(*wxTRANSPARENT_PEN);

	if (_styleFocus) {
		if (_next) dc.SetBrush(wxBrush(wxColor(170, 255, 170, 200), wxSOLID));
		else dc.SetBrush(wxBrush(wxColor(180, 250, 250, 200), wxSOLID));
		dc.DrawRectangle(rect);
	}
	else if (_styleSelect) {
		dc.SetBrush(wxBrush(wxColor(240, 240, 200, 200), wxSOLID));
//		if (_next) dc.SetBrush(wxBrush(wxColor(230, 210, 130), wxSOLID));
//		else dc.SetBrush(wxBrush(wxColor(230, 210, 230), wxSOLID));
//		dc.SetBrush(wxBrush(wxColor(180, 250, 250), wxSOLID));
		dc.DrawRectangle(rect);
	}

}


void hctTextBlock_link::debugLinkedList() {
	wxLogDebug("hctTextBlock_link::debugLinkedList()");	
	wxLogDebug("  %i", getSize());
	int depth = 1;
	hctTextBlock_link* curBlock = _next;
	while (curBlock) {
		wxLogDebug("  %i", curBlock->getSize());
		depth++;
		curBlock = curBlock->_next;
	}
	wxLogDebug("  depth=%i", depth);
}
