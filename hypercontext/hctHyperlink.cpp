#include "stdwx.h"
#include "hctHyperlink.h"
#include "hctTextBlock_link.h"

//#include "hctApp.h"
//#include "hctRootFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctHyperlink::hctHyperlink() {
	_numBlocks = 0;
	_index = -1;
}

hctHyperlink::~hctHyperlink() {
	hctTextBlock_link *block = popBlock();
	while (block) {
		delete block;
		block = popBlock();
	}
}


void hctHyperlink::pushBlock(hctTextBlock_link *block) {
	if (!block) return;
	if (_numBlocks >= hctMAX_BLOCKS) {
		delete block;
		return;
	}

	block->setLink(this);
	_block[_numBlocks] = block;
	++_numBlocks;
}


hctTextBlock_link *hctHyperlink::popBlock() {
	if (_numBlocks > 0) return _block[--_numBlocks];
	else return NULL;
}

hctTextBlock_link *hctHyperlink::topBlock() {
	if (_numBlocks > 0) return _block[_numBlocks-1];
	else return NULL;
}


void hctHyperlink::setFocus(bool focus) {
	for (int i=0; i<_numBlocks; i++) {
		hctTextBlock_link *curBlock = _block[i];
		curBlock->setStyleFocus(focus);
		curBlock->refresh();
		if (curBlock->getNext()) {  // find "root" block of nested link
			for (;;) {
				if (curBlock->getNext()) curBlock = curBlock->getNext();
				else break;
			}
			curBlock->setStyleFocus(focus);
			curBlock->refresh();
		}
	}
}


void hctHyperlink::setSelected(bool selected) {
	for (int i=0; i<_numBlocks; i++) {
		if (selected != _block[i]->getStyleSelect()) {
			_block[i]->setStyleSelect(selected);
			_block[i]->refresh();
		}
	}
}
/*
bool hctHyperlink::find(hctTextBlock *block) {

	for (int i=0; i<_numBlocks; i++) {
		if (*_block[i]==*block) return true;
	}
	return false;
}
*/

bool hctHyperlink::find(wxString text, hctLanguage_t language) {
	for (int i=0; i<_numBlocks; i++) {
		if (_block[i]->textMatches(text, language)) return true;
	}
	return false;
}
