#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>

// MenuNewGet reads in the menu resource of id.  All referenced popup menus
// are read in as well.

void MenuNewGet(short id) {
	ResMenu		*r_menu;
	char		*s, *item, *meta;
	ResMenuItem	*r_item;
	short		num, i, popup[MAX_ITEMS+1];

	if ((r_menu = (ResMenu *) ResLoad(R_Menu, id)) == NULL)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_NotFound), id);
	s = r_menu->title;
	MenuNew(id, s, r_menu->flash, r_menu->help_id);
	while (*s++)
		;				// find beginning of menu item data
	r_item = (ResMenuItem *) s;
	num = r_menu->num;
	for (i=0; i<num; i++) {
		item = meta = r_item->item_meta;
		while (*meta++)
			;			// find beginning of meta string
		popup[i] = r_item->popup_id;
		MenuItemAppend(id, item, meta, r_item->help_id, r_item->popup_id);
		while (*meta++)
			;			// find beginning of next menu item data
		r_item = (ResMenuItem *) meta;
		}
	for (i=0; i<num; i++)
		if (popup[i])
			MenuNewGet(popup[i]);
	ResUnLoad(R_Menu, id);
	}
