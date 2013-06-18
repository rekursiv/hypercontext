#include "stdwx.h"
#include "hctLinkManager.h"

#include "hctSelectionManager.h"
#include "hctApp.h"
#include "hctRootFrame.h"
#include "hctHyperlink.h"
#include "hctTextBlock_link.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


hctLinkManager::hctLinkManager(text_t *text) {
	_text = text;
	_curLinkBlockFocused = NULL;
	_linkFocusLocked = false;     //  (currently not in use)
	_firstLinkFound = _searchMap.end();
	_lastLinkFound = _searchMap.end();
	_curFoundLink = _searchMap.end();
}


hctLinkManager::~hctLinkManager() {
	freeLinkMemory();
}


void hctLinkManager::deleteAllLinks() {
	wxGetApp().getRootWindow()->endNoteEdit();
	wxGetApp().getRootWindow()->clearDefText();
	_curLinkBlockFocused = NULL;
	freeLinkMemory();
}

void hctLinkManager::freeLinkMemory() {
	set<hctHyperlink*>::iterator pos;
	for (pos = _hyperlinks.begin(); pos != _hyperlinks.end(); pos++) {
		delete *pos;
	}
	_hyperlinks.clear();

	_searchMap.clear();
	_firstLinkFound = _searchMap.end();
	_lastLinkFound = _searchMap.end();
	_curFoundLink = _searchMap.end();
}


//			=== file I/O ===

void hctLinkManager::resetLinks()
{
	int count = 0;
	for (txtitr_t i=_text->begin(); i!=_text->end(); ++i)
	{
		i->setIndex(count++);
		hctTextBlock_link *block = i->getLinkBlock();
		if (block)
		{
			hctHyperlink *link = block->getLink();
			if (link) link->setIndex(-1);
		}
	}
}

hctHyperlink *hctLinkManager::addLink(hctHyperlink *link, hctTempLink &tmpLink) {
//	wxLogDebug("addLink:  so=%i   eo=%i", tmpLink.start - tmpLink.offset, tmpLink.end - tmpLink.offset);

	// get range from info in temp link
	int count = -1;
	txtitr_t start;
	txtitr_t end;
	for (txtitr_t i=tmpLink.search_from; ; ++i) {
		if (tmpLink.start - tmpLink.offset == count) {
			start = i;
		}
		if (tmpLink.end - tmpLink.offset == count) {
			end = i;
			break;
		}
		++count;
	}

	// create a new block
	hctTextBlock_link *block = new hctTextBlock_link(_text);
	block->setRange(start, end);
	block->_wordCont = tmpLink.isCont;
	block->_wordInfreq = tmpLink.isInfreq;
	block->_wordSyn = tmpLink.isSyn;

	// create a new link, if needed
	if (!link || _hyperlinks.find(link)==_hyperlinks.end()) {
		link = new hctHyperlink;
		_hyperlinks.insert(link);
	}

	// add block to link
	link->pushBlock(block);

	// add text to search map
	addToSearchMap(block);

	return link;
}
//  ^^^  (end file I/O)  ^^^


void hctLinkManager::addToSearchMap(hctTextBlock_link* block) {
	if (!block) return;
	if (block->getLanguage() == hctLANG_TIBETAN) {
		_searchMap.insert(make_pair(block->getText(), block));
	}
// TODO:  english too??
}


// TODO: optimize - search for upper/lower bound, check for block between
void hctLinkManager::removeFromSearchMap(hctTextBlock_link* block) {  ///////////  TESTME
	if (!block) return;

	// clear previous selection
	for (smapitr_t i = _firstLinkFound; i != _lastLinkFound; ++i) {
		i->second->getLink()->setSelected(false);
	}
	_firstLinkFound = _searchMap.end();
	_lastLinkFound = _searchMap.end();

	if (block->getLanguage() == hctLANG_TIBETAN) {
		for (smapitr_t i = _searchMap.begin(); i != _searchMap.end(); ++i) {
			if (block == i->second) {
//wxLogDebug("    hctLinkManager::removeFromSearchMap()  _searchMap.erase()");
				_searchMap.erase(i);
				break;				//  always only one???
			}
		}
	}
	// TODO:  english too??
}


bool hctLinkManager::refocus(txtitr_t atom, bool lookupTibetan) {
	bool focusChanged = false;
	if (_linkFocusLocked) return focusChanged;
	hctTextBlock_link *block = NULL;
	if (atom != _text->end()) { //  look for links at this pos
		block = atom->getLinkBlock();
	}

	if (_curLinkBlockFocused) { 	// check to see if current link needs de-focusing
		bool deFocus = false;
		if (block != _curLinkBlockFocused) deFocus = true;
		else if (_curLinkBlockFocused != atom->getLinkBlock()) deFocus = true;         //////   TESTME
		if (deFocus) {
			focusChanged = true;
			hctHyperlink *link = _curLinkBlockFocused->getLink();
			if (link) link->setFocus(false);
			else wxLogDebug("!! hctLinkManager::refocus()  tried to unfocus NULL link!");
			_curLinkBlockFocused = NULL;
			if (lookupTibetan) wxGetApp().getRootWindow()->clearDefText();
			clearMarked();  //???
		}
	}

	if (block) {  // set the new current link and focus it
		if (_curLinkBlockFocused != block) {
			_curLinkBlockFocused = block;
			hctHyperlink *link = block->getLink();
			if (link) {
				focusChanged = true;
				if (_curLinkBlockFocused->getLanguage() == hctLANG_TIBETAN && lookupTibetan) {
					_curLinkBlockFocused->define();
				}
				link->setFocus(true);
			}
			else wxLogDebug("!! hctLinkManager::refocus()  tried to focus NULL link!");
		}
	}
	return focusChanged;
}


