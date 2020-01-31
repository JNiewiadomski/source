#include "menu\_menu.h"

#include <seashell\resource.h>

void MenuItemDelete(short id, short item) {
	MenuItems	*mp;
	short		num, i;
	ENAB_TYPE	enable;

	mp = MenuIDFind(id);
	num = mp->num;
	if (item <= num) {
		r_free(mp->items[item]);
		for (i=item; i<num; i++) {
			mp->items[i] = mp->items[i+1];
			mp->hot[i] = mp->hot[i+1];
			mp->equiv[i] = mp->equiv[i+1];
			mp->check[i] = mp->check[i+1];
			mp->popup[i]  = mp->popup[i+1];
			mp->help[i]  = mp->help[i+1];
			}
		mp->items[num] = NULL;
		enable = mp->enable;
		mp->enable = (enable|(ENAB_HIGH<<item))&((ENAB_MSB|(enable>>1))|(ENAB_HIGH>>(MAX_ITEMS-item)));
		mp->num = num - 1;
		MenuCalcSize(mp);
		mp->changed = True;
		}
	}
