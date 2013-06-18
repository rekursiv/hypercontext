#ifndef hctTextLink_H
#define hctTextLink_H

#define hctMAX_BLOCKS 21

#ifndef hctTypes_H
#include "hctTypes.h"
#endif


class hctTextBlock_link;
class hctTextBlock;

/// Links together blocks of text which are related in some way
/**
	A hyperlink holds one or more "link text blocks" (class hctTextBlock_link)
	Each block holds a pointer to the hyperlink that own it, so that any block
	can find out which other blocks it is linked to, if any.

	Each hyperlink can also contain a note that the user can add, delete, or edit.
*/
class hctHyperlink
{
public:
	hctHyperlink();

	///  this class deletes memory -- it would take some work to make it copyable
	~hctHyperlink();

	void pushBlock(hctTextBlock_link *block);  // adds
	//  TODO:  return copy of top block, then DELETE it
	hctTextBlock_link *popBlock();  // relinquish ownership of block, DOES NOT delete it  (must be deleted by caller)
	hctTextBlock_link *topBlock();  // return pointer to top block
	hctTextBlock_link *getBlock(int index) {
		if (index >= _numBlocks || index < 0) return NULL;
		else return _block[index];
	}

	void setFocus(bool focus);
	void setSelected(bool selected);

	void setIndex(int i) {
		_index = i;
	}
	int getIndex() {
		return _index;
	}

	int getNumBlocks() {
		return _numBlocks;
	}

	///  find a block in this link with the same text as the one passed-in
//	bool find(hctTextBlock *block);
	bool find(wxString text, hctLanguage_t language);

private:
	hctTextBlock_link *_block[hctMAX_BLOCKS];  // TODO: make this std::stack<hctTextBlock_link>
	int _numBlocks;
	int _index;

	hctHyperlink(const hctHyperlink&);  ///< no copying allowed
	hctHyperlink& operator=(const hctHyperlink& rhs);  ///< no assignment allowed
};


#endif
