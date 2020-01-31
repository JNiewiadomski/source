#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>

// MenuIDFree removes menu id from MenuList and returns a pointer to the
// menu.

MenuItems *MenuIDFree(short id) {
	short	menu_id;

	if (mListNum < 1)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_MenuListNil));
	for (menu_id=0; menu_id<MAX_LIST; menu_id++)
		if (mList[menu_id].id == id)
			break;
	if (menu_id == MAX_LIST)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_MenuNotFound));
	mList[menu_id].id = 0;
	mListNum--;
	return (mList[menu_id].menu);
	}
