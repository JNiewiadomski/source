#include "menu\_menu.h"

// MenuBarRemove -- removes menu from menu bar

void MenuBarRemove(short id) {
	short	loc;

	if (theMenu == NULL)
		return;
	loc = 0;
	while ((loc < theMenu->num) && (theMenu->id[loc] != id))
		loc++;
	if (theMenu->id[loc] != id)
		return;
	if (theMenu->current == id)
		theMenu->current = NO_MENU;
	while (id != NO_MENU) {
		id = theMenu->id[loc+1];
		theMenu->id[loc++] = id;
		}
	theMenu->num--;
	MenuBarCalcLoc();
	MenuBarDraw();
	}
