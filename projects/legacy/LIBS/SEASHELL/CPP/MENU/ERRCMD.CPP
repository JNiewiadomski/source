#include "menu\_menu.h"

#include <seashell\seashell.rh>

void MenuErrorCommand(short item) {
	MenuItems	*mp;
	short		i;

	mp = MenuIDFind(_MenuErrorMENU);
	if (item > 0) {
		if (item == 1)
			for (i=mp->num; i>2; i--)
				MenuItemDelete(_MenuErrorMENU, i);
		else {
			if (mp->num == MAX_ERRORS-1)
				MenuItemDelete(_MenuErrorMENU, MAX_ERRORS-1);
			MenuItemDelete(_MenuErrorMENU, item);
			}
		if (mp->num < 3)
			MenuBarRemove(_MenuErrorMENU);
		}
	}
