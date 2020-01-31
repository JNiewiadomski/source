#include "menu\_menu.h"

short MenuItemNext(short id, short item) {
	MenuItems	*mp;
	short		num, nextItem, i;
	ENAB_TYPE	first, enable;

	mp = MenuIDFind(id);
	num = mp->num;
	first = mp->enable >> 1;
	enable = mp->enable >> item;
	nextItem = item;
	i = 0;
	do {
		if (nextItem == num) {
			nextItem = 1;
			enable = first;
			}
		else {
			nextItem++;
			enable >>= 1;
			}
		i++;
		} while ((i <= num) && !(enable & ENAB_LSB));
	return (i>num ? item : nextItem);
	}
