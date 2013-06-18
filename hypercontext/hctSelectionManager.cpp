#include "stdwx.h"
#include "wx/clipbrd.h"

#include "hctSelectionManager.h"
#include "hctHyperlink.h"
#include "hctTextSelector.h"
#include "hctApp.h"   ///////
#include "hctRootFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctSelectionManager::hctSelectionManager(text_t *text) {
	_text = text;

	_selectingText = false;
	_selectingTextSimple = false;
	_curSelector = 0;
	_numSelectors = 0;

	_clipBegin = _text->end();
	_clipEnd = _text->end();
}

hctSelectionManager::~hctSelectionManager() {
	freeSelectorMemory();
}

void hctSelectionManager::freeSelectorMemory() {
	for (int i=0; i<_numSelectors; i++) {
		if (_selection[i]) {
			delete _selection[i];
			_selection[i] = NULL;
		}
	}
}


bool hctSelectionManager::tibetanSelectorInUse() {
	for (int i=_numSelectors-1; i>=0; i--) {
		if (_selection[i]->getLanguage() == hctLANG_TIBETAN) {
			return true;
		}
	}
	return false;
}

void hctSelectionManager::paint() {
	if (_numSelectors == 0) return;
	for (int i=_numSelectors-1; i>=0; i--) {
		if (_selection[i]->getLanguage() == hctLANG_TIBETAN) {
			_selection[i]->paint();
		}
	}
}


void hctSelectionManager::cycleLayers() {
	if (_numSelectors == 0) return;
	int selectorTibetan = -1;
	if (_selection[_curSelector]->getLanguage() == hctLANG_TIBETAN) selectorTibetan = _curSelector;
	else {
		for (int i=_numSelectors-1; i>=0; i--) {
			if (_selection[i]->getLanguage() == hctLANG_TIBETAN) {
				selectorTibetan = i;
				break;
			}
		}
	}
	if (selectorTibetan > -1) _selection[selectorTibetan]->cycleLayers();
}

void hctSelectionManager::nextWord() {
	if (_numSelectors > 0 && _curSelector > -1) {
		_selection[_curSelector]->nextWord();
	}
}
void hctSelectionManager::prevWord() {
	if (_numSelectors > 0 && _curSelector > -1) {
		_selection[_curSelector]->prevWord();
	}
}


//			=== selection stuff  ===   

void hctSelectionManager::clear() {
	freeSelectorMemory();
	_curSelector = 0;
	_numSelectors = 0;
	wxGetApp().getRootWindow()->clearDefText();
}

void hctSelectionManager::beginSimple() {
	clear();
	_selectingTextSimple = true;
}

void hctSelectionManager::begin(txtitr_t atom, bool multi) {
	if (atom == _text->end()) {   // outside of text area
		clear();
		return;
	}
//	else if (atom->getCtrlChar() == hctCTRL_NOTE) {
//	}
	_selectingText = true;  

	// find out if a text selector exists for the current language
	hctLanguage_t curLanguage = atom->getLanguage(); 
	int selectorWithSameLanguage = -1;
	for (int i=_numSelectors-1; i>=0; i--) {
		if (_selection[i]->getLanguage() == curLanguage) {
			selectorWithSameLanguage = i;
			break;
		}
	}

	// add, delete, or move selectors as needed
	hctTextBlock_select* overlap = atom->getSelectBlock();
	if (multi) {  // multi-select mode
		if (overlap) {
			for (int i=0; i<_numSelectors; i++) {
				if (_selection[i]->overlaps(overlap)) {
					_curSelector = i;
					break;
				}
			}
			delSelector();
			_selectingText = false;
		}
		else 
		{
			if (_numSelectors > 0) _selection[_curSelector]->freeze();
			addSelector(atom);
		}
//		wxLogDebug("   multi  ns= %i", _numSelectors);
	}
	else if (selectorWithSameLanguage < 0) {
//		wxLogDebug("   1  swsl= %i", selectorWithSameLanguage);
		if (overlap == NULL) addSelector(atom);
		///else ????
	}
	else {
//		wxLogDebug("   2  swsl= %i", selectorWithSameLanguage);
		_curSelector = selectorWithSameLanguage;
		if (_numSelectors > 0 && !_selection[_curSelector]->begin(atom)) clear();
	}
}

