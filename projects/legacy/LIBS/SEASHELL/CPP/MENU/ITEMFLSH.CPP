#include "menu\_menu.h"

#include <common\ttimer.h>

#include <seashell\window.h>

void MenuItemFlash(short vsnum, short id, short row, short pos) {
	MenuItems	*mp;
	char		buff[80];
	short		i;
	TTimer		delay;

	mp = MenuIDFind(id);
	MenuItemMakeStr(buff, mp->check[pos], mp->items[pos], mp->hot[pos], mp->equiv[pos], mp->popup[pos], mp->width);
	for (i=0; i<3; i++) {
		VirtualWrite2(vsnum, row, 0, GIANT, buff, menuHigh, menuHot);
		ScreenSynch();
		delay.Reset(4L);
		delay.Wait(NULL);
		VirtualWrite2(vsnum, row, 0, GIANT, buff, menuInvLow, menuInvHigh);
		ScreenSynch();
		delay.Reset(4L);
		delay.Wait(NULL);
		}
	}
