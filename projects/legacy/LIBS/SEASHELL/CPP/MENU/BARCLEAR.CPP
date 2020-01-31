#include "menu\_menu.h"

// MenuBarClear initializes the menu bar.  Any previously inserted menus are
// removed from the list.

void MenuBarClear(void) {
	theMenu->num = 0;
	theMenu->id[0] = NO_MENU;
	theMenu->level = 0;
	theMenu->current = NO_MENU;
	}
