#include "menu\_menu.h"

short MenuItemCount(short id) {
	MenuItems	*mp;

	mp = MenuIDFind(id);
	return (mp->num);
	}
