#include "stdwx.h"
#include "hctTextBlock_select_layered.h"
#include "hctTextAtom.h"

//#include "hctApp.h"
//#include "hctRootWindow.h"
//#include "hctDictTibEng.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctTextBlock_select_layered::hctTextBlock_select_layered(text_t *text) : hctTextBlock_select(text) {
	_endReselect = _text->begin();
	_numLayers = 0;
	_curLayer = 0;
}
hctTextBlock_select_layered::~hctTextBlock_select_layered() {
//	wxLogDebug("~hctTextBlock_select_layered()");
	unlayer();
}


void hctTextBlock_select_layered::addLayer(txtitr_t pos) {
	if (_numLayers == 0) _endReselect = pos;
	pos->setStartsLayer(true);
	_curLayer = _numLayers;
	_numLayers++;
}

void hctTextBlock_select_layered::cycleLayers() {
	if (_curLayer > 0) _curLayer--;
	else _curLayer = _numLayers-1;
	refresh();
//	wxString s; s << "curLayer = " << _curLayer << "    " << getLayerText(); wxLogDebug(s);
}

wxString hctTextBlock_select_layered::getLayerText() {
	if (_empty) return "";
	if (_numLayers == 0) return getText();

	int layer = 0;
	wxString txt;
	txtitr_t e = _end; ++e;
	for (txtitr_t i=_start; i!=e; ++i) {
		if (layer > _curLayer) break;
		if (i->getStartsLayer()) ++layer;
		txt << i->getLetters();
	}
	return txt;
}


txtitr_t hctTextBlock_select_layered::getEnd() {
	return getEnd(_curLayer);
}

txtitr_t hctTextBlock_select_layered::getEnd(int layer) {   //  TESTME
	if (_empty) return _text->begin();
	else if (_numLayers == 0) return hctTextBlock::getEnd();

	int curLayer = 0;
	txtitr_t e = _end; ++e;
	txtitr_t i;
	for (i=_start; i!=e; ++i) {
		if (curLayer > layer) break;
		if (i->getStartsLayer()) ++curLayer;
	}
	return --i;
}

bool hctTextBlock_select_layered::isReselect(txtitr_t atom) {
	if (_numLayers == 0) return hctTextBlock_select::isReselect(atom);
	for (txtitr_t i = _endReselect; ; --i) {
		if (i == atom) return true;
		if (i == _start) return false;
		if (i == _text->begin()) return false;
	}
	return false;
}

void hctTextBlock_select_layered::unlayer() {
	int layer = 0;
	txtitr_t e = _end; ++e;
	txtitr_t newEnd = _end;
	for (txtitr_t i=_start; i!=e; ++i) {
		if (i->getStartsLayer()) {
			i->setStartsLayer(false);
			if (layer <= _curLayer) newEnd = i;
			++layer;
		}
		i->refresh(); 
	}
	if (newEnd != _end) setEnd(newEnd);
	_endReselect = _end;
	_curLayer = 0;
	_numLayers = 0;
	_numLayersPainted = 0;
}

void hctTextBlock_select_layered::paint(wxDC &dc, wxRect rect, bool startsLayer) {

	// let parent class paint
	hctTextBlock::paint(dc, rect);

//	wxString s; s << "NLP = " << _numLayersPainted; wxLogDebug(s);

	if (_numLayersPainted <= _curLayer) 
	{
		// set background colors
//		dc.SetLogicalFunction(wxAND);
		dc.SetPen(*wxTRANSPARENT_PEN);
	
		dc.SetBrush(wxBrush(wxColor(200, 200, 234, 200), wxSOLID));
		dc.DrawRectangle(rect);

//		dc.SetLogicalFunction(wxCOPY);
	}

	if (startsLayer && _numLayers > 0) 
	{
		_numLayersPainted++;
		dc.SetLogicalFunction(wxXOR);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(rect.x+rect.width-1, rect.y, 1, rect.height);
	}

}
