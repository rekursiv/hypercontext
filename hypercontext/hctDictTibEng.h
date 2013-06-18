#ifndef hctDictTibEng_H
#define hctDictTibEng_H

#include <vector>
#include <string>
using namespace std;

class hctDictEntry
{
public:
	hctDictEntry() {
		offset = 0;
		size = 0;
		finding = false;
	}
	string word;
	int offset;
	int size;
	bool finding;

	bool operator <(const hctDictEntry& rhs) const {
//		wxString test;
//		if (rhs.finding) test << "l:  " << word.c_str() << "\t\t*r: " << rhs.word.c_str() << "\t\t" << word.compare(0, rhs.word.size(), rhs.word);
//		else test << "*l: " << word.c_str() << "\t\tr:  " << rhs.word.c_str() << "\t\t" << rhs.word.compare(0, word.size(), word);
//		wxLogDebug (test);

		if (rhs.finding) return word.compare(0, rhs.word.size(), rhs.word) < 0;		// lower_bound
		else return rhs.word.compare(0, word.size(), word) > 0;						// upper_bound
	}

};

class hctDictFindInfo
{
public:
	hctDictFindInfo() {
		found = exact = unique = false;
		remainder = 0;
		entry = 0;
	}
	int remainder;
	bool found;
	bool exact;
	bool unique;
	int entry;
};

class hctDictTibEng
{
public:
	hctDictTibEng(){
		_defFile=NULL;
	}
	~hctDictTibEng(){}
	hctDictFindInfo find(string word);
	string define(string word);
	string getDef(int entry);
	void load(string path);


private:
	typedef vector<hctDictEntry> wl_t;
	typedef wl_t::iterator wlitr_t;
//	typedef pair<wlitr_t, wlitr_t> wlrng_t;

	FILE* _defFile;
	char _buf[65536];

	wl_t _wordList;
//	wlrng_t _range;
};

#endif