#ifndef hctTempLink_H
#define hctTempLink_H

typedef std::list<hctTextAtom>::iterator txtitr_t;

class hctTempLink {
public:
	hctTempLink(void){
		start = 0;
		end = 0;
		offset = 0;
		index = -1;
		isInfreq = false;
		isSyn = false;
		isCont = false;
	}
	txtitr_t search_from;
	int start, end, offset, index;
	bool isInfreq, isSyn, isCont;
};

#endif