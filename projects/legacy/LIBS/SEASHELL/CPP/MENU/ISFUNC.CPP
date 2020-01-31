#include "menu\_menu.h"

#include <common\old\keyboard.h>

// MenuIsFunc -- this routine returns true is ch is a function key.
// Otherwise it returns false.

Boolean MenuIsFunc(short ch) {
	return ((ch >= kK_F1 && ch <= kK_F9) ? True : False);
	}
