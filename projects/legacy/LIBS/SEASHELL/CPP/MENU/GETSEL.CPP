#include "menu\_menu.h"

#include <common\old\keyboard.h>

short MenuGetSelection(short keyCode, short *index) {
	char		choice;
	short		i, id;
	MenuItems	*mp;

	choice = MenuIsAlt(keyCode) ? *(MENU_STR+keyCode-kK_ALTQ) : keyCode;
	for (i=0; i<theMenu->num; i++) {
		id = theMenu->id[i];
		mp = MenuIDFind(id);
		if (*(mp->items[0]) == choice) {
			*index = i;
			return (id);
			}
		}
	return (NO_MENU);
	}
