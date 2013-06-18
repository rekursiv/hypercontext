#include "stdwx.h"
#include "hctParseACIP.h"
#include "hctTextWindow.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


hctParseACIP::hctParseACIP()
{
}

bool hctParseACIP::parse(wxString fileName, hctTextWindow &textWin)
{
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

	_curLanguage = hctLANG_TIBETAN;
//	_token_level = 0;

	char c;
	char prev_char = '\0';
	bool is_token = false;
	bool is_index = false;
	wxString token_buf;
	wxString index_buf;

	bool skipNewlineToSpace = false;

	bool parsing = true;
	while (parsing)
	{
		file.Read(&c, 1);
		if (file.Eof()) {
			parsing = false;
			continue;
		}
		else if (file.Error() || c < 0) {
			_errorMsg.Empty();
			_errorMsg << "Error reading file \"" << fileName << "\n";
			success = false;
			parsing = false;
			continue;
		}

		if (c == '[')  // beginning of token
		{
			is_token = true;
			continue;
		}
		else if (c == '@')  // beginning of index
		{
			is_index = true;
			continue;
		}
		else if (is_token)
		{
//			if (c == ' ')
			if (c == ']')  // ???
			{
				token_buf.Clear();
				is_token = false;
			}
			else
			{
				token_buf << c;
			}
			continue;
		}
		else if (is_index)
		{
			if (c == ' ')
			{
				index_buf.Clear();
				is_index = false;
			}
			else
			{
				index_buf << c;
//				wxLogDebug("%c\n", c);
				continue;
			}
		}
		else if (c == '*')
		{
//			if (prev_char == ' ') textWin.getText()->pop_back();  // ????
			textWin.getGlyphStacker()->stack();
			hctTextAtom atom;
			atom.setCtrlChar(hctCTRL_BREAK);
			atom.setLanguage(_curLanguage);
			textWin.getTextManager()->addTextAtom(atom);
			textWin.getTextManager()->addTextAtom(atom);
			skipNewlineToSpace = true;
		}


		// replace the first instance of a newline with a space
		if (skipNewlineToSpace) skipNewlineToSpace = false;
		else if (c == '\n' && prev_char != '\n') c = ' ';
		prev_char = c;

		if (!iscntrl(c))
		{
			hctTextAtom atom;
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
	textWin.setNeedsUpdate();

	return success;
}


