#ifndef hctLinkManager_H
#define hctLinkManager_H

//#include "hctTextWindow.h"
//#include "hctTextAtom.h"

#ifndef hctTextManager_H
#include "hctTextManager.h"
#endif

#ifndef hctTempLink_H
#include "hctTempLink.h"
#endif

#include <set>
#include <map>
class hctTextBlock_link;
class hctSelectionManager;
class hctHyperlink;

class hctLinkManager
{

public:
	hctLinkManager(text_t *text);
	virtual ~hctLinkManager();

	//@{@name- these provide access to links in the window (used mostly during load/save)
	hctHyperlink *addLink(hctHyperlink *link, hctTempLink &tmpLink);
	void resetLinks();
	//@}

	void addLink(hctHyperlink *link);

	void addToSearchMap(hctTextBlock_link* block);
	void removeFromSearchMap(hctTextBlock_link* block);

	void searchLinks();
	int searchLinks(wxString text, hctLanguage_t language = hctLANG_TIBETAN);
	void clearMarked();

	void deleteAllLinks();

	/// handles link mouseover effect (returns true if focus changed)
	bool refocus(txtitr_t atom, bool lookupTibetan = false);

	bool linkFocused() {
		return (_curLinkBlockFocused != NULL);
	}

	//@{@name- text editing
	/// links any selected text together as a single link
//	void link(hctSelectionManager *selectionManager);

	/// de-links all components of the link under the mouse into separate selection blocks
	void unlink(hctSelectionManager &selectionManager);

	/// deletes the link under the mouse
	void deleteFocusedLink();
	//@}

//	void moveFocusedLink(txtitr_t dest) {
//		if (_curLinkBlockFocused) _curLinkBlockFocused->move(dest);
//	}


	void click(txtitr_t atomUnderMouse);


	hctTextBlock_link* getNextFound();
	hctTextBlock_link* getPrevFound();

//	void debug() {
//		if (_curLinkBlockFocused) _curLinkBlockFocused->debugLinkedList();
//	}

protected:

//	void deleteLink(hctHyperlink *link);

	//	virtual void init();
	text_t *_text;

	typedef multimap<wxString, hctTextBlock_link*> searchMap_t;
	typedef searchMap_t::iterator smapitr_t;
	searchMap_t _searchMap;

	smapitr_t _firstLinkFound;
	smapitr_t _lastLinkFound;
	smapitr_t _curFoundLink;

	set<hctHyperlink*> _hyperlinks;
	hctTextBlock_link *_curLinkBlockFocused;
	bool _linkFocusLocked;

private:
	void freeLinkMemory();

};

#endif

