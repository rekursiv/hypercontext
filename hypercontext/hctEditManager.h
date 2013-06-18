#ifndef hctEditManager_H
#define hctEditManager_H

//#ifndef hctTextManager_H
//#include "hctTextManager.h"
//#endif


class hctEditManager;
class hctCursorBlinker : public wxTimer
{
public:
	hctCursorBlinker() {
		_em = NULL;
	}
	void SetOwner(hctEditManager *em) {
		_em = em;
		reset();
	}
	void reset() {
		wxTimer::Start(600);
	}
	void Notify();
	hctEditManager* _em;
};


class hctTextWindow;
/**
This class manages editing functions
*/
class hctEditManager
{

public:
	hctEditManager(hctTextWindow* ownerWin);
	virtual ~hctEditManager();

	void resetCursorBlink() {
		_cursorBlinker.reset();
		if (!_cursorShowing) paintCursor();
	}

	void beginSystemPaint() {
		_cursorBlinker.reset();
		if (_cursorShowing) paintCursor();
	}
	void endSystemPaint() {
		if (!_cursorShowing) paintCursor();
	}

	void hideCursor() {
		if (_cursorShowing) paintCursor();
		_cursorEnabled = false;
	}
	void showCursor() {
		_cursorEnabled = true;
		if (_cursorShowing) paintCursor();
	}
	bool isCursorShown() {
		return _cursorEnabled;
	}

	void updateCursorScreenPos();

	void setCursor(txtitr_t newPos, bool refresh = true, bool updateScreenPosX = true, bool updateScreenPosY = true);
//	void setCursorToPrev(wxRect view);

	txtitr_t getCursorPos() {
		return _cursorPos;
	}

	wxPoint getCursorScreenPos() {
		return _cursorScreenPos;
	}

	void autoLinguify();

	void moveCursorRight();
	void moveCursorLeft();
	void moveCursorUp();
	void moveCursorDown();

	void insertChar(char c);
	void insertNewline();
	void deleteChar(bool delPrev);

	void moveLineUp();

	void paintCursor();

private:
	text_t *text();

	bool _cursorEnabled;
	bool _cursorShowing;
	hctTextWindow *_ownerWin;
	txtitr_t _cursorPos;
	wxPoint _cursorScreenPos;

	hctCursorBlinker _cursorBlinker;
};

#endif

