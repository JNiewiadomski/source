#include "menu\_menu.h"

void MenuItemSetEquiv(short id, short item, char equiv) {
	MenuItems	*mp;

	mp = MenuIDFind(id);
	if (item <= mp->num) {
		mp->equiv[item] = 'A' + equiv - 1;
		mp->changed = True;
		}
	}
