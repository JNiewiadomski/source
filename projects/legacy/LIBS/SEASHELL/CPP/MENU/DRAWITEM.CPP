#include "menu\_menu.h"

#include <seashell\resclass.h>
#include <seashell\window.h>

void MenuDrawItems(short vsnum, short id, short pos, short num) {
	MenuItems	*mp;
	ENAB_TYPE	enable;
	short		i;
	char		buff[RES_MENU_ITEM+1];

	mp = MenuIDFind(id);
	enable = (pos==1) ? mp->enable : mp->enable>>(pos-1);
	for (i=0; i<num; i++, pos++) {
		MenuItemMakeStr(buff, mp->check[pos], mp->items[pos], mp->hot[pos], mp->equiv[pos], mp->popup[pos], mp->width);
		enable >>= 1;
		if (enable & ENAB_LSB)
			VirtualWrite2(vsnum, i, 0, GIANT, buff, menuHigh, menuHot);
		else
			VirtualWrite2(vsnum, i, 0, GIANT, buff, menuLow, menuLow);
		}
	}
