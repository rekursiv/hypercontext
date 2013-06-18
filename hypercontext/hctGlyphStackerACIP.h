#ifndef hctGlyphStackerACIP_H
#define hctGlyphStackerACIP_H

#ifndef hctTextAtom_H
#include "hctTextAtom.h"
#endif

#include <vector>
using namespace std;

/// Kerning table structure definition
typedef struct kern_entry {
	wxString acip;
	unsigned short glyphs[hctTIB_GLYPHS_MAX];
	bool operator <(const kern_entry& rhs) const {
		return acip < rhs.acip;
	}
} kern_entry_t;

class hctTextManager;


/// Splits Wylie codes into stacks of Tibetan glyphs
/**
	The parser feeds a stream of ACIP Wylie codes into this class while reading the 
	input file.  The class buffers the input until a control or punctuation code is
	reached (most commonly a "tsek") and then arranges the Wylie codes into "stacks."
	Each stack is then looked up in a kerning table to determine the set of Tibetan
	glyphs that should be used.  The codes for both the Wylie and the Tibetan
	glyphs are fed into a new hctTextAtom, and then the text atom is fed into
	the main text window.
*/
class hctGlyphStackerACIP
{
public:
	hctGlyphStackerACIP(hctTextManager* textManager);
	void addTextAtom(hctTextAtom &atom);
	void stack();
	int getNumTextAtomsPending();
	void clear() {
		_tcBuf.clear();
		_needsTsek = false;
	}
	bool loadKerningTable(wxString fname);

	///////  utils
	void testKerningTable();
	void writeKerningTable();
	void testFont();


private:
	void findComponents();
	int findFirstVowel(wxString str);
	bool checkForPunctuation(char c);
	bool checkForPrefix(wxString str);
	bool checkForSuffix(wxString str, int &end, int firstVowelPos);

	void processTextAtom(wxString str);
	void kern(hctTextAtom &atom);

	typedef vector<kern_entry> kerningTable_t;
	typedef kerningTable_t::iterator ktItr_t;
	kerningTable_t _kerningTable;

	vector<hctTextAtom> _tcBuf;

	bool _needsTsek;
	wxString _prefix, _root, _suffix, _suffix2, _numbers;

	wxString _debugStr;

	hctTextManager* _textManager;   // TODO: make this hctTextManager* instead
	int _curFontSize;
};

#endif
