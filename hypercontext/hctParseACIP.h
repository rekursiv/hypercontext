#ifndef hctParseACIP_H
#define hctParseACIP_H

#include "hctParser.h"

/// Loads and parses ACIP files

class hctParseACIP : public hctParser
{

public:
	hctParseACIP();
	bool parse(wxString fileName, hctTextWindow &textWin);


};


#endif
