#include "stdwx.h"
#include "hctParseHyp.h"
#include "hctTextWindow.h"

#include "hctLinkManager.h"

//#include "hctHyperlink.h"
//#include "hctTextBlock_link.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class hctHyperlink;

hctParseHyp::hctParseHyp()
{
	_token_level = 0;
	_prevLinkIndex = 0;
	_oldFileFormat = false;
}

bool hctParseHyp::parse(wxString fileName, hctTextWindow &textWin)
{
//	hctHyperTextWindow *hyperTextWin = static_cast<hctHyperTextWindow*>(&textWin);

	bool success = true;
	_errorMsg = "Unknown error parsing file.";
	_errorFatal = false;

	wxFFile file(fileName, "rt");
	if (!file.IsOpened()) {
		_errorMsg.Empty();
		_errorMsg << "Could not open file \"" << fileName << "\"\n";
		_errorFatal = true;
		return false;
	}

//	_curLanguage = hctLANG_TIBETAN;
//	_curLanguage = hctLANG_ENGLISH;
	_curLanguage = hctLANG_NONE;
	_curFontSize = 3;
	_token_level = 0;
	_skipNewlineToSpace = false;
	_prevLinkIndex = 0;
	int prevRootLinkIndex = 0;

	bool convertNewlines = true;
	wxString extension = fileName.AfterLast('.');
	if (extension.CmpNoCase("OLD") == 0) {
		convertNewlines = false;
		_oldFileFormat = true;
		textWin.setEditMode(hctEM_LINK);
	}
	else if (extension.CmpNoCase("HYP") == 0) textWin.setEditMode(hctEM_LINK);
	else textWin.setEditMode(hctEM_TEXT);

	char c;
	char prev_char = '\0';
	bool is_token = false;
	bool is_comment = false;
	wxString token_buf;
	wxString comment_buf;

	bool parsing = true;
	while (parsing)
	{
		file.Read(&c, 1);
//		wxLogDebug("%c", c);
		if (file.Eof()) {
			parsing = false;
			continue;
		}
		else if (file.Error() || c < 0) {
			_errorMsg.Empty();
			_errorMsg << "Error reading file \"" << fileName << "\"   Last link processed:  " << _prevLinkIndex << "\n";
			success = false;
			parsing = false;
			continue;
		}
		else if (is_comment)
		{
			comment_buf << c;
			if (c == '<')
			{
				_token_level++;
			}
			else if (c == '>')
			{
				_token_level--;
				if (_token_level == 0)
				{
					is_comment = false;
					is_token = false;
					_skipNewlineToSpace = true;
					addNote(comment_buf, textWin);
					comment_buf.Empty();
				}
			}
			continue;
		}
		else if (c == '$')  // escape char
		{
			file.Read(&c, 1);
		}
		else if (c == '<')  // beginning of token
		{
//			wxLogDebug("%c", c);
			_token_level++;
			is_token = true;
			continue;
		}
		else if (is_token)
		{
			if (c == '!') is_comment = true;
			else if (c == '>')
			{
				processToken(token_buf, true, textWin);
				token_buf.Clear();
				is_token = false;
				_token_level--;
			}
			else if (isspace(c))
			{
				processToken(token_buf, false, textWin);
				token_buf.Clear();
				is_token = false;
			}
			else
			{
				token_buf << c;
			}
			continue;
		}
		else if (c == '>')   // end of token
		{
			_token_level--;
			if (!_linkStack.empty()) {
				_linkStack.top().end = textWin.getText()->size() + textWin.getGlyphStacker()->getNumTextAtomsPending();
				_linkMap.insert(make_pair(_linkStack.top().index, _linkStack.top()));
				_linkStack.pop();
			}
			if (_token_level == 0) {
				is_comment = false;
				is_token = false;
				prevRootLinkIndex = _prevLinkIndex;
			}
			else if (_token_level < 0) {   // error:  mismatched '<'
				_errorMsg.Empty();
				_errorMsg << "Error reading file \"" << fileName << "\"   Last link processed:  " << _prevLinkIndex << "\n";
				_errorMsg << "  Missing '<'\n";
				success = false;
				parsing = false;
			}
			continue;
		}

		if (convertNewlines) {
			if (c == '\n') {
				token_buf.Clear();
				token_buf << "br";
				processToken(token_buf, true, textWin);
				token_buf.Clear();
//				wxLogDebug("found nl");
			}
		}
		else {

			// replace the first instance of a newline with a space
			if (_skipNewlineToSpace) _skipNewlineToSpace = false;
			else if (c == '\n')
			{
				if (prev_char == ' ')
				{
					textWin.getText()->pop_back();
				}
				else if (prev_char != '\n') c = ' ';
			}
		}
		prev_char = c;

		if (!iscntrl(c))
		{
			hctTextAtom atom;
			atom.setFontSize(_curFontSize);
			atom.setLanguage(_curLanguage);
			atom.setLetters(c);
			if (_curLanguage == hctLANG_TIBETAN)
			{
				textWin.getGlyphStacker()->addTextAtom(atom);
			}
			else
			{
				textWin.getTextManager()->addTextAtom(atom);
			}
		}
//wxLogDebug("%c\n", c);
	}

	textWin.getGlyphStacker()->stack();
	bool linksOK = addLinks(textWin);
	textWin.setNeedsUpdate();;

	// handle errors
	bool err = false;
	wxString errType;
	if (!linksOK) {    // error:  found bad link '>'
		errType << "Bad Link";
		err = true;
	}
	if (_token_level > 0) {   // error:  mismatched '>'
		errType << "Missing '>'";
		err = true;
	}
	if (err) {
		_errorMsg.Empty();
		_errorMsg << "Error reading file \"" << fileName << "\"   Last root-level link processed:  ";
		_errorMsg << prevRootLinkIndex << "\n";
		_errorMsg << "   Reason:     " << errType << "\n";
		return false;
	}
	return success;
}



