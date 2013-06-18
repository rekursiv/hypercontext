#ifndef hctTextBlock_link_H
#define hctTextBlock_link_H
#include "hctTextBlock.h"

class hctHyperlink;

/// A block of text that is used as part of a link (is-a hctTextBlock)
/** 
	This class has a close relationship to both hctTextAtom and hctHyperlink.
	Every text atom has a pointer to a hctTextBlock_link, and this class has a 
	pointer to a hctHyperlink.  This means that if we have access to a single 
	text atom, we also can get at any other text atom in the link, if there are any.
	This is very handy for lighting up a link under the mouse pointer, for example.
	
	This class automatically handles nested blocks (link blocks can have other 
	link blocks inside them.) There are some restrictions to how the blocks can overlap.
	As long as one block is completely inside another and they are not the same size, 
	everything is lovely.  Of course, to make sure the user has access to all the links, 
	even ones stacked on top of eachother, the smallest blocks must be considered to be 
	"on top of" the larger ones.  To implement this, this class contains a built-in 
	linked list, and each block is sorted smallest-to-largest as they are added and
	found to overlap.

	WARNING:  This class is very fragile when it comes to overlapping links!  There
	is a function for checking this: hctTextSelector::isValidForLinking() This MUST be
	used to make sure that two overlapping links can be nested.  If they overlap
	in the wrong way this class leaves dangling pointers everywhere and the app
	WILL crash very soon thereafter!

\verbatim
	Here are the rules for overlapping blocks: (+ block1   x block2   * both)
	***xxxxx	valid - block1 on block2
	xxxxx***	valid - block1 on block2
	xx***xxx	valid - block1 on block2
	***+++++	valid - block2 on block1
	+++++***	valid - block2 on block1
	++***+++	valid - block2 on block1
	++***xxx	INVALID - blocks overlap, but neither one fully contains the other
	********	INVALID - blocks are both the same size.
\endverbatim
*/
class hctTextBlock_link : public hctTextBlock {
public:
	hctTextBlock_link(text_t *text);
	virtual ~hctTextBlock_link();

	virtual void paint(wxDC &dc, wxRect rect);

	virtual void define();

	/// focus is used for mouseover effect
	void setStyleFocus(bool s) {
		_styleFocus = s;
	}

	/// currently only used for "find text in link" feature
	bool getStyleSelect() {
		return _styleSelect;
	}
	void setStyleSelect(bool s) {
		_styleSelect = s;
	}

	void setLink(hctHyperlink* link) {
		_link = link;
	}
	hctHyperlink* getLink() {
		return _link;
	}

	hctTextBlock_link* getNext() {
		return _next;
	}

	virtual void prepareForDelete(const hctTextAtom* const atom) {
		if (_next) _next->prepareForDelete(atom);
		hctTextBlock::prepareForDelete(atom);
	}

	virtual void incEnd() {
		if (_next && _next->_end == _end) _next->incEnd();
		hctTextBlock::incEnd();
	}

/*
	virtual void setStart(txtitr_t start) {
		wxLogDebug("setStart");
		if (_next && _next->_start == _start) _next->setStart(start);
		wxLogDebug("setStart2");
		setRange(start, _end);
	}
	virtual void setEnd(txtitr_t end) {
		wxLogDebug("setEnd");
		if (_next && _next->_end == _end) _next->setEnd(end);
		wxLogDebug("setEnd2");
		setRange(_start, end);
	}
*/
	hctTextBlock_link* addBlock(hctTextBlock_link* block);

//	void move(txtitr_t dest);

	void debugLinkedList();

	// TODO:  decide on how this will work...
	bool _wordCont, _wordInfreq, _wordSyn;

private:

	///  detaches all previous text, attaches all new text
	virtual bool updateTextAttachment();
	virtual void attachText(txtitr_t start, txtitr_t end);
	virtual void detachText(txtitr_t start, txtitr_t end);

	bool _styleFocus, _styleSelect;

	hctHyperlink *_link;
	hctTextBlock_link *_next;
};

#endif
