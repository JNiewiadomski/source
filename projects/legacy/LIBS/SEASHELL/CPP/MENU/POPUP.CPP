#include "menu\_menu.h"

#include <common\old\keyboard.h>

#include <seashell\seashell.rh>
#include <seashell\status.h>

void MenuPopup(short id) {
	short		ch;
	MenuItems	*mp;

	// We ask MenuIDFind to return a pointer to the popup menu, returning
	// NULL if it can't be found.  A popup menu may not have been read in
	// yet.  If it hasn't, we read it in and ask for a pointer to it
	// once more, this time letting it fail if the id is invalid.

	if ((mp = MenuIDFind(id, False)) == NULL) {
		MenuNewGet(id);
		mp = MenuIDFind(id);
		}
	StatusPush(_MenuSTATUS, _MenuSTATUS_PopUp);
	do {
		ch = MenuPullDown((theMenu->r).Top()+1, mp->loc, id);
		} while (ch != kK_ESCAPE && ch != kK_RETURN);
	StatusPop();
	}
