#include "menu\_menu.h"

void MenuItemEnable(short id, short item) {
	MenuItems	*mp;

	mp = MenuIDFind(id);
	if (item <= mp->num) {
		mp->enable |= (ENAB_LSB << item);
		mp->changed = True;
		}
	}
