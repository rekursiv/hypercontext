#ifndef hctCachedFont_H
#define hctCachedFont_H

enum hctFont_t
{
	hctFONT_DEFAULT,
	hctFONT_TIBETAN,
	hctFONT_WYLIE,
	hctFONT_EXTRA
};

/// The thing that is "cached" here is the "extents" of each glyph in the font
class hctCachedFont{
public:
	hctCachedFont(hctFont_t type, int size);
//	~hctCachedFont();

	wxFont *getFont() {
		return &_font;
	}

	wxSize getExtents(unsigned char c) {
		return _extents[c];
	}

private:
	void precalcExtents();

	wxFont _font;
	wxSize _extents[256];

};

#endif