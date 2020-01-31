#include "menu\_menu.h"

#include <common\old\keyboard.h>

#include <seashell\seashell.rh>
#include <seashell\status.h>

void MenuSelect(short keyCode) {
	short		id, index, ch, new_id;
	Boolean		done;
	MenuItems	*mp;

	if ((id=MenuGetSelection(keyCode, &index)) == NO_MENU)
		return;
	StatusPush(_MenuSTATUS, _MenuSTATUS_PullDown);
	done = False;
	while (!done) {
		MenuHilite(id);
		mp = MenuIDFind(id);
		ch = MenuPullDown((theMenu->r).Top()+1, mp->loc, id);
		switch (ch) {
			case kK_ESCAPE:
			case kK_RETURN:
				done = True;
				break;
			case kK_LEFT:
				if (index == 0)
					index = theMenu->num-1;
				else
					index--;
				id = theMenu->id[index];
				break;
			case kK_RIGHT:
				if (index == theMenu->num-1)
					index = 0;
				else
					index++;
				id = theMenu->id[index];
				break;
			default:
				if (MenuIsAlt(ch)) {
					new_id = MenuGetSelection(ch, &index);
					if (new_id != NO_MENU)
						id = new_id;
					}
				else if (MenuIsCtrl(ch) || MenuIsFunc(ch)) {
					if (MenuKey(ch))
						done = True;
					}
				break;
			}
		}
	StatusPop();
	}
