#include "stdwx.h"
#include "hctSaveHyp.h"
#include "hctTextWindow.h"
#include "hctHyperlink.h"
#include "hctTextBlock_link.h"
#include "hctApp.h"
#include "hctRootWindow.h"

#include <stack>
#include <queue>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void hctSaveHyp::save(wxString fileName, hctTextWindow &textWin) {
	textWin.getLinkManager()->resetLinks();
	wxFFile file;
	if (!file.Open(fileName, "wt")) return;
	int i = 0;
	int nextLinkIndex = 0;
	wxString buf;
	hctLanguage_t curLanguage = hctLANG_NONE, prevLanguage = hctLANG_NONE;
	hctTextBlock_link *prevBlock = NULL;
	bool isComment = false;
	bool commentIsLink = false;
	int curFontSize = 3, prevFontSize = 3;
//	int linkDepth = 0;

	for (txtitr_t atom = textWin.getText()->begin(); atom != textWin.getText()->end(); ++atom) {
		buf.Empty();
		isComment = atom->isNote();
		curLanguage = atom->getLanguage();
		if (curLanguage != prevLanguage && !isComment) {
			if (curLanguage == hctLANG_TIBETAN) buf << "<t>";
			else if (curLanguage == hctLANG_ENGLISH) buf << "<e>";
			else if (curLanguage == hctLANG_SANSKRIT) buf << "<s>";
			else if (curLanguage == hctLANG_PHONETIC_TIBETAN) buf << "<p>";
			prevLanguage = curLanguage;
		}

		curFontSize = atom->getFontSize();
		if (curFontSize != prevFontSize) {
			switch (curFontSize) {
				case 1: buf << "<fs-2>"; break;
				case 2: buf << "<fs-1>"; break;
				case 3: buf << "<fs>"; break;
				case 4: buf << "<fs+1>"; break;
				case 5: buf << "<fs+2>"; break;
			}
			prevFontSize = curFontSize;
		}

		//		wxLogDebug("2 atom %i", atom->getIndex());
		hctTextBlock_link *block = atom->getLinkBlock();
		if (block != prevBlock) {   // we have entered a new link

			// check for the end of a link
			hctTextBlock_link *tmpBlock = prevBlock;
			while (tmpBlock) {  // if the links are nested, we need to check them all
				int end = tmpBlock->getEnd()->getIndex()+1;
				if (i == end) buf << ">";    // write end-of-link marker
				commentIsLink = false;
				tmpBlock = tmpBlock->getNext();
			}

			// check for the beginning of a link
			// since we need to output the links from largest-to-smallest, and since
			// they are sorted smallest-to-largest, we push them onto a stack here
			// so we can pop them off in reverse order a little later
			stack<hctHyperlink*> linkStack;
			stack<wxString> modStack;       // for "word modifier codes"  (+&=)
			tmpBlock = block;
			while (tmpBlock) {  // if the links are nested, we need to check them all
				int start = tmpBlock->getStart()->getIndex();
				if (i == start) {
					hctHyperlink *link = tmpBlock->getLink();
					if (link) {
						linkStack.push(link);
						wxString wordMods;   // for "word modifier codes"  (+&=)
						if (tmpBlock->_wordCont) wordMods << '+';
						if (tmpBlock->_wordSyn) wordMods << '&';
						if (tmpBlock->_wordInfreq) wordMods << '=';
						modStack.push(wordMods);   // temp   /////////////////////////
					}

				}
				tmpBlock = tmpBlock->getNext();
			}

			// yes, the moment has come - we now pop the links off the stack in reverse order
			while (!linkStack.empty()) {
				int li = linkStack.top()->getIndex();
				if (li == -1) {  // if this link has not been indexed,
					if (linkStack.top()->getNumBlocks() > 1) li = ++nextLinkIndex;  // then give it a new one,
					else li = 0;  // or assign it 0 if it links to nowhere.
					linkStack.top()->setIndex(li);
				}
				buf << "<";
				if (isComment) {
					buf << "!";
					commentIsLink = true;
				}
				buf << modStack.top() << li << " ";  // write beginning-of-link marker
				linkStack.pop();
				modStack.pop();   //  temp
			}
			prevBlock = block;
		}

		if (atom->isNote()) {
			if (!commentIsLink) buf << "<!";
			// make sure '<' and '>' are balanced within comment
			wxString comment = atom->getLetters();
			int lt = 0, gt = 0;
			for (int i=0; i<comment.size(); ++i) {
				if (comment.at(i) == '>') ++gt;
				else if (comment.at(i) == '<') ++lt;
			}
			// fix it if they're not
			for (int i=0; i<lt-gt; ++i) comment << '>';
			for (int i=0; i<gt-lt; ++i) buf << '<';
			buf << comment;
			if (!commentIsLink) buf << ">";				
		}
		else if (atom->isCtrl()) {
			if (atom->getCtrlChar() == hctCTRL_PARAGRAPH) {
				buf << "\n\n";
			}
			else if (atom->getCtrlChar() == hctCTRL_BREAK) {
				buf << "\n" ;
			}
		}
		else {
			// check for "token" characters, escape them if necessary
			wxString letters = atom->getLetters();
			char c = letters.at(0);
			if (c=='<'||c=='>'||c=='$') letters.Prepend("$");
			buf << letters;
		}

		file.Write(buf);
		if (file.Error()) return;
		i++;
	}
	wxGetApp().getRootWindow()->setHasBeenEdited(false);
}
