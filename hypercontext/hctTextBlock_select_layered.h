#ifndef hctTextBlock_select_layered_H
#define hctTextBlock_select_layered_H

#ifndef hctTextBlock_select_H
#include "hctTextBlock_select.h"
#endif

//#include <vector>
//using namespace std;

/// A block of selected text, with layers (is-a hctTextBlock_select)
class hctTextBlock_select_layered : public hctTextBlock_select {
public:
	hctTextBlock_select_layered(text_t *text);
	virtual ~hctTextBlock_select_layered();

	virtual void paint(wxDC &dc, wxRect rect, bool startsLayer);

	/////////
	virtual bool isReselect(txtitr_t atom);
	virtual void cycleLayers();
	virtual void addLayer(txtitr_t pos);
	virtual void resetNumLayersPainted() {
		_numLayersPainted = 0;
	}
	virtual wxString getLayerText();
	virtual int getNumLayers() {
		return _numLayers;
	}
	virtual void unlayer();
	virtual txtitr_t getEnd();
	virtual txtitr_t getEnd(int layer);

private:

	txtitr_t _endReselect;
//	typedef vector<txtitr_t> layers_t;
//	typedef layers_t::iterator litr_t;
//	layers_t _layers;

	int _numLayersPainted;
	int _numLayers;
	int _curLayer;

	/// finds text that needs attaching or detaching, and does only the required operations
//	virtual void updateTextAttachment();
//	virtual void attachText(txtitr_t start, txtitr_t end);
//	virtual void detachText(txtitr_t start, txtitr_t end);


};

#endif