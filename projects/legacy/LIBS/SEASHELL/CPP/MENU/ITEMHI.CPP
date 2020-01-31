#include "menu\_menu.h"

#include <seashell\resclass.h>
#include <seashell\window.h>

void MenuItemHilite(short vsnum, short id, short row, short pos, Boolean hilite) {
	MenuItems	*mp;
	char		buff[RES_MENU_ITEM+1];

	mp = MenuIDFind(id);
	MenuItemMakeStr(buff, mp->check[pos], mp->items[pos], mp->hot[pos], mp->equiv[pos], mp->popup[pos], mp->width);
	if (hilite)
		VirtualWrite2(vsnum, row, 0, GIANT, buff, menuInvLow, menuInvHigh);
	else
		VirtualWrite2(vsnum, row, 0, GIANT, buff, menuHigh, menuHot);
	}
