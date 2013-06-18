#include "stdwx.h"
#include "hctCachedFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

hctCachedFont::hctCachedFont(hctFont_t type, int size){
	int romanPtSize=13;
	int tibetanPtSize=29;
	switch (size) {
		case 2: romanPtSize=11; tibetanPtSize=22; break;
		case 1: romanPtSize=9; tibetanPtSize=18; break;
		case 4: romanPtSize=14; tibetanPtSize=36; break;
		case 5: romanPtSize=18; tibetanPtSize=42; break;
	}
#ifdef __WXMAC__
	// fonts are rendered smaller on the Mac
	tibetanPtSize += 8;
	romanPtSize += 2;
#endif
    
	switch (type) {
		case hctFONT_DEFAULT:
			_font.SetFamily(wxROMAN);
			_font.SetPointSize(romanPtSize);
			break;
		case hctFONT_WYLIE:
			_font.SetFamily(wxSWISS);
			_font.SetPointSize(romanPtSize);
			break;
		case hctFONT_TIBETAN:
			_font.SetFaceName("HypercontextTibetan");
			_font.SetPointSize(tibetanPtSize);
			break;
	}
	precalcExtents();
}

void hctCachedFont::precalcExtents() {
	wxMemoryDC dc;
	wxBitmap buf;
	buf.Create(128, 128, -1);
	dc.SelectObject(buf);
	wxString s;
	unsigned short c;
	dc.SetFont(_font);
	for (c=32; c<255; ++c) {
//		s.Printf("%c", c);
        
		dc.GetTextExtent(wxChar(c), &_extents[c].x, &_extents[c].y);
//		wxLogDebug("Glyph Sizes:  %i : %i, %i\n", c, _extents[c].x, _extents[c].y);
	}
}
