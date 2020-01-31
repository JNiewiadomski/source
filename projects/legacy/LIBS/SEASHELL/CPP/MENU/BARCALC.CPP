#include <string.h>

#include "menu\_menu.h"

// MenuBarCalcLoc recalculates the locations of menu titles in the menu bar.

void MenuBarCalcLoc(void) {
	short		i, loc;
	MenuItems	*mp;

	loc = (theMenu->r).Lef()+1;
	for (i=0; i<theMenu->num; i++) {
		mp = MenuIDFind(theMenu->id[i]);
		mp->loc = loc;
		loc += strlen(mp->items[0]) + 2;
		}
	}
