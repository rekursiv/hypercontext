#include "stdwx.h"
#include "hctGlyphStackerACIP.h"
#include "hctTextWindow.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctGlyphStackerACIP::hctGlyphStackerACIP(hctTextManager* textManager) {
	_textManager = textManager;  // TODO: make this hctTextManager* instead
	_needsTsek = false;
	_curFontSize = 3;
}

void hctGlyphStackerACIP::addTextAtom(hctTextAtom &atom)
{
	_curFontSize = atom.getFontSize();
	char c = atom.getLetters().GetChar(0);
	if (c == '-') {  // check for '-' "stack now" operator
		_tcBuf.push_back(atom);
		stack();
	}
	else if (checkForPunctuation(c))
	{
		stack();  // stack what we have collected
		kern(atom);
		_textManager->addTextAtom(atom);  // add the punctuation to the end
		_needsTsek = false;


		// add "debug marker" to text, if requested
		if (!_debugStr.IsEmpty())
		{
			//		_debugWritePos = false;
			//		wxLogDebug("    i=%i", textWin.getTextSize());
			hctTextAtom atom;
			atom.setLanguage(hctLANG_ENGLISH);
			atom.setLetters(_debugStr);
			_textManager->addTextAtom(atom);
			atom.setLanguage(hctLANG_TIBETAN);
			unsigned short space[] = {160, 0, 0, 0, 0 };
			atom.setTibetanGlyphs(space);
			_textManager->addTextAtom(atom);
			_debugStr.Empty();
		}
	}
	else
	{
		_tcBuf.push_back(atom);
		_needsTsek = true;
	}
}


void hctGlyphStackerACIP::findComponents()
{
	// reset destination strings
	_prefix.Empty();
	_root.Empty();
	_suffix.Empty();
	_suffix2.Empty();
	_numbers.Empty();

	int tcBufSize = _tcBuf.size();
	if (tcBufSize == 0) return;

	// build a string to work from
	wxString syllable;
	for (int i=0; i<tcBufSize; i++) syllable << _tcBuf[i].getLetters();
//	wxLogDebug("syllable=%s", syllable.c_str());

	// check to see if we are working with numbers
	if (syllable.IsNumber()) {
		_numbers = syllable;
		return;
	}

	// these vars mark the start and end characters (within syllable) of the root letter
	int start = 0;
	int end = syllable.Len()-1;

	// we start by finding the first vowel in the syllable
	int vowelPos = findFirstVowel(syllable);
	if (vowelPos > 0) {
		if (vowelPos == 1) { // second letter is vowel
			// first letter must be the root
			_root = syllable.Left(2);
		}
		else {   // vowel appears later in the syllable
			// check for a prefix
			if (checkForPrefix(syllable))
			{
				_prefix = syllable.at(0);
				start = 1;
			}
			_root = syllable.SubString(start, vowelPos);
		}

		// check for suffixes
		if (checkForSuffix(syllable, end, vowelPos)) {
			int firstSuffixPos = end;
			if (checkForSuffix(syllable, end, vowelPos)) {
				_suffix = syllable.SubString(end+1, firstSuffixPos);
				_suffix2 = syllable.SubString(firstSuffixPos+1, tcBufSize-1);
			}
			else {
				_suffix = syllable.SubString(end+1, tcBufSize-1);
			}
		}
	}

//_debugStr << " <" << _prefix << ":" << _root << ":" << _suffix << ":" << _suffix2 << "|" << syllable << " ";

	// The code above will stack about 99% of normal Tibetan text.  However, it will fail on text
	// that does not follow the normal rules.  This could include typos, abbreviations, Sanskrit,
	// and maybe other things I don't know about.  We can tell if the text needs further processing
	// by checking to see if all the text is accounted for.
	int totalProcessed = _prefix.Len() + _root.Len() + _suffix.Len() + _suffix2.Len();
	if (totalProcessed != syllable.Len()) {
//		_debugStr << " <" << _prefix << ":" << _root << ":" << _suffix << ":" << _suffix2 << "|" << syllable << " ";
		// could be anything, let's try to kern it
		_prefix.Empty();
		_suffix.Empty();
		_suffix2.Empty();
		_root = syllable;
	}
}

