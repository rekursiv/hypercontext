#ifndef hctTextBlock_select_H
#define hctTextBlock_select_H

#ifndef hctTextBlock_H
#include "hctTextBlock.h"
#endif

/// A block of selected text (is-a hctTextBlock)
/** 
	This class has a close relationship to hctTextAtom.  Every text atom has a 
	pointer to a hctTextBlock_select.

	The Tibetan "auto-word-finder" used derived class hctTextBlock_select_layered
*/
class hctTextBlock_select : public hctTextBlock {
public:
	hctTextBlock_select(text_t *text);
	virtual ~hctTextBlock_select();

	virtual void paint(wxDC &dc, wxRect rect, bool startsLayer);

	/////////

	virtual bool isReselect(txtitr_t atom){
//		wxLogDebug("hctTextBlock_select::isReselect()");
		txtitr_t e = _end; ++e;
		for (txtitr_t i=_start; i!=e; ++i) {
			if (i == atom) return true;
		}
		return false;
	}
	virtual void cycleLayers() {}
	virtual void addLayer(txtitr_t pos) {}
	virtual void resetNumLayersPainted() {}
	virtual wxString getLayerText() {return getText();}
	virtual int getNumLayers() {
		return 0;
	}
	virtual void unlayer() {}
	virtual txtitr_t getEnd(int layer) {
		return hctTextBlock::getEnd();
	}
	virtual txtitr_t getEnd() {
		return hctTextBlock::getEnd();
	}

private:

	/// finds text that needs attaching or detaching, and does only the required operations
	virtual bool updateTextAttachment();
	virtual bool attachText(txtitr_t start, txtitr_t end);
	virtual void detachText(txtitr_t start, txtitr_t end);

};

#endif