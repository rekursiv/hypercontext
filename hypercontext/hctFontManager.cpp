#include "stdwx.h"
#include "hctFontManager.h"

#include <wx/fontenum.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctFontManager* hctFontManager::s_instance = NULL;

hctFontManager::hctFontManager() {
}

hctFontManager::~hctFontManager() {
	for (fontMapItr_t i = _fontMap.begin(); i!=_fontMap.end(); ++i) {
		delete i->second;
	}
}

bool hctFontManager::areTibetanFontsInstalled() {
	wxFontEnumerator e;
	wxArrayString as;

	e.EnumerateFacenames();
	as = e.GetFacenames();
	for (int i=0; i<as.GetCount(); ++i) {
//		wxLogDebug("    %s", as->Item(i).c_str());
		if (as.Item(i) == "HypercontextTibetan") return true;
	}
	return false;
}


hctFontManager* hctFontManager::getInstance() {
	if (!s_instance) {
		s_instance = new hctFontManager();
	}
	return s_instance;
}

hctCachedFont* hctFontManager::getCachedFont(hctFont_t type, int size) {
	wxString fontId = makeString(type, size);
	fontMapItr_t pos = _fontMap.find(fontId);
	if (pos == _fontMap.end()) {
		hctCachedFont *font = new hctCachedFont(type, size);
		_fontMap.insert(std::make_pair(fontId, font));
//		wxLogDebug("hctFontManager::getCachedFont() - made new font:  %s", fontId);
		return font;
	}
	else return pos->second;
}

wxString hctFontManager::makeString(hctFont_t type, int size) {
	wxString id;

	switch (type) {
		case hctFONT_DEFAULT:
				id = "d_";
			break;
		case hctFONT_WYLIE:
				id = "w_";
			break;
		case hctFONT_TIBETAN:
				id = "t_";
			break;
	}
	id << size;
	
	return id;
}