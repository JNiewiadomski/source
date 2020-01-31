#include "menu\_menu.h"

void MenuItemCheck(short id, short item, Boolean checked) {
	MenuItems	*mp;

	mp = MenuIDFind(id);
	if (item <= mp->num) {
		mp->check[item] = checked ? CHECKMARK : ' ';
		mp->changed = True;
		}
	}
