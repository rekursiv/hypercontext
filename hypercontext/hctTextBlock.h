#ifndef hctTextBlock_H
#define hctTextBlock_H

#ifndef hctTypes_H
#include "hctTypes.h"
#endif

#include <list>
using namespace std;

class hctTextAtom;
typedef list<hctTextAtom> text_t;
typedef text_t::iterator txtitr_t;

/// An abstract base class for blocks of text
/** 
	This class forms the foundation for derived classes that keep track of
	blocks of text.  A block of text is defined by the "start" and "end" index
	(text between must be contiguous.)  A block with start=end=i contains a
	single text atom at index i.
*/
class hctTextBlock
{
public:

	///  this class needs direct access to the text it is responsible for
	hctTextBlock(text_t *text);

	///  this class deletes memory -- it would take some work to make it copyable
	///  TODO:  update docs, don't think the above is true anymore...  could be useful to allow copying...
	virtual ~hctTextBlock();

	/// Sets the range of text (start MUST be on or behind end)
	virtual bool setRange(txtitr_t start, txtitr_t end);

	virtual bool checkRange(txtitr_t &start, txtitr_t &end);


	virtual txtitr_t getStart();
	virtual txtitr_t getEnd();
	virtual txtitr_t getCollision();

	virtual void setEnd(txtitr_t end) {
		setRange(_start, end);
	}
	virtual void setStart(txtitr_t start) {
		setRange(start, _end);
	}

	virtual void incEnd();  // TODO:  make recursive for link
	virtual void decEnd();  //  ???

	virtual int getSize();

	///  Get language of the text in this block
	/** 
		This function does not try to account for the fact that the block
		could contain more than one language - it just returns the language
		at the start of the block.  
	*/
	virtual hctLanguage_t getLanguage();

	/// returns a string of text from this block
	virtual wxString getText();
	virtual wxString getText(txtitr_t end);

	/// call this to get all text in the block ready to paint
	virtual void refresh();

	virtual void paint(wxDC &dc, wxRect rect);

	virtual void setMarked(bool m) {
		_marked = m;
	}
	virtual bool getMarked() {
		return _marked;
	}

	virtual void prepareForDelete(const hctTextAtom* const atom);

	virtual bool isEmpty() {
		return _empty;
	}

	virtual void define();

	/// compares the text in two blocks
	/** 
		Tibetan searches are case-sensitive, English are not.
	*/
//	bool operator==(hctTextBlock& rhs);
	bool textMatches(wxString text, hctLanguage_t language);


//	txtitr_t _start, _start_prev, _end, _end_prev;		/////////test
protected:

	//@{@name- These need to be implemented in your derived classes
	/// called when the start or end of the block changes
	virtual bool updateTextAttachment() = 0;
/*
	/// called when text gets added to the block
	virtual bool attachText(txtitr_t start, txtitr_t end) = 0;

	/// called when text gets removed from the block
	virtual void detachText(txtitr_t start, txtitr_t end) = 0;
	//@}
*/
	txtitr_t _start, _start_prev, _end, _end_prev, _collision;
	text_t *_text;
	int _size;

	bool _marked;
	bool _empty;

private:
	hctTextBlock(const hctTextBlock&);  ///< no copying allowed
	hctTextBlock& operator=(const hctTextBlock& rhs);  ///< no assignment allowed

};
#endif