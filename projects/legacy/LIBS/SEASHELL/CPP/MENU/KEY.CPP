#include "menu\_menu.h"

#include <common\old\keyboard.h>

Boolean MenuKey(short ch) {
	Boolean		ok;
	short		i, mMax, ptr, count, top, lef, max, num, loc, width;
	short		vsnum, id, item;
	MenuItems	*mp;
	ENAB_TYPE	enable;

	if (MenuIsFunc(ch))
		ch = ch - kK_F1 + '1';
	else
		ch += 'A' - 1;
	ok = False;
	if ((ch >= 'A' && ch <= 'Z') || (ch >= '1' && ch <= '9')) {
		i = 0;
		mMax = theMenu->num;
		while ((i < mMax) && (!ok)) {
			id = theMenu->id[i++];
			mp = MenuIDFind(id);
			ptr = 0;
			count = mp->num;
			while ((ptr < count) && (!ok))
				if (ch == mp->equiv[++ptr]) {
					enable = mp->enable;
					if ((enable >> ptr) & ENAB_LSB)
						ok = True;
					else {
						ptr = count;
						i = mMax;
						}
					}
			}
		}
	if (ok) {
		item = ptr;
		top = (theMenu->r).Top() + 1;
		lef = mp->loc;
		width = mp->width;
		MenuGetPos(count, width, &top, &lef, &max, &num);
		loc = (ptr > num) ? (ptr-num+1) : 1;
		MenuHilite(id);
		vsnum = MenuOpen(top, lef, num, width);
		MenuDrawItems(vsnum, id, loc, num);
		MenuItemFlash(vsnum, id, ptr-loc, ptr);
		MenuClose(vsnum);
		MenuDoCommand(id, item);
		MenuHilite(NO_MENU);
		}
	return (ok);
	}
