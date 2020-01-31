#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>

// MenuBarInsert -- inserts menu into menu bar

void MenuBarInsert(short id, short beforeID) {
	short	loc, nextID;

	if (theMenu->num >= MAX_MENUS)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_BarTooMany), id);
	if (beforeID == LAST_MENU)
		loc = theMenu->num;
	else {
		loc = 0;
		while (loc < theMenu->num && theMenu->id[loc] != beforeID)
			loc++;
		}
	while (id != NO_MENU) {
		nextID = theMenu->id[loc];
		theMenu->id[loc++] = id;
		id = nextID;
		}
	theMenu->id[loc] = NO_MENU;
	theMenu->num++;
	MenuBarCalcLoc();
	MenuBarDraw();
	}
