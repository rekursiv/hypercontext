#ifndef hctSelectionManager_H
#define hctSelectionManager_H

//#include "stdwx.h"  //??


#ifndef hctTextManager_H
#include "hctTextManager.h"
#endif



#define hctMAX_SELECTORS 21
class hctTextSelector;
class hctTextBlock_link;
class hctHyperlink;

class hctSelectionManager
{
public:
	hctSelectionManager(text_t *text);
	~hctSelectionManager();

	/// clears selected status of all text
	void clear();

	/// begins the whole text selection process
	/**
	This function does many different things, depending on the context.
	In most cases it selects a single word in any one language.  It keeps
	track of which languages have a selected word, and allows for multiple
	selections only across languages.  (Set the multi arg to "true" for 
	multiple selections in the same language.)  This function can also start
	a "drag select" if activated over an existing selection.
	*/
	void begin(txtitr_t atom, bool multi);
	void drag(txtitr_t atom, wxPoint pos);
	void end();

	void beginSimple();

	/// adds selector to the selection list
	void addSelector(txtitr_t atom);

	void addSelector(hctTextBlock_link *block);

	/// deletes selector from the selection list
	void delSelector();

	void cycleLayers();
	void nextWord();
	void prevWord();

	int getNumSelectors() {
		return _numSelectors;
	}

	bool tibetanSelectorInUse();

	void linkSelected(hctHyperlink *link);

	bool isValidForLinking();

	bool isSelecting() {
		return _selectingText;
	}

	void paint(void);

	void copy();
	void getPrevCopy(txtitr_t &clipBegin, txtitr_t &clipEnd) {
		clipBegin = _clipBegin;
		clipEnd = _clipEnd;
	}
	void getRange(txtitr_t &begin, txtitr_t &end);

	void debug();

private:
	void freeSelectorMemory();

	text_t *_text;
	hctTextSelector *_selection[hctMAX_SELECTORS]; 
	int _curSelector;
	int _numSelectors;
	bool _selectingText;
	bool _selectingTextSimple;
	
	txtitr_t _clipBegin;
	txtitr_t _clipEnd;
};

#endif