// isFinal is true when the token was ended with a '>'
// example:  <e>   
// isFinal is false when the token was ended with a space
// example:  <e This is some text>
void hctParseHyp::processToken(wxString token, bool isFinal, hctTextWindow &textWin)
{
	if (token.IsEmpty()) return;
//	wxLogDebug("** parse:  token = %s", token.c_str());

	// handle numbered links
	hctTempLink link;
	int tokLen = token.Len();
	int i;
	char code;
	for (i=0; i<tokLen; i++) {  // extract "codes" from link tag
		code = token.at(i);
		if (code == '+') link.isCont = true;
		else if (code == '=') link.isInfreq = true;
		else if (code == '&') link.isSyn = true;
		else break;
	}
	wxString num = token.Mid(i);
	if (num.IsNumber())
	{
		num.ToLong((long *)&_prevLinkIndex);  // grab number from link tag
		link.index = _prevLinkIndex;
		link.offset = textWin.getText()->size() + 1;   //????
		link.start = link.offset + textWin.getGlyphStacker()->getNumTextAtomsPending();
		txtitr_t end = textWin.getText()->end();
		if (end != textWin.getText()->begin()) --end;
		link.search_from = end;
		_linkStack.push(link);
		return;
	}

	// handle other tags
	if (token == "pg")
	{
		textWin.getGlyphStacker()->stack();
		hctTextAtom atom;
		atom.setCtrlChar(hctCTRL_PARAGRAPH);
		atom.setFontSize(_curFontSize);
		atom.setLanguage(_curLanguage);
		textWin.getTextManager()->addTextAtom(atom);
		_skipNewlineToSpace = true;
	}
	else if (token == "br")
	{
		textWin.getGlyphStacker()->stack();
		hctTextAtom atom;
		atom.setCtrlChar(hctCTRL_BREAK);
		atom.setFontSize(_curFontSize);
		atom.setLanguage(_curLanguage);
		textWin.getTextManager()->addTextAtom(atom);
		_skipNewlineToSpace = true;
	}
	else if (token == "e")
	{
		_curLanguage = hctLANG_ENGLISH;
		textWin.getGlyphStacker()->stack();
	}
	else if (token == "t")
	{
		_curLanguage = hctLANG_TIBETAN;
		textWin.getGlyphStacker()->clear();
	}
	else if (token == "s")
	{
		_curLanguage = hctLANG_SANSKRIT;
		textWin.getGlyphStacker()->stack();
	}
	else if (token == "p")
	{
		_curLanguage = hctLANG_PHONETIC_TIBETAN;
		textWin.getGlyphStacker()->stack();
	}
	else if (token == "fs")
	{
		_curFontSize = 3;
	}
	else if (token == "fs+1")
	{
		_curFontSize = 4;
	}
	else if (token == "fs+2")
	{
		_curFontSize = 5;
	}
	else if (token == "fs-1")
	{
		_curFontSize = 2;
	}
	else if (token == "fs-2")
	{
		_curFontSize = 1;
	}

	// TODO:  Take into account the following...
/* Language directives can be either <language ...>,
   which sets the language temporarily, or
   <language>, which sets it until the next language
   directive.   These cases are handled somewhat
   differently. */

}

void hctParseHyp::addNote(wxString comment, hctTextWindow &textWin) {
	wxString linkNum = comment.BeforeFirst(' ');  // parse out link number, if any
	int index = 0;

	wxString note;
	if (!linkNum.IsEmpty() && linkNum.IsNumber()) {  // we found a number
		linkNum.ToLong((long *)&index);
		note = comment.Mid(linkNum.Len()+1);
		note.RemoveLast();
	}
	else {  // no number - "floating" comment
		note = comment.RemoveLast();
	}

	if (index > 0) {
		hctTempLink link;
		link.index = index;
		_prevLinkIndex = index;
		link.offset = textWin.getText()->size() + 1;
		link.start = link.offset;
		link.end = link.start;
		txtitr_t end = textWin.getText()->end();
		if (end != textWin.getText()->begin()) --end;
		link.search_from = end;
		_linkMap.insert(make_pair(index, link));
	}

	//////////////////////////////////////////////////////////////////////////
	hctTextAtom atom;
	atom.setIsNote(true);
	atom.setLetters(note);
	textWin.getTextManager()->addTextAtom(atom);
}

bool hctParseHyp::addLinks(hctTextWindow &textWin)
{
	bool success = true;
	int index;
	int prevIndex = -1;
	hctHyperlink *prevLink = NULL;
	multimap<int, hctTempLink>::iterator linkPos;
	for (linkPos = _linkMap.begin(); linkPos != _linkMap.end(); ++linkPos) {
		if (linkPos->second.start > linkPos->second.end) {
			wxLogDebug("*** BAD LINK:  i=%i  s=%i  e=%i", linkPos->first, linkPos->second.start, linkPos->second.end);
			success = false;
			continue;
		}
//		wxLogDebug("i=%i  s=%i  e=%i", linkPos->first, linkPos->second.start, linkPos->second.end);
		index = linkPos->second.index;

		int zeroLink = 0;
		if (_oldFileFormat) zeroLink = -2;
//		if (index != prevIndex) {  // to load old file:  uncomment me, comment out one below
		if (index == zeroLink || index != prevIndex) {
			prevLink = textWin.getLinkManager()->addLink(NULL, linkPos->second);
		}
		else textWin.getLinkManager()->addLink(prevLink, linkPos->second);
		prevIndex = index;
	}
	return success;
}
