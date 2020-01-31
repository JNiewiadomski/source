#include "menu\_menu.h"

#include <seashell\seashell.rh>
#include <seashell\error.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\window.h>

// MenuBarPush pushes the old menu bar into the internal menu bar stack and
// reads in a new menu bar specified by id*index.  The new menu bar is then
// drawn.

void MenuBarPush(short id) {
	ResBar	*r_bar;
	MenuDef	*mdp;
	short	i;

	if ((r_bar = (ResBar *) ResLoad(R_Bar, id)) == NULL)
		Error(Err_Fatal, ErrorGet(_MenuERROR, _MenuERROR_BarNotFound), id);
	mdp = (MenuDef *) r_malloc(sizeof(MenuDef));
	mdp->r = r_bar->loc;
	mdp->vsnum = VirtualNew(1, mdp->r.Width(), WMBAR, mdp->r, NULL, menuInvLow);
	mdp->prev = theMenu;
	theMenu = mdp;
	MenuBarClear();
	for (i=0; i<r_bar->num; i++) {
		MenuNewGet(r_bar->id[i]);
		MenuBarInsert(r_bar->id[i]);
		}
	ResUnLoad(R_Bar, id);
	}
