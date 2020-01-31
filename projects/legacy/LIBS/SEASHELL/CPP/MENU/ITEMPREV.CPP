#include "menu\_menu.h"

short MenuItemPrev(short id, short item) {
	MenuItems	*mp;
	short		num, prevItem, i;
	ENAB_TYPE	enable;

	mp = MenuIDFind(id);
	num = mp->num;
	enable = mp->enable;
	prevItem = item;
	i = 0;
	do {
		if (prevItem <= 1)
			prevItem = num;
		else
			prevItem--;
		i++;
		} while ((i <= num) && !((enable >> prevItem) & ENAB_LSB));
	return (i>num ? item : prevItem);
	}