int hctLinkManager::searchLinks(wxString text, hctLanguage_t language) {
	// clear previous selection
	for (smapitr_t i = _firstLinkFound; i != _lastLinkFound; ++i) {
		i->second->getLink()->setSelected(false);
	}
	_firstLinkFound = _searchMap.end();
	_lastLinkFound = _searchMap.end();	

	int numFound = 0;
	if (language == hctLANG_TIBETAN) {
		_firstLinkFound = _searchMap.lower_bound(text);
		_lastLinkFound = _searchMap.upper_bound(text);
		numFound = distance(_firstLinkFound, _lastLinkFound);
	}

	{
//		int lb = distance(_searchMap.begin(), _firstLinkFound);
//		int ub = distance(_searchMap.begin(), _lastLinkFound);
//		int last = distance(_searchMap.begin(), _searchMap.end());
//		wxString s;
//		s << "hctLinkManager::searchLinks()   "  << lb	<< " - " << ub << " : " << last;
//		wxLogDebug(s);
	}

	// select new selection
	for (smapitr_t i = _firstLinkFound; i != _lastLinkFound; ++i) {
		i->second->getLink()->setSelected(true);
	}

	if (numFound == 0) _curFoundLink = _searchMap.end();
	else _curFoundLink = _firstLinkFound;

	return numFound;
}

hctTextBlock_link* hctLinkManager::getNextFound() {
	if (_curFoundLink == _searchMap.end()) return NULL;

	++_curFoundLink;
	if (_curFoundLink == _lastLinkFound) _curFoundLink = _firstLinkFound;

	wxString txt;
	txt << distance(_firstLinkFound, _curFoundLink)+1 << " of " <<
		distance(_firstLinkFound, _lastLinkFound);
	wxGetApp().getRootWindow()->setNoteText(txt);

	return _curFoundLink->second;
}

hctTextBlock_link* hctLinkManager::getPrevFound() {
	if (_curFoundLink == _searchMap.end()) return NULL;

	if (_curFoundLink == _firstLinkFound) _curFoundLink = _lastLinkFound;
	--_curFoundLink;

	wxString txt;
	txt << distance(_firstLinkFound, _curFoundLink)+1 << " of " <<
		distance(_firstLinkFound, _lastLinkFound);
	wxGetApp().getRootWindow()->setNoteText(txt);

	return _curFoundLink->second;
}

void hctLinkManager::searchLinks() {
	if (!_curLinkBlockFocused) {  
		clearMarked();
		return;
	}

	int found = searchLinks(_curLinkBlockFocused->getText(), _curLinkBlockFocused->getLanguage());
	if (found > 1) {
		wxString txt;
		txt << "Found " << found << " links to the word \"" << _curLinkBlockFocused->getText() << "\"";
		wxGetApp().getRootWindow()->setNoteText(txt);
	}
	else wxGetApp().getRootWindow()->clearDefText();
}

void hctLinkManager::clearMarked() {
	searchLinks("", hctLANG_NONE);  // don't really search, just clear links
}


//			=== editing ===

void hctLinkManager::deleteFocusedLink() {
	if (_curLinkBlockFocused) {
		wxGetApp().getRootWindow()->endNoteEdit();
		wxGetApp().getRootWindow()->clearDefText();
		hctHyperlink *link = _curLinkBlockFocused->getLink();
		if (link) {
			hctTextBlock_link *block = link->popBlock();
			while (block != NULL) {
				removeFromSearchMap(block);
				delete block;
				block = link->popBlock();
			}
			_hyperlinks.erase(link);
			delete link;
		}
		_curLinkBlockFocused = NULL;
		wxGetApp().getRootWindow()->setHasBeenEdited(true);
	}
}

void hctLinkManager::addLink(hctHyperlink *link) {
	if (!_hyperlinks.insert(link).second) {
		wxLogDebug("!!!Link Collision, memory leaks ahead!!!");
		_hyperlinks.erase(link);
		_hyperlinks.insert(link);
	}
	else {
		for (int i=0; i <= link->getNumBlocks(); ++i) {
			addToSearchMap(link->getBlock(i));
		}
	}

	wxGetApp().getRootWindow()->setHasBeenEdited(true);
}


void hctLinkManager::unlink(hctSelectionManager &selectionManager) {
	//	selectionManager.clear();   TODO:  check for problems with overlap or dangling text selector ptrs

	if (_curLinkBlockFocused) {
		wxGetApp().getRootWindow()->endNoteEdit();
		wxGetApp().getRootWindow()->clearDefText();
		hctHyperlink *link = _curLinkBlockFocused->getLink();
		if (!link) return;
		_curLinkBlockFocused = NULL;
		hctTextBlock_link *block = link->popBlock();
		while (block != NULL) {
			removeFromSearchMap(block);
			selectionManager.addSelector(block);  // deletes the block
			block = link->popBlock();
		}
		_hyperlinks.erase(link);
		delete link;
		wxGetApp().getRootWindow()->setHasBeenEdited(true);
	}
}

void hctLinkManager::click(txtitr_t atomUnderMouse) {
if (_curLinkBlockFocused) {   // unfocus on click
		hctHyperlink *link = _curLinkBlockFocused->getLink();
		if (link) link->setFocus(false);
		else wxLogDebug("!! hctLinkManager::click() --  tried to unfocus NULL link!");
		_curLinkBlockFocused = NULL;
	}
}

//  ^^^  (end editing)  ^^^
