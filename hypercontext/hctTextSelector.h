#ifndef hctTextSelector_H
#define hctTextSelector_H

#define hctMAX_WORD_END_PTS 8

#ifndef hctTextBlock_select_layered_H
#include "hctTextBlock_select_layered.h"
#endif

#ifndef hctTextBlock_link_H
#include "hctTextBlock_link.h"
#endif

#ifndef hctTypes_H
#include "hctTypes.h"
#endif

class hctTextAtom;


///  Handles text selection duties
/**
	Tries to select whole words, but also allows for selection of punctuation.  An existing selection
	can be modified by clicking on it and then dragging outside of it, thus "grabbing" the edge of the 
	selection.  For example, if you dragged outside the left part of the selection, the left side of that
	selection would start to follow your mouse pointer, and you could make the selection bigger or smaller
	from the left side.  (It would work in the same way on the right.)  A more "traditional" selection routine
	is also implemented for "Text Edit" mode.

	If the clicked-on word (syllable, actually) is Tibetan, this class will attempt to find all words that
	are in the dictionary from that syllable onward (look-ahead distance = hctMAX_WORD_END_PTS,) and if there
	are any multi-syllable words found, they are stored in the class.  The user can then cycle through all 
	the words found by clicking on the syllable multiple times.

*/
class hctTextSelector
{
public:

	///  this class needs direct access to the text it is selecting
	hctTextSelector(text_t *text);

	///  this class deletes memory -- it would take some work to make it copyable
	~hctTextSelector();

	///  starts the text selection (usually on left mouse click)
	bool begin(txtitr_t atom);
	void beginSimple(txtitr_t atom, wxPoint pos);

	///  called when dragging the mouse on a started link
	void drag(txtitr_t atom, wxPoint pos);

	void reSelect();

	///  resets the class
	void reset();

	///  returns the language that is being selected 
	hctLanguage_t getLanguage();

	/// returns true if the selection area is invalid or cannot be displayed
	bool isBlank();

	///  returns true if any part of the passed-in block overlaps this selection
	bool overlaps(hctTextBlock_select *block) {
		if (!block) return false;
		if (block == _curSel) return true;
		return false;
	}

	///  returns true if the passed-in block is directly on top of this selection
	bool duplicates(hctTextBlock *block);

	///  takes a multi-word Tibetan block and converts it into a single selection
	void freeze();

	///  used to convert a link block into a select block (when unlinking)
	void setLinkBlock(hctTextBlock_link *block);

	///  used to link selected text
	hctTextBlock_link *getLinkBlock();

	hctTextBlock_select *getSelectBlock() {
		return _curSel;
	}

	void paint() {
		if (_curSel) _curSel->resetNumLayersPainted();
	}

	wxString getText() {
		if (_curSel) return _curSel->getText();
		else return wxString();
	}

	/// checks to see if this selection can be linked
	/**
		This function MUST be called before turning a selection into a link.
		Otherwise, text that is overlapping in the wrong way (an "invalid" selection)
		could be turned into a link block, resulting in a crash.
		@see hctTextBlock_link
	*/
	bool isValidForLinking();

	void cycleLayers();
	void nextWord();
	void prevWord();


private:

	/// selects an entire word (or "word possibilities" if text is Tibetan)
	/**
	If the mouse is clicked over an English word, a single word is selected.
	If clicked over a Tibetan syllable, that syllable and several others to
	the right of it are searched for in the dictionary.  Clicking on the same
	word again cycles through the words found, if any.
	*/
	bool selectWord(txtitr_t atom);

	/// breakpoints are spaces, ctrl chars, and puctuation
	bool checkForBreakPoint(txtitr_t atom);

	/// returns "true" if the atom could be an ending particle
	bool isParticle(txtitr_t atom);

	/// find where a word begins (uses checkForBreakPoint)
	txtitr_t findStartPt(txtitr_t atom);

	/// find where a word ends (uses checkForBreakPoint)
	txtitr_t findEndPt(txtitr_t atom);

	txtitr_t findNextEndPt(txtitr_t atom);

	void findLayers();


	text_t *_text;
	hctTextBlock_select *_curSel;

	bool _selecting;
	bool _selSimple;
	txtitr_t _selAnchor;


	hctTextSelector(const hctTextSelector&);  ///< no copying allowed
	hctTextSelector& operator=(const hctTextSelector& rhs);  ///< no assignment allowed
};

#endif

