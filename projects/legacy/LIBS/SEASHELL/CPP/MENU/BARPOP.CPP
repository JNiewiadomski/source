#include "menu\_menu.h"

#include <seashell\resource.h>
#include <seashell\window.h>

// MenuBarPop pops off the current menu bar list and resets the bar to the
// previous list.

void MenuBarPop(void) {
	short	num;
	MenuDef	*oldMenu;

	for (num=theMenu->num; num; num--)
		MenuDispose(theMenu->id[0]);	// dispose does the id shuffle
	VirtualDispose(theMenu->vsnum);
	oldMenu = theMenu;
	theMenu = theMenu->prev;
	r_free(oldMenu);
	}
