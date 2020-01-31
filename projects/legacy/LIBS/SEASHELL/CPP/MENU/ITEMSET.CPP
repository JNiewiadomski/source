#include <string.h>

#include "menu\_menu.h"

#include <seashell\resource.h>

void MenuItemSet(short id, short item, char *s) {
	MenuItems	*mp;

	mp = MenuIDFind(id);
	if (item <= mp->num) {
		r_free(mp->items[item]);
		mp->items[item] = (char *) r_malloc(strlen(s)+1);
		strcpy(mp->items[item], s);
		MenuCalcSize(mp);
		mp->changed = True;
		}
	}
