#include "stdwx.h"
#pragma check_stack(off) 
#include "hctDictTibEng.h"

#include <algorithm>


void hctDictTibEng::load(string name) {
	string defFileName = name + "/" + name +".def";
	string wdlFileName = name + "/" + name + ".wrd";
//	wxLogDebug("loading %s", defFileName.c_str());

	_defFile = fopen(defFileName.c_str(), "rb");
	if (!_defFile || ferror(_defFile)) {
		wxLogDebug("hctDictTibEng::load()  error opening definition file");
		return;
	}

	FILE *infile = fopen(wdlFileName.c_str(), "rb");
	if (!_defFile || ferror(infile)) {
		wxLogDebug("hctDictTibEng::load()  error opening wordlist file");
		return;
	}

	unsigned int listSize;
	fread(&listSize, sizeof(listSize), 1, infile);
	listSize = wxUINT32_SWAP_ON_BE(listSize);
//	wxString s; s << "listSize: " << listSize; wxLogDebug(s);
	_wordList.reserve(listSize);

	while (!feof(infile)) {
		unsigned short wordSize;
		fread(&wordSize, sizeof(wordSize), 1, infile);
		wordSize = wxUINT16_SWAP_ON_BE(wordSize);
		fread(_buf, 1, wordSize, infile);
		_buf[wordSize]='\0';

		unsigned short defSize;
		unsigned int defOffset;
		fread(&defSize, sizeof(defSize), 1, infile);
		fread(&defOffset, sizeof(defOffset), 1, infile);
		if (feof(infile)) break;

		hctDictEntry entry;
		entry.word = _buf;
		entry.size = wxUINT16_SWAP_ON_BE(defSize);
		entry.offset = wxUINT32_SWAP_ON_BE(defOffset);
		_wordList.push_back(entry);
//		wxLogDebug("ws=%i  ds=%i  do=%i  wrd=%s", wordSize, entry.size, entry.offset, entry.word.c_str());
	
	}
//	wxLogDebug("done loading");	wxLogDebug("");
//	for (wlitr_t pos = _wordList.begin(); pos != _wordList.end(); ++pos) {
//		wxLogDebug(pos->word.c_str());
//	}
//	wxLogDebug("");
}

string hctDictTibEng::define(string word) {
//	wxLogDebug("defining %s", word.c_str());
	string def;
	hctDictFindInfo fi = find(word);
	if (fi.found) {
		if (!fi.exact) def += '<';
		def += _wordList.at(fi.entry).word;
		if (!fi.exact) def += '>';
		def += '\t';
		def += getDef(fi.entry);
	}
	else def = "(definition not found)";
	return def;
}

string hctDictTibEng::getDef(int entry) {
	if (ferror(_defFile)) return "(file error)";
	fseek(_defFile, _wordList.at(entry).offset, SEEK_SET);
	if (ferror(_defFile)) return "(file error)";

	unsigned short defSize = _wordList.at(entry).size;
//	wxString s; s << "defSize: " << defSize; wxLogDebug(s);	wxLogDebug("");
	fread(_buf, 1, defSize, _defFile);
	_buf[defSize]='\0';
	return _buf;
}

hctDictFindInfo hctDictTibEng::find(string word) {
//	wxLogDebug(word.c_str());
	hctDictFindInfo info;
	hctDictEntry findme;
	findme.word = word;
	findme.finding = true;

//	wxLogDebug("doing lower_bound()");
	wlitr_t lb = lower_bound(_wordList.begin(), _wordList.end(), findme);
	if (lb == _wordList.end()) return info;  //  nothing found
//	wxLogDebug("doing upper_bound()");
	wlitr_t ub = upper_bound(lb, _wordList.end(), findme);
	if (lb == ub) return info;  //  nothing found
	info.found = true;
	info.remainder = lb->word.size() - findme.word.size();
	info.entry = distance(_wordList.begin(), lb);
	if (info.remainder == 0) info.exact = true;
	if (++lb == ub) info.unique = true;

//	wxString temp;
//	temp << "lb:   " << distance(_wordList.begin(), lb) << "   ub:   " << distance(_wordList.begin(), ub);
//	wxLogDebug(temp);

	return info;
}

