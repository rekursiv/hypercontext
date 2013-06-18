#ifndef hctTextManager_H
#define hctTextManager_H

#ifndef hctTextAtom_H
#include "hctTextAtom.h"
#endif

#ifndef hctGlyphStackerACIP_H
#include "hctGlyphStackerACIP.h"
#endif

//#include <vector>
#include <list>
using namespace std;
typedef list<hctTextAtom> text_t;
typedef text_t::iterator txtitr_t;


class hctTextWindow;


class hctRow
{
public:
	wxRect getRect() {
		return _rect;
	}
	void setRect(wxRect rect) {
		_rect = rect;
	}

	void setIndex(int i) {
		_index = i;
	}
	int getIndex() {
		return _index;
	}

	void setAtomBegin(txtitr_t atom) {
		_atomBegin = atom;
	}
	txtitr_t getAtomBegin() {
		return _atomBegin;
	}
	void setAtomEnd(txtitr_t atom) {
		_atomEnd = atom;
	}
	txtitr_t getAtomEnd() {
		return _atomEnd;
	}

private:
	wxRect _rect;
	int _index;
	txtitr_t _atomBegin;
	txtitr_t _atomEnd;
};

typedef std::list<hctRow> row_t;

/**
This class holds and manages a list of hctTextAtoms
*/
class hctTextManager
{

public:
	hctTextManager(hctTextWindow* ownerWin);
	virtual ~hctTextManager();

	void addTextAtom(hctTextAtom &atom);
	void addTextAtomAt(txtitr_t pos, hctTextAtom &atom);
	bool deleteTextAtomAt(txtitr_t pos);

	/// deletes all text and re-initializes variables
	virtual void clear();

	/// arranges all text atoms
	int arrangeText(int clientWidth);

	text_t* getText() {
		return &_text;
	}

	void moveText(txtitr_t srcBegin, txtitr_t srcEnd, txtitr_t dest) {
		_text.splice(dest, _text, srcBegin, srcEnd);
	}

	wxPoint getTextStartPt() {
		return wxPoint(_margin, _margin/2);
	}

	void paint(wxPaintDC &dc);

	/// find rows currently visible in window
	/**
	sets the following member variables:
	_visTxtRange_start - the first visible text atom
	_visTxtRange_end - the last visible text atom
	_visRowRange_start - the first visible row
	_visRowRange_end - the last visible row
	*/
	void calcVisibleTextRange(wxRect view);

	/// find the text atom at position pos
	txtitr_t findTextAt(wxPoint pos);

	bool getTextWasFound() {
		return _textWasFound;
	}

	void setWylieMode(bool iswm) {
		if (_noTibetanFonts) return;
		_textInWylieMode = iswm;
		_textSwitchModes = true;
	}
	bool getWylieMode() {
		return _textInWylieMode;
	}
	void toggleWylieMode() {
		setWylieMode(!_textInWylieMode);
	}

	row_t *getRows() {
		return &_row;
	}

	int getNumRows() {
		return _row.size();
	}

	hctGlyphStackerACIP* getGlyphStacker() {
		return &_glyphStacker;
	}

	void makeTibetan(txtitr_t begin, txtitr_t end);

	txtitr_t getTopVisibleAtom() {
		return _visTxtRange_start;
	}



protected:
	virtual void init();

	void endRow(txtitr_t atom, rowitr_t row, int rowHeight, int rowTop);
	void startRow(txtitr_t atom);

	text_t _text;
	row_t _row;

	txtitr_t _visTxtRange_start;
	txtitr_t _visTxtRange_end;
	rowitr_t _visRowRange_start;
	rowitr_t _visRowRange_end;

	bool _textInWylieMode;
	bool _textSwitchModes;
	bool _noTibetanFonts;

	int _margin;
	int _spaceBetweenParagraphs;

	bool _textWasFound;

	bool _needsFullRefresh;

	hctTextWindow* _ownerWin;
	hctGlyphStackerACIP _glyphStacker;

};

#endif