int hctGlyphStackerACIP::getNumTextAtomsPending()
{
	findComponents();
	int num = 0;
	if (!_prefix.IsEmpty()) num++;
	if (!_root.IsEmpty()) num++;
	if (!_suffix.IsEmpty()) num++;
	if (!_suffix2.IsEmpty()) num++;
	if (!_numbers.IsEmpty()) num+=_numbers.size();
	return num;
}


void hctGlyphStackerACIP::stack() {

	_debugStr.Empty();
	findComponents();

	// fill text atoms and add them to the text window
	if (!_prefix.IsEmpty()) processTextAtom(_prefix);
	if (!_root.IsEmpty()) processTextAtom(_root);
	if (!_suffix.IsEmpty()) processTextAtom(_suffix);
	if (!_suffix2.IsEmpty()) processTextAtom(_suffix2);
	if (!_numbers.IsEmpty()) {
		wxString s;
		for (int i=0; i<_numbers.size(); ++i) {
			s = _numbers.at(i);
			processTextAtom(s);
		}
	}

	_tcBuf.clear();
}


int hctGlyphStackerACIP::findFirstVowel(wxString str)
{
	int pos;
	wxString vowelList("AEIOU");  //  "I'd like to buy a vowel..."
	for (pos=0; pos<str.Len(); pos++)
	{
		if (vowelList.Find(str.at(pos)) != -1) break;
	}
	if (pos == str.Len()) return -1;
	else return pos;
}

bool hctGlyphStackerACIP::checkForPunctuation(char c)
{
	wxString punctList(" ,;:&`#*");
	if (punctList.Find(c) != -1)
	{
		return true;
	}
	return false;
}

bool hctGlyphStackerACIP::checkForPrefix(wxString str)
{
	wxString prefixList("GDBM'");
	if (prefixList.Find(str.at(0)) != -1)
	{
		// if we got here, we probably have a prefix, but we need to check
		// a couple of special cases - it could be part of a stack...
		if (str.at(1) == '+') return false;  // check for '+' stack op
		if (str.at(1) == 'Y')  // check YATA subscript
			if (str.at(0) == 'G' || str.at(0) == 'B' || str.at(0) == 'M') return false;
		if (str.at(1) == 'R')  // check RATA subscript
			if (str.at(0) == 'G' || str.at(0) == 'M') return false;
		if (str.at(1) == 'V') return false;  // check for WAZUR subscript
		return true;
	}
	return false;
}

bool hctGlyphStackerACIP::checkForSuffix(wxString str, int &end, int firstVowelPos) {
	if (end > firstVowelPos) {  // for cases like "LE'U'I"
		wxString vowelList("AEIOU");
		if (vowelList.Find(str.at(end)) != -1) end--;
	}

	wxString suffixList("GDBM'RLSN");
	if (suffixList.Find(str.at(end)) != -1) {
		end--;
		if (str.at(end) == 'N') {  // special case for "NGA"
			end--;
		}
		return true;
	}
	return false;
}



void hctGlyphStackerACIP::processTextAtom(wxString str)
{
	// create a new text atom, fill in data, and copy links
	hctTextAtom atom;
	atom.setFontSize(_curFontSize);
	atom.setLanguage(hctLANG_TIBETAN);
	atom.setLetters(str);

	// kern it and add it to the text window
	kern(atom);
	_textManager->addTextAtom(atom);

}


void hctGlyphStackerACIP::kern(hctTextAtom &atom)
{
	wxString wylie = atom.getLetters();
	int len = wylie.Len();
	if (len == 0) return;

	// a ' ' character usually maps to a tzek, but in
	// some cases it can actually be a space.  We do
	// the conversion here if necessary.
	if (wylie.at(0) == ' ' && !_needsTsek)
	{
		unsigned short space[] = {160, 0, 0, 0, 0 };
		atom.setTibetanGlyphs(space);
		return;
	}

	if (wylie.Last() == '-') wylie.RemoveLast();  //  remove '-' operator
	if (wylie.Last() == 'A' && len > 1) wylie.RemoveLast();  // remove any trailing 'A'

//	wxLogDebug("wylie=%s", wylie);
	// lookup the wylie codes for this text atom in the kerning table
	kern_entry ke;
	ke.acip = wylie;
	ktItr_t foundPos = std::lower_bound(_kerningTable.begin(), _kerningTable.end(), ke);
	if (foundPos != _kerningTable.end()) atom.setTibetanGlyphs(foundPos->glyphs);
	else wxLogDebug("hctGlyphStackerACIP::kern() - Entry not found in kerning table:  %s", wylie.c_str());
}



