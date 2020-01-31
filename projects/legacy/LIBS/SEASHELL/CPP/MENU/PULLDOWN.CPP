#include <ctype.h>

#include "menu\_menu.h"

#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\status.h>
#include <seashell\window.h>

short MenuPullDown(short top, short lef, short id) {
	MenuItems	*mp;
	short		loc, nextPtr, ptr, count, width, max, num, vsnum, i, ch;
	Boolean		done1, done2, ok;
	ENAB_TYPE	enable;
	C_Event		event;

	mp = MenuIDFind(id);
	(theMenu->level)++;
	loc = 1;
	nextPtr = 0;
	HelpPush(mp->help[nextPtr], mp->items[nextPtr]);
	done1 = False;
	while (!done1) {
		ptr = -1;
		count = mp->num;
		width = mp->width;
		MenuGetPos(count, width, &top, &lef, &max, &num);
		enable = mp->enable;
		vsnum = MenuOpen(top, lef, num, width);
		MenuDrawItems(vsnum, id, loc, num);
		mp->changed = False;
		done2 = False;
		while (!done2 && !mp->changed) {
			if (ptr != nextPtr) {
				if ((ptr > 0) && ((enable >> ptr) & ENAB_LSB))
					MenuItemHilite(vsnum, id, ptr-loc, ptr, False);
				ptr = nextPtr;
				if (ptr > 0) {
					if (ptr < loc) {
						loc = ptr;
						MenuDrawItems(vsnum, id, loc, num);
						}
					else if (ptr > (loc+num-1)) {
						loc = ptr - num + 1;
						MenuDrawItems(vsnum, id, loc, num);
						}
					MenuItemHilite(vsnum, id, ptr-loc, ptr, True);
					}
				HelpSet(mp->help[ptr], mp->items[ptr]);
				}
			ScreenSynch();
			while (!event.GetNext(EVT_EVERY) && !mp->changed)
				;
			if (event.What() == EVT_KEY)
				switch (event.Message()) {
					case kK_ESCAPE:
						done1 = done2 = True;
						break;
					case kK_RETURN:
						if ((ptr == 0) || (mp->popup[ptr] == 0)) {
							if (ptr > 0) {
								MenuItemFlash(vsnum, id, ptr-loc, ptr);
								VirtualHide(vsnum);
								MenuDoCommand(id, ptr);
								}
							done1 = done2 = True;
							}
						else {
							if (theMenu->level < 2)
								StatusPush(_MenuSTATUS, _MenuSTATUS_PopUp);
							MenuPullDown(top+ptr-loc+2, lef+5, mp->popup[ptr]);
							if (theMenu->level < 2)
								StatusPop();
							nextPtr = ptr;
							}
						break;
					case kK_LEFT:
					case kK_RIGHT:
						if (theMenu->level < 2)
							done1 = done2 = True;
						break;
					case kK_HOME:
					case kK_PGUP:
						nextPtr = MenuItemNext(id, 0);
						break;
					case kK_UP:
						nextPtr = MenuItemPrev(id, ptr);
						break;
					case kK_END:
					case kK_PGDN:
						nextPtr = MenuItemPrev(id, 0);
						break;
					case kK_DOWN:
					case kK_SPACE:
						nextPtr = MenuItemNext(id, ptr);
						break;
					default:
						ch = event.Message();
						if ((MenuIsAlt(ch) || MenuIsCtrl(ch) || MenuIsFunc(ch)) && (theMenu->level < 2))
							done1 = done2 = True;
						else {
							ch = toupper(ch);
							i = 0;
							ok = False;
							while ((++i <= count) && (!ok))
								if ((toupper(mp->hot[i])==ch) && ((enable>>i) & ENAB_LSB))
									ok = True;
							if (ok) {
								if ((ptr > 0) && ((enable >> ptr) & ENAB_LSB))
									MenuItemHilite(vsnum, id, ptr-loc, ptr, False);
								ptr = i-1;
								if (ptr < loc) {
									loc = ptr;
									MenuDrawItems(vsnum, id, loc, num);
									}
								else if (ptr > (loc+num-1)) {
									loc = ptr - num + 1;
									MenuDrawItems(vsnum, id, loc, num);
									}
								MenuItemHilite(vsnum, id, ptr-loc, ptr, True);
								if (ptr == 0 || mp->popup[ptr] == 0) {
									if (ptr > 0) {
										MenuItemFlash(vsnum, id, ptr-loc, ptr);
										VirtualHide(vsnum);
										MenuDoCommand(id, ptr);
										event.SetMessage(kK_RETURN);
										done1 = done2 = True;
										}
									}
								else {
									if (theMenu->level < 2)
										StatusPush(_MenuSTATUS, _MenuSTATUS_PopUp);
									MenuPullDown(top+ptr-loc+2, lef+5, mp->popup[ptr]);
									if (theMenu->level < 2)
										StatusPop();
									nextPtr = ptr;
									}
								}
							}
						break;
					}
			}
		MenuClose(vsnum);
		}
	HelpPop();
	(theMenu->level)--;
	if (theMenu->level == 0)
		MenuHilite(NO_MENU);
	return (event.Message());
	}
