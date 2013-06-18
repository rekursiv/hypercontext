#ifndef hctFontManager_H
#define hctFontManager_H

#ifndef hctCachedFont_H
#include "hctCachedFont.h"
#endif

#include <map>

class hctFontManager {
public:
	~hctFontManager();
	static hctFontManager* getInstance();
	static void release() {
		if (s_instance) delete s_instance;
	}

	hctCachedFont* getCachedFont(hctFont_t type, int size);
	bool areTibetanFontsInstalled();


private:
	static hctFontManager *s_instance;

	wxString makeString(hctFont_t type, int size);

	typedef std::map<wxString, hctCachedFont*> fontMap_t;
	typedef fontMap_t::iterator fontMapItr_t;

	fontMap_t _fontMap;

	///
	hctFontManager();
	hctFontManager(const hctFontManager&);
	hctFontManager& operator=(const hctFontManager& rhs);
};

#endif