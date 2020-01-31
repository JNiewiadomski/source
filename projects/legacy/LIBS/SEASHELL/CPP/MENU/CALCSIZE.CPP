#include <string.h>

#include "menu\_menu.h"

void MenuCalcSize(MenuItems *mp) {
	short	width, i, len;

	width = 0;
	for (i=1; i <= mp->num; i++) {
		len = strlen(mp->items[i]) + 4;
		if (mp->equiv[i] != ' ')
			len++;
		if (width < len)
			width = len;
		}
	mp->width = width;
	}
