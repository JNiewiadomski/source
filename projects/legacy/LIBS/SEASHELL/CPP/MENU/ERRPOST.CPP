#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>

void MenuErrorPost(char *eStr, Boolean goAway) {
	MenuItems	*mp;

	mp = MenuIDFind(_MenuErrorMENU);
	if (mp->num < MAX_ERRORS-2) {
		if (mp->num < 3)
			MenuBarInsert(_MenuErrorMENU);
		MenuItemAppend(_MenuErrorMENU, eStr, goAway ? "" : "(", 0, 0);
		if (mp->num == MAX_ERRORS-2)
			MenuItemAppend(_MenuErrorMENU, ErrorGet(_MenuERROR, _MenuERROR_TooManyMenus), "(", 0, 0);
		}
	ErrorSound();
	}
