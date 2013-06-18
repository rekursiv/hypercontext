#ifndef hctParser_H
#define hctParser_H

class hctTextWindow;

#ifndef hctTextAtom_H
#include "hctTextAtom.h"
#endif

/// ABC for parsers
class hctParser
{

public:
	virtual ~hctParser(){}
	virtual bool parse(wxString fileName, hctTextWindow &textWin) = 0;
	virtual wxString getLastError() {
		return _errorMsg;
	}
	virtual bool getLastErrorWasFatal() {
		return _errorFatal;
	}

protected:
//	hctGlyphStackerACIP _glyphStacker;
	hctLanguage_t _curLanguage;

	wxString _errorMsg;
	bool _errorFatal;

};


#endif
