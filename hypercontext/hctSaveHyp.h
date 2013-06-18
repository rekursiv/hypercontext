#ifndef hctTextAtom_H
#include "hctTextAtom.h"
#endif

class hctTextWindow;

/// Saves the information in a text window to an "HYP" file
class hctSaveHyp
{
public:
	void save(wxString fileName, hctTextWindow &textWin);

};