#include <string.h>

#include "menu\_menu.h"

#include <common\old\colors.h>

#include <seashell\resource.h>

// MenuNew -- creates and initializes a new menu structure and inserts a
// pointer to the structure in the mList.  Make sure menu_id > 0 !!!

void MenuNew(short id, char *name, Boolean flash, short help_id) {
	MenuItems *mp;

	mp = (MenuItems *) r_malloc(sizeof(MenuItems));
	mp->num = 0;
	mp->width = 0;
	mp->enable = ENAB_HIGH;
	if (name==NULL || strlen(name)==0)
		name = "?";
	mp->items[0] = (char *) r_malloc(strlen(name)+1);
	strcpy(mp->items[0], name);
	mp->items[1] = NULL;
	mp->flash = flash ? BLINK : 0;
	mp->help[0] = help_id;
	mp->changed = True;
	MenuIDAlloc(id, mp);
	}
