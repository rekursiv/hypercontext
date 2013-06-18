#ifndef hctTextAtom_H
#define hctTextAtom_H


#include <wx/wfstream.h>

#ifndef hctTypes_H
#include "hctTypes.h"
#endif

class hctTextBlock;
class hctTextBlock_select;
class hctTextBlock_link;
class hctTextWindow;

class hctRow;
#include <list>
typedef std::list<hctRow>::iterator rowitr_t;

static const int hctTIB_GLYPHS_MAX = 5;

class hctTextBlock;

/// A multi-language self-contained unit of text and associated properties
/**
	A text atom is defined as the minimum amount of text that you might want to select
	with the mouse.  With normal English letters, this is a single character.  In Tibetan,
	it could consist of one or more glyphs.  If you have a stack of Tibetan glyphs, it is
	much easier to work with the entire stack as a single unit.  If the Tibetan is represented
	with Wylie codes, a single atom could be several characters wide.  A text atom can also 
	be a control code (line break or paragraph), in which case it contains no characters.

	Note that this class contains a pointer to a hctTextBlock_link, and a pointer to a
	hctTextBlock_select.  This means that select blocks and link blocks can freely overlap
	eachother with no restrictions.  Select blocks can never overlap, and the attachSelectBlock()
	and detachSelectBlock() methods enforce this rule.  Link blocks can overlap, and the pointer
	here always points to the topmost block.  The blocks beneath can be accessed by using the
	hctTextBlock_link::getNext() method in classic linked-list style.
	@see hctTextBlock_select
	@see hctTextBlock_link

	@todo
	User-definable fonts, colors, etc.  These should be in a class (hctTextProperties?) that
	can be accessed from all hctTextAtom instances (or globally.)
*/
class hctTextAtom
{
public:
	hctTextAtom();
	~hctTextAtom();

	void prepareForDelete();

	void setHasInsertMarker(bool has) {
		_hasInsertMarker = has;
	}

	void resetDefaults();

	//@{@name- block management

	bool attachSelectBlock(hctTextBlock_select* block) {
		if (_selectBlock) return false;
		_selectBlock = block;
		return true;
	}
	bool detachSelectBlock(hctTextBlock_select* block)
	{
		if (block!=_selectBlock) return false;
		_selectBlock = NULL;
		return true;
	}
	hctTextBlock_select* getSelectBlock()
	{
		return _selectBlock;
	}

	void setLinkBlock(hctTextBlock_link* block)
	{
		_linkBlock = block;
	}
	hctTextBlock_link* getLinkBlock()
	{
		return _linkBlock;
	}

	hctTextBlock* getBlock();
	//@}


	//@{@name- text setup and access
	void setTibetanGlyphs(unsigned short *codes)
	{
		for (int i=0; i<hctTIB_GLYPHS_MAX; i++) _tibetanGlyphs[i]=codes[i];
		_unstackable = false;
		_renderWylie = false;
	}
	void setLetters(wxString letters)
	{
		_letters = letters;
	}
	wxString getLetters()
	{
		return _letters;
	}

	void setWylieDisplayMode(bool isWylie)
	{
		if (isWylie) {
			_renderWylie = true;
		}
		else if (!_unstackable && !_wylieEditMode) {
			_renderWylie = isWylie;
		}
		_wylieDisplayMode = isWylie;
		calcSize();
	}
	bool getWylieDisplayMode()
	{
		return _wylieDisplayMode;
	}

	void setLanguage(hctLanguage_t language)
	{
		_language = language;
	}
	hctLanguage_t getLanguage()
	{
		return _language;
	}

	void setIndex(int index) {
		_index = index;
	}
	int getIndex() {
		return _index;
	}
	void setRow(rowitr_t index) {
		_row = index;
	}
	rowitr_t getRow() {
		return _row;
	}
	void setColumn(int index) {
		_column = index;
	}
	int getColumn() {
		return _column;
	}
	//@}

	//@{@name- control properites
	bool isWordWrapPnt()
	{
		return (_letters.at(0) == ' ' || isCtrl());
	}

	void setCtrlChar(hctCtrlChar_t cc)
	{
		_ctrlChar = cc;
		if (cc != hctCTRL_NONE) {
			_letters = " ";
			_tibetanGlyphs[0] = 0xA0;
		}
	}
	hctCtrlChar_t getCtrlChar()
	{
		return _ctrlChar;
	}

	bool isCtrl()
	{
		return (_ctrlChar != hctCTRL_NONE);
	}
	//@}


	//@{@name-  bounding rect manipulation
	wxRect getRect()
	{
		return _rect;
	}

	void calcSize();
	int getWidth()
	{
		return _rect.width;
	}

	void setHeight(int h)
	{
		_rect.height = h;
	}
	int getHeight()
	{
		return _rect.height;
	}

	void setPos(wxPoint pos)
	{
		_rect.x = pos.x;
		_rect.y = pos.y;
	}
	wxPoint getPos() {
		return _rect.GetPosition();
	}

	bool isInside(wxPoint pos)
	{
		return _rect.Contains(pos);
	}
	bool isInsideX(int x)
	{
		wxPoint pos = _rect.GetPosition();
		pos.x = x;
		return isInside(pos);
	}
	bool isRightSide(wxPoint pos)
	{
		int center = _rect.x+_rect.width/2;
		return (pos.x > center);
	}
	//@}



	//@{@name- painting

	/// This needs to be set before using the class (used by refresh code)
	void setOwnerWin(hctTextWindow* win);

	void setNeedsRefresh(bool n) {
		_needsRefresh = n;
	}

	void refresh();

	void paint(wxDC &dc);
	//@}

	void setStartsLayer(bool s) {
		_startsLayer = s;
	}
	bool getStartsLayer() {
		return _startsLayer;
	}

	bool isNote() {
		return _isNote;
	}
	void setIsNote(bool is) {
		_isNote = is;
	}

	void setFontSize(int size) {
		_fontSize = size;
	}
	int getFontSize() {
		return _fontSize;
	}

	void debug();  ///<  temp debugging code


private:
/*
	static wxFont defaultFont;
	static wxFont wylieFont;
	static wxFont tibFont;
	static wxFont debugFont;
*/

	void prepareFonts(wxDC &dc);  ///<  must be called before calculating width or painting

	bool _showCtrl;

	hctTextWindow* _ownerWin;

	wxString _letters;   ///<  One English letter or one or more Wylie codes
	char _tibetanGlyphs[hctTIB_GLYPHS_MAX];  ///<  a single "stack" of Tibetan glyphs
	int _fontSize;

	hctLanguage_t _language;

	bool _needsRefresh;

	bool _renderWylie;
	bool _wylieDisplayMode;
	bool _wylieEditMode;
	bool _unstackable;

	bool _hasInsertMarker;

	bool _startsLayer;
	bool _isNote;

	wxRect _rect;

	hctCtrlChar_t _ctrlChar;

	hctTextBlock_select* _selectBlock;
	hctTextBlock_link*	 _linkBlock;

	rowitr_t _row;

	int _index, _column;
};

#endif
