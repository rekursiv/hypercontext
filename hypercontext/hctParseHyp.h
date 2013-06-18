#ifndef hctParseHyp_H
#define hctParseHyp_H

#include "hctParser.h"

#ifndef hctTempLink_H
#include "hctTempLink.h"
#endif

#include <stack>
#include <map>
using namespace std;

class hctTextWindow;


/// Loads and parses "HYP" files.
/**
	I am calling the current ACI Hypertexting file format "HYP" and hoping that
	someday we can move to a more robust XML-based format.
*/
class hctParseHyp : public hctParser
{

public:
	hctParseHyp();
	bool parse(wxString fileName, hctTextWindow &textWin);

private:
	void processToken(wxString token, bool isFinal, hctTextWindow &textWin);
	bool addLinks(hctTextWindow &textWin);
	void addNote(wxString comment, hctTextWindow &textWin);

//	void linkComment(wxString comment);
//	void addNote(int index, hctHyperlink *link, hctTextWindow &textWin);

	bool _skipNewlineToSpace;

	stack<hctTempLink> _linkStack;
	multimap<int, hctTempLink> _linkMap;
	map<int, wxString> _noteMap;

	int _token_level;
	int _prevLinkIndex;

	bool _oldFileFormat;

	int _curFontSize;

};


#endif
