#include <ctype.h>

#include "menu\_menu.h"

#include <common\old\keyboard.h>

char MenuItemGetEquiv(short id, short item) {
	MenuItems	*mp;
	char		equiv;

	mp = MenuIDFind(id);
	if (item > mp->num)
		return ('\0');
	equiv = mp->equiv[item];
	return (isdigit(equiv) ? equiv - '1' + kK_F1 : equiv - 'A' + kK_CTRLA);
	}
