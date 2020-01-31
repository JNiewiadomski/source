#include "menu\_menu.h"

#include <common\old\keyboard.h>

// MenuIsAlt -- this routine returns true if ch is the key kK_ALTA
// through kK_ALTZ.  Otherwise it returns false.

Boolean MenuIsAlt(short ch) {
	return ((ch >= kK_ALTQ && ch <= kK_ALTM) ? True : False);
	}
