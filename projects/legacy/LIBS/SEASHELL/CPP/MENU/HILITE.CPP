#include <stdio.h>

#include "menu\_menu.h"

#include <seashell\resclass.h>
#include <seashell\window.h>

void MenuHilite(short id) {
	MenuItems	*mp;
	char		buff[RES_MENU_NAME+1];

	if (theMenu->current != NO_MENU) {
		mp = MenuIDFind(theMenu->current);
		sprintf(buff, " {%c}%s ", *(mp->items[0]), (mp->items[0])+1);
		VirtualWrite2(theMenu->vsnum, 0, mp->loc, GIANT, buff, menuInvLow|mp->flash, menuInvHigh|mp->flash);
		theMenu->current = NO_MENU;
		}
	if (id != NO_MENU) {
		mp = MenuIDFind(id);
		sprintf(buff, " %s ", mp->items[0]);
		VirtualWrite(theMenu->vsnum, 0, mp->loc, GIANT, buff, menuLow);
		theMenu->current = id;
		}
	ScreenSynch();
	}
