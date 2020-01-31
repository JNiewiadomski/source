#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>

// MenuIDFind searches through MenuList and returns a pointer to the
// associated menu id.

MenuItems *MenuIDFind(short id, Boolean fail) {
	short	menu_id;

	for (menu_id=0; menu_id<MAX_LIST; menu_id++)
		if (mList[menu_id].id == id)
			return (mList[menu_id].menu);
	if (fail)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_NoMatch));
	return (NULL);
	}