bool hctGlyphStackerACIP::loadKerningTable(wxString fname) {
//	wxLogDebug("hctGlyphStackerACIP::loadKerningTable()");
	FILE *ktFile;
	ktFile = fopen(fname, "rb");
	if (!ktFile) return false;
	if (ferror(ktFile)) return false;
	char c;
	wxString s;
	wxString glyph;
	wxString acip;
	int glyphIndex = -1;
	bool readingACIP = false;
	kern_entry_t ke;
	while (!feof(ktFile)) {
		c = (char)getc(ktFile);	
		if (c == '\"') {
			if (readingACIP) {
				readingACIP = false;
				ke.acip = acip;
			}
			else {
				readingACIP = true;
				acip.clear();
			}
		}
		else if (readingACIP) {
			acip << c;
		}
		else if (c == ' ') {
			if (glyphIndex >= 0) {
				long g;
				glyph.ToLong(&g);
				ke.glyphs[glyphIndex] = (unsigned short)g;
			}
			glyph.clear();
			++glyphIndex;
		}
		else if (c == '\n') {
			glyphIndex = -1;
			_kerningTable.push_back(ke);
		}
		else {
			glyph << wxString::From8BitData(&c, 1);
		}
	}

	fclose(ktFile);
	return true;
/*
//	for (int i=0; i<_kerningTable.size(); ++i) {
	for (int i=0; i<50; ++i) {
		s.clear();
		s << _kerningTable[i].acip.c_str() << " - " << _kerningTable[i].glyphs[0];
		wxLogDebug(s);
	}
*/
}



void hctGlyphStackerACIP::writeKerningTable(void) {

	FILE *outFile;
	outFile = fopen("ACIP.out", "wb");  // TODO:  overwrite old??
	if (!outFile) return;
	if (ferror(outFile)) return;
	wxString s = "\n";
	std::sort(_kerningTable.begin(), _kerningTable.end());

	for (int i=0; i<_kerningTable.size(); ++i) {
//	for (int i=0; i<50; ++i) {
		s = "\"";
		s << _kerningTable[i].acip;
		s << "\" ";
		for (int j=0; j < hctTIB_GLYPHS_MAX; j++)
		{
			s << _kerningTable[i].glyphs[j];
			s << ' ';
		}
		s << "\n";
		fwrite(s.c_str(), 1, s.size(), outFile);
	}

	s = " ";
	fwrite(s.c_str(), 1, s.size(), outFile);
	fclose(outFile);
}


void hctGlyphStackerACIP::testKerningTable() {
	hctTextAtom spaceAtom;
	unsigned short space[] = {160, 0, 0, 0, 0 };
	spaceAtom.setLanguage(hctLANG_TIBETAN);
	spaceAtom.setLetters(" ");
	spaceAtom.setTibetanGlyphs(space);

	for (int i=0; i<_kerningTable.size(); ++i) {
		hctTextAtom atom;
		atom.setLanguage(hctLANG_TIBETAN);
		atom.setTibetanGlyphs(_kerningTable[i].glyphs);
		atom.setLetters(_kerningTable[i].acip);
		_textManager->addTextAtom(atom);
		_textManager->addTextAtom(spaceAtom);
	}

}

void hctGlyphStackerACIP::testFont() {
	hctTextAtom sep;
	unsigned short tsek[] = {45, 0, 0, 0, 0 };
	sep.setLanguage(hctLANG_TIBETAN);
	sep.setLetters(" ");
	sep.setTibetanGlyphs(tsek);
//	sep.setCtrlChar(hctCTRL_BREAK);

	unsigned short glyphs[] = {0, 0, 0, 0, 0 };
	wxString s;
	for (int i=33; i<252; ++i) {
		glyphs[0]=i;
		hctTextAtom atom;
		atom.setLanguage(hctLANG_TIBETAN);
		atom.setTibetanGlyphs(glyphs);
		s.Empty();
		s << i;
		atom.setLetters(s);
		_textManager->addTextAtom(atom);
//		sep.setLetters(s);
		_textManager->addTextAtom(sep);
	}

}