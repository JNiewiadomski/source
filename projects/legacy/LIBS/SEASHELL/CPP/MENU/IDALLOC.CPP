#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>

// MenuIDAlloc inserts menu into the next available slot in the MenuList.

void MenuIDAlloc(short id, MenuItems *menu) {
	short	menu_id;

	if (mListNum >= MAX_LIST)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_TooManyMenus));
	for (menu_id=0; menu_id<MAX_LIST; menu_id++)
		if (mList[menu_id].id == NO_ID)
			break;
	if (menu_id == MAX_LIST)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_NoneFree));
	mList[menu_id].id = id;
	mList[menu_id].menu = menu;
	mListNum++;
	}
