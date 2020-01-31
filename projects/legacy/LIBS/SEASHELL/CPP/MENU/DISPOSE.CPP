#include "menu\_menu.h"

#include <seashell\resource.h>

// MenuDispose -- frees all memory associated with id and removes its
// reference from mList.

void MenuDispose(short id) {
	MenuItems	*mp;
	char		**ip;

	MenuBarRemove(id);
	mp = MenuIDFree(id);
	ip = mp->items;
	while (*ip)
		r_free(*ip++);
	r_free(mp);
	}
