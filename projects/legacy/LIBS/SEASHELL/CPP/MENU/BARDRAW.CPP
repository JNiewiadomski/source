#include <stdio.h>

#include "menu\_menu.h"

#include <seashell\window.h>

// MenuBarDraw draws the current list of menu titles in the menu bar.

void MenuBarDraw(void) {
	short		i, id;
	MenuItems	*mp;
	char		buff[MAX_BAR_LEN+1];

	VirtualFill(theMenu->vsnum, theMenu->r, ' ', menuInvLow);
	for (i=0; i<theMenu->num; i++) {
		id = theMenu->id[i];
		mp = MenuIDFind(id);
		if (theMenu->current == id) {
			sprintf(buff, " %s ", mp->items[0]);
			VirtualWrite(theMenu->vsnum, 0, mp->loc, GIANT, buff, menuLow);
			}
		else {
			sprintf(buff, " {%c}%s ", *(mp->items[0]), (mp->items[0])+1);
			VirtualWrite2(theMenu->vsnum, 0, mp->loc, GIANT, buff, menuInvLow | mp->flash, menuInvHigh | mp->flash);
			}
		}
	ScreenSynch();
	}
