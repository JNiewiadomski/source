#include "menu\_menu.h"

#include <common\old\keyboard.h>

// MenuIsCtrl -- this routine returns true is ch is a control character
// between kK_CTRLA and kK_CTRLZ.  Otherwise it returns false.

Boolean MenuIsCtrl(short ch) {
	return ((ch >= kK_CTRLA && ch <= kK_CTRLZ) ? True : False);
	}
