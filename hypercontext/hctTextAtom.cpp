#include "stdwx.h"
#include "hctTextAtom.h"
#include "hctTextWindow.h"
#include "hctTextBlock_select.h"
#include "hctTextBlock_link.h"
#include "hctFontManager.h"

#include "hctApp.h"  // for hctTextAtom::debug()
#include "hctTextManager.h"  // for hctTextAtom::debug()

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


hctTextAtom::hctTextAtom() {
	for (int i=0; i<hctTIB_GLYPHS_MAX; i++) _tibetanGlyphs[i]=0;

#ifdef __WXDEBUG__
	_showCtrl = true;
#else
	_showCtrl = false;
#endif

	_ownerWin = NULL;

	_ctrlChar = hctCTRL_NONE;
	_language = hctLANG_NONE;

	_needsRefresh = true;

	_renderWylie = true;
	_unstackable = true;
	_wylieEditMode = false;
	_wylieDisplayMode = false;

	_selectBlock = NULL;
	_linkBlock = NULL;

	_index = -1;
	_column = -1;

	_hasInsertMarker = false;

	_startsLayer = false;

	_isNote = false;

	_fontSize = 3;
}

hctTextAtom::~hctTextAtom() {}

void hctTextAtom::setOwnerWin(hctTextWindow* win) {
	_ownerWin = win;
}

void hctTextAtom::refresh() {
	_needsRefresh = true;
	if (_ownerWin) _ownerWin->onAtomRefresh(_rect, _index);
}

hctTextBlock* hctTextAtom::getBlock() {
	if (_selectBlock) return _selectBlock;
	return _linkBlock;
}

void hctTextAtom::prepareForDelete() {
	if (_selectBlock) {
		_selectBlock->prepareForDelete(this);
	}
	if (_linkBlock) {
		_linkBlock->prepareForDelete(this);
	}
}

void hctTextAtom::resetDefaults() {
	_startsLayer = false;
	_isNote = false;
	_ctrlChar = hctCTRL_NONE;
	_language = hctLANG_NONE;
}

void hctTextAtom::prepareFonts(wxDC &dc) {
	if (_language == hctLANG_TIBETAN) {
		if (_renderWylie) dc.SetFont(*hctFontManager::getInstance()->getCachedFont(hctFONT_WYLIE, _fontSize)->getFont());
		else dc.SetFont(*hctFontManager::getInstance()->getCachedFont(hctFONT_TIBETAN, _fontSize)->getFont());
	}
	else dc.SetFont(*hctFontManager::getInstance()->getCachedFont(hctFONT_DEFAULT, _fontSize)->getFont());
}

void hctTextAtom::calcSize() {
	if (isNote()) {
		_rect.SetSize(hctFontManager::getInstance()->getCachedFont(hctFONT_DEFAULT, _fontSize)->getExtents('A'));
	}
	else if (_language == hctLANG_TIBETAN) {
		_rect.SetSize(hctFontManager::getInstance()->getCachedFont(hctFONT_TIBETAN, _fontSize)->getExtents(_tibetanGlyphs[0]));
#ifndef __WXMAC__        
		_rect.height-=8+(3*_fontSize);  // make the Tibetan a bit more compact
#endif
        
		if (_renderWylie) {
			wxSize size;
			int width = 0;
			for (int i=0; i<_letters.size(); ++i) {
				size = hctFontManager::getInstance()->getCachedFont(hctFONT_WYLIE, _fontSize)->getExtents(_letters.GetChar(i));
				width += size.GetWidth();
			}
			_rect.width = width;
		}

		// for testing fonts
//		if (_tibetanGlyphs[0]==45) _rect.width = 20;  //////////////////////////////////////////////////////////////////////////
		
	}
	else {
		_rect.SetSize(hctFontManager::getInstance()->getCachedFont(hctFONT_DEFAULT, _fontSize)->getExtents(_letters.GetChar(0)));
		_rect.height+=2;  // spread the English out a bit
	}
}

void hctTextAtom::paint(wxDC &dc) {

	if (!isCtrl()) {
		prepareFonts(dc);

		if (_linkBlock) _linkBlock->paint(dc, _rect);
//		else dc.SetTextForeground(wxColor(128, 128, 128));
//		else dc.SetTextForeground(wxColor(0, 64, 128));
		else dc.SetTextForeground(*wxBLACK);

		if (_selectBlock) _selectBlock->paint(dc, _rect, _startsLayer);

		dc.SetLogicalFunction(wxCOPY);

		if (_isNote) {
			dc.SetTextForeground(wxColor(255, 220, 0));
			dc.SetFont(*hctFontManager::getInstance()->getCachedFont(hctFONT_DEFAULT, _fontSize)->getFont());
			wxString str;
			str << "[]";
			dc.DrawText(str, _rect.x, _rect.y);
		}
		else {
			if (_language == hctLANG_TIBETAN) {
				if (_renderWylie) dc.DrawText(_letters, _rect.x, _rect.y+_rect.height/4);
				else {
                    wxString s = wxString::From8BitData(_tibetanGlyphs);
                    dc.DrawText(s, _rect.x, _rect.y-5);
                }
			}
			else {
				dc.DrawText(_letters, _rect.x, _rect.y);
			}
		}

	}
	else {
		if (_showCtrl) {             // for debugging control chars
			int x, y;
			x = _rect.x;
			y = _rect.y + _rect.height/3;

			dc.SetTextForeground(wxColor(255, 120, 0));
			dc.SetFont(*hctFontManager::getInstance()->getCachedFont(hctFONT_DEFAULT, 1)->getFont());
			wxString str;
            char dcc;
			if (_ctrlChar == hctCTRL_BREAK) dcc = 0xab;
			else if (_ctrlChar == hctCTRL_PARAGRAPH) dcc = 0xb6;
			else if (_ctrlChar == hctCTRL_EOF) dcc = 0x87;
            str << wxString::From8BitData(&dcc, 1);
			dc.DrawText(str, x, y);
		}
	}

	if (_hasInsertMarker) {
		dc.SetLogicalFunction(wxXOR);
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawLine(_rect.GetLeft(), _rect.GetTop(), _rect.GetLeft(), _rect.GetBottom());
		dc.DrawLine(_rect.GetLeft()+1, _rect.GetTop(), _rect.GetLeft()+4, _rect.GetTop());
		dc.DrawLine(_rect.GetLeft(), _rect.GetBottom(), _rect.GetLeft()+4, _rect.GetBottom());
	}


/*
// start/end of link debugging
	if (_endsLink) {
		dc.SetLogicalFunction(wxXOR);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(_rect.x, _rect.y+_rect.height, _rect.width, 5);
	}
	if (_startsLink) {
		dc.SetLogicalFunction(wxXOR);
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(_rect.x, _rect.y, _rect.width, 5);
	}
*/
}


// temp debugging code
void hctTextAtom::debug() {

	int row = _row->getIndex();

	wxString msg;
	msg.Printf("i=%i  r=%i  c=%i  l=%i", 
		_index, row, _column, _language);

	((wxFrame *)(wxGetApp().GetTopWindow()))->SetStatusText(msg);

}

