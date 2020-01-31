#include "menu\_menu.h"

char *MenuItemGet(short id, short item) {
	MenuItems	*mp;

	mp = MenuIDFind(id);
	return (item <= mp->num ? mp->items[item] : "");
	}
