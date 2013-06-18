#include "stdwx.h"
#include "hctSearchManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctSearchManager::hctSearchManager(text_t *text) {
	_text = text;
	_foundBegin = _text->end();
	_foundEnd = _text->end();
	_curLang = hctLANG_ENGLISH;
	_isCaseSensitive = true;
	setRangeToAll();
}

bool hctSearchManager::isSame(wxString a, wxString b) {
	if (_isCaseSensitive) return a.Cmp(b)==0;
	else return a.CmpNoCase(b)==0;
}

bool hctSearchManager::find() {
	txtitr_t begin;
	int ttfPos = 0;
	wxString a, b;
	begin = _searchBegin;
	for (txtitr_t pos = _searchBegin; pos != _searchEnd; ++pos) {
		if (pos->getLanguage() == _curLang) 
		{
			a = pos->getLetters();
			b = _txtToFind.Mid(ttfPos, a.size());
//			wxLogDebug("  SM matching:  %s : %s", a, b);
			if (isSame(a, b)) {
				ttfPos += a.size();
				if (ttfPos >= _txtToFind.size()) {  // found 
					_foundBegin = begin;
					++_foundBegin;
					_foundEnd = pos;
					++_foundEnd;
					return true;
				}
			}
			else {
				ttfPos = 0;
				begin = pos;
			}
		}
		else {
			ttfPos = 0;
			begin = pos;
		}
	}
	return false;
}