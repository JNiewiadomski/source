#include "menu\_menu.h"

#include <stdlib.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>

short		menuLow, menuHigh, menuHot, menuInvLow, menuInvHigh;
MenuFunc	menuCallBack;
short		mListNum=0;
MenuList	mList[MAX_LIST];
MenuDef		*theMenu=NULL;

static void	MenuCleanUp(void);


static void MenuCleanUp(void) {
	short	i;

	while (theMenu != NULL)
		MenuBarPop();
	for (i=0; i<MAX_LIST; i++)
		if (mList[i].id != NO_ID) {
			MenuDispose(mList[i].id);
			mList[i].id = NO_ID;
			}
	}


// MenuInit -- initializes the menu color definitions, assigns the menu call
// back routine, initializes the menu list and reads in the Error menu.

void MenuInit(MenuFunc call_back) {
	short i;

	menuLow = ColorGet(_MenuCOLOR, _COLOR_Low);
	menuHigh = ColorGet(_MenuCOLOR, _COLOR_High);
	menuInvLow = ColorGet(_MenuCOLOR, _COLOR_InvLow);
	menuInvHigh = ColorGet(_MenuCOLOR, _COLOR_InvHigh);
	menuHot = ColorGet(_MenuCOLOR, _COLOR_Hot);
	menuCallBack = call_back;
	for (i=0; i<MAX_LIST; i++)
		mList[i].id = NO_ID;
	MenuNewGet(_MenuErrorMENU);
	atexit(MenuCleanUp);
	}