void hctSelectionManager::drag(txtitr_t atom, wxPoint pos) {
	if (_numSelectors > 0 && _selectingText) {  // do a text select drag
		_selection[_curSelector]->drag(atom, pos);
	}
	if (_selectingTextSimple) {
		_selectingTextSimple = false;
		_selection[_numSelectors] = new hctTextSelector(_text);
		_selection[_numSelectors]->beginSimple(atom, pos);
		_curSelector = _numSelectors;
		++_numSelectors;
		_selectingText = true;
	}
}

void hctSelectionManager::end() {
	_selectingText = false;
	_selectingTextSimple = false;

	// find and delete any "blank selections"
	for (int i=0; i<_numSelectors; i++) {
		if (_selection[i]->isBlank()) {
//			wxLogDebug("    FOUND EMPTY!");
			_curSelector = i;
			delSelector();
			_curSelector = _numSelectors-1;
		}
	}
}

void hctSelectionManager::addSelector(txtitr_t atom) {
	if (_numSelectors >= hctMAX_SELECTORS) return;  // too many selectors in use
	_selection[_numSelectors] = new hctTextSelector(_text);
	if (_selection[_numSelectors]->begin(atom)) {
		_curSelector = _numSelectors;
		++_numSelectors;
	}
	else {
		delete _selection[_numSelectors];
		_selection[_numSelectors] = NULL;
	}

}

void hctSelectionManager::addSelector(hctTextBlock_link *block) {
	if (_numSelectors >= hctMAX_SELECTORS) return;  // too many selectors in use
	_selection[_numSelectors] = new hctTextSelector(_text);
	_selection[_numSelectors]->setLinkBlock(block);   // deletes the block
	_numSelectors++;
}

void hctSelectionManager::delSelector() {
	if (_numSelectors == 0) return;
	delete _selection[_curSelector];
	_selection[_curSelector] = NULL;

	// shift array elements into "hole" left by our deletion
	for (int i=_curSelector+1; i<_numSelectors; i++) {
		_selection[i-1]=_selection[i];
	}
	_numSelectors--;
	_curSelector = _numSelectors-1;
}

void hctSelectionManager::linkSelected(hctHyperlink *link) {
	for (int i=0; i<_numSelectors; i++) {
		link->pushBlock(_selection[i]->getLinkBlock());
		delete _selection[i];
		_selection[i] = NULL;
	}
	_numSelectors = 0;
	_curSelector = 0;
}

bool hctSelectionManager::isValidForLinking() {
	for (int i=0; i<_numSelectors; i++) {
		if (!_selection[i]->isValidForLinking()) {
//			wxLogDebug("found invald link:  %i", i);
//			debug();
			return false;
		}
	}
	return true;
}
//  ^^^  (end selection stuff)  ^^^


void hctSelectionManager::copy() {
	if (_numSelectors == 0) return;
	wxLogDebug("hctSelectionManager::copy()");
	_clipBegin = _selection[_curSelector]->getSelectBlock()->getStart();
	_clipEnd = _selection[_curSelector]->getSelectBlock()->getEnd();
	if (wxTheClipboard->Open())
	{
		wxLogDebug("clipboard open");
		wxTheClipboard->SetData(new wxTextDataObject(_selection[_curSelector]->getText().c_str()));
		wxTheClipboard->Close();
	}
	
}

void hctSelectionManager::getRange(txtitr_t &begin, txtitr_t &end) {
	if (_numSelectors == 0) {
		begin = _text->end();
		end = _text->end();
		return;
	}
	begin = _selection[_curSelector]->getSelectBlock()->getStart();
	end = _selection[_curSelector]->getSelectBlock()->getEnd();
	if (end != _text->end()) ++end;  // return open range
}


void hctSelectionManager::debug() {
	for (int i=0; i<_numSelectors; i++) {
		wxLogDebug("%i : '%s'", i, _selection[i]->getText().c_str());
	}
}