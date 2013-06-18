#ifndef hctSearchManager_H
#define hctSearchManager_H

#ifndef hctTextManager_H
#include "hctTextManager.h"
#endif

class hctSearchManager {
public:
	hctSearchManager(text_t *text);
	~hctSearchManager(){};

	void setRangeToAll() {
		_searchBegin = _text->begin();
		_searchEnd = _text->end();
	}
	void setRangeBegin(txtitr_t begin) {
		_searchBegin = begin;
	}
	void setRange(txtitr_t begin, txtitr_t end) {
		_searchBegin = begin;
		_searchEnd = end;
	}
	void setText(wxString txt) {
		_txtToFind = txt;
	}
	void setLanguage(hctLanguage_t lang) {
		_curLang = lang;
	}
	bool find();
	txtitr_t getFoundBegin() {
		return _foundBegin;
	}
	txtitr_t getFoundEnd() {
		return _foundEnd;
	}
	void setCaseSensitive(bool set) {
		_isCaseSensitive = set;
	}

private:
	bool isSame(wxString a, wxString b);

	text_t *_text;
	wxString _txtToFind;
	hctLanguage_t _curLang;
	txtitr_t _searchBegin, _searchEnd;
	txtitr_t _foundBegin, _foundEnd;
	bool _isCaseSensitive;
};

#endif