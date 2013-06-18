#ifndef hctTypes_H
#define hctTypes_H

enum hctLanguage_t
{
	hctLANG_NONE,
	hctLANG_ANY,       // why???
	hctLANG_ENGLISH,
	hctLANG_TIBETAN,
	hctLANG_PHONETIC_TIBETAN,  //  TODO:  enum hctPhoneticRep_t
	hctLANG_SANSKRIT
};

enum hctCtrlChar_t
{
	hctCTRL_NONE,
	hctCTRL_EOF,			// obsolete
	hctCTRL_BREAK,			// \n
	hctCTRL_PARAGRAPH,		// obsolete
	hctCTRL_TOKEN
};

#endif