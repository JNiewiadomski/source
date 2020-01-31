#include "menu\_menu.h"

#include <seashell\window.h>

void MenuGetPos(short count, short width, short *top, short *lef, short *max, short *num) {
	if ((*top+count+2) > SCREEN_HEIGHT-1)
		*top = SCREEN_HEIGHT - count - 3;
	*max = SCREEN_HEIGHT - *top - 3;
	*num = min(*max, count);
	if ((*lef+width+2) > SCREEN_WIDTH)
		*lef = SCREEN_WIDTH - width - 2;
	}
