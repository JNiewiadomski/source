// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <stdlib.h>

#include <common\common.h>
#include <common\colors.h>
#include <common\coord.h>
#include <common\jiffy.h>
#include <common\keyboard.h>

#include <seashell\menu.h>
#include <seashell\window.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "mach.h"
#include "graph.h"
#include "query.h"

void GraphAdjustMenu(void) {
	if (mchMax > 0) {
		MenuItemEnable(GraphMENU, GraphMENU_Rates);
		MenuItemEnable(GraphMENU, GraphMENU_PercComplete);
		}
	else {
		MenuItemDisable(GraphMENU, GraphMENU_Rates);
		MenuItemDisable(GraphMENU, GraphMENU_PercComplete);
		}
	}


void GraphDoMenu(short item) {
	switch (item) {
		case GraphMENU_Rates:
			GraphRates();
			break;
		case GraphMENU_PercComplete:
			GraphPercComplete();
			break;
		}
	}


void GraphInit(void) {
	GraphInitScreen();
	}


#define GPH_EXT		".GPH"

#define GRAPH_MAX	18
#define BAR_CHAR	'Ü'

#define RATE_START_COL	17
#define RATE_MAX_DIV	1000

#define ACTIVE_ATTR	(BWHT)
#define PASSIVE_ATTR	(MAG)

#define PERC_START_COL	27

enum G_TYPE { G_RATES, G_PERC_COMPLETE };

static ScreenInfo	gs;
static G_TYPE		graphType;


static void	GraphDisplay(void);
static void	GraphRedisplay(Boolean update);
static void	GraphStatus(short counter, short row);
static void	GraphUpdateRates(short counter);
static void	GraphUpdatePercComplete(short counter);


void GraphAdjustPtrs(short counter) {
	short mPos;

	if (mchMax > 0) {
		mPos = mchMax-1;
		while ((mPos > 0) && (mach[mPos]->counter != counter))
			mPos--;
		gs.nextPos = mPos;
		gs.num = (mchMax>gs.max) ? gs.max : mchMax;
		if (mPos < gs.loc)
			gs.loc = mPos;
		else if (mPos > gs.loc+gs.num-1)
			gs.loc = mPos-gs.num+1;
		else if (gs.loc+gs.num > mchMax)
			gs.loc = max(0, mchMax-gs.num);
		GraphRedisplay(true);
		}
	}


static void GraphDisplay(void) {
	short	i;

	for (i=gs.loc; i<gs.loc+gs.num; i++)
		GraphUpdate(mach[i]->counter);
	}


void GraphDoScreen(int ch) {
	Boolean changed;

	changed = false;
	switch (ch) {
		case K_HOME:
			if (gs.pos > 0) {
				gs.nextPos = 0;
				changed = true;
				}
			break;
		case K_UP:
			if (gs.pos > 0) {
				gs.nextPos = gs.pos-1;
				changed = true;
				}
			break;
		case K_PGUP:
			if (gs.pos > gs.num-1) {
				gs.nextPos = gs.pos-gs.num+1;
				changed = true;
				}
			else if (gs.pos > 0) {
				gs.nextPos = 0;
				changed = true;
				}
			break;
		case K_DOWN:
			if (gs.pos < mchMax-1) {
				gs.nextPos = gs.pos+1;
				changed = true;
				}
			break;
		case K_PGDN:
			if (gs.pos+gs.num-1 < mchMax) {
				gs.nextPos = gs.pos+gs.num-1;
				changed = true;
				}
			else if (gs.pos < mchMax-1) {
				gs.nextPos = mchMax-1;
				changed = true;
				}
			break;
		case K_END:
			if (gs.pos < mchMax-1) {
				gs.nextPos = mchMax-1;
				changed = true;
				}
			break;
		case K_SPACE:
			if (mchMax > 1) {
				gs.nextPos = (gs.pos==mchMax-1) ? 0 : gs.pos+1;
				changed = true;
				}
			break;
		case K_RETURN:
			if (graphType == G_RATES)
				GraphPercComplete();
			break;
		case K_ESC:
			if (graphType == G_PERC_COMPLETE)
				GraphRates();
			break;
		}
	if (changed)
		GraphRedisplay(false);
	}


void GraphInitScreen(void) {
	gs.pos = MCH_NONE;
	gs.nextPos = gs.loc = 0;
	gs.max = GRAPH_MAX;
	gs.num = (mchMax > gs.max) ? gs.max : mchMax;
	}


Boolean GraphIsDisplayed(short counter) {
	short i;

	for (i=gs.loc; i<gs.loc+gs.num; i++)
		if (mach[i]->counter == counter)
			return (true);
	return (false);
	}


static void GraphRedisplay(Boolean update) {
	short	old;
	Rect	r;

	if ((gs.pos != gs.nextPos) || update) {
		old = gs.pos;
		gs.pos = gs.nextPos;
		if (old != MCH_NONE)
			GraphUpdate(mach[old]->counter);
		if (gs.pos < gs.loc) {
			gs.loc = gs.pos;
			GraphDisplay();
			}
		else if (gs.pos > gs.loc+gs.num-1) {
			gs.loc = gs.pos-gs.num+1;
			GraphDisplay();
			}
		else if (update)
			GraphDisplay();
		else
			GraphUpdate(mach[gs.pos]->counter);
		if (update && (gs.num<gs.max)) {
			r.Set(gs.num+3, 0, scrn.bot, scrn.rig);
			VirtualFill(scrn.num, r, ' ', scrn.lo);
			}
		}
	}


void GraphPercComplete(void) {
	Point	p;

	scrn.cur = SCREEN_GRAPH;
	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, GraphVIDEO_PercComplete, p);

	graphType = G_PERC_COMPLETE;
	GraphRedisplay(true);
	ScreenTitleSet(MenuItemGet(GraphMENU, GraphMENU_PercComplete));
	}


void GraphRates(void) {
	Point	p;

	scrn.cur = SCREEN_GRAPH;
	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, GraphVIDEO_Rates, p);
	VirtualPrint(scrn.num, 0, RATE_START_COL, 1, ACTIVE_ATTR, "%c", BAR_CHAR);
	VirtualPrint(scrn.num, 1, RATE_START_COL, 1, PASSIVE_ATTR, "%c", BAR_CHAR);

	graphType = G_RATES;
	GraphRedisplay(true);
	ScreenTitleSet(MenuItemGet(GraphMENU, GraphMENU_Rates));
	}


static void GraphStatus(short counter, short row) {
	switch (counterStatus(counter)) {
		case QUERY_STATUS_RUN:
			VirtualPrint(scrn.num, row, 2, GIANT, BWHT, "-");
			VirtualPrint(scrn.num, row, 3, GIANT, BWHT, "-");
			VirtualPrint(scrn.num, row, 4, GIANT, LGRN, "%c", 0x0F);
			break;
		case QUERY_STATUS_IDLE:
			VirtualPrint(scrn.num, row, 2, GIANT, LRED, "%c", 0x0F);
			VirtualPrint(scrn.num, row, 3, GIANT, BWHT, "-");
			VirtualPrint(scrn.num, row, 4, GIANT, BWHT, "-");
			break;
		case QUERY_STATUS_SETUP:
			VirtualPrint(scrn.num, row, 2, GIANT, BWHT, "-");
			VirtualPrint(scrn.num, row, 3, GIANT, YEL, "%c", 0x0F);
			VirtualPrint(scrn.num, row, 4, GIANT, BWHT, "-");
			break;
		case QUERY_STATUS_UPDATING:
			VirtualPrint(scrn.num, row, 2, GIANT, BWHT | BLINK, "-");
			VirtualPrint(scrn.num, row, 3, GIANT, BWHT | BLINK, "-");
			VirtualPrint(scrn.num, row, 4, GIANT, BWHT | BLINK, "-");
			break;
		default:
			VirtualPrint(scrn.num, row, 2, GIANT, BWHT, "-");
			VirtualPrint(scrn.num, row, 3, GIANT, BWHT, "-");
			VirtualPrint(scrn.num, row, 4, GIANT, BWHT, "-");
			break;
		}
	}


void GraphUpdate(short counter) {
	switch (graphType) {
		case G_RATES:
			GraphUpdateRates(counter);
			break;
		case G_PERC_COMPLETE:
			GraphUpdatePercComplete(counter);
			break;
		}
	}


static void GraphUpdateRates(short counter) {
	short	mPos, v, attr1, attr2;
	Rect	r;
	DWord	curRate, aveRate, val1, val2, from;
	Byte	fact;

	mPos = MachineFind(counter);
	if (mPos<gs.loc || mPos>gs.loc+gs.num-1)
		return;
	v = mPos-gs.loc+3;
	fact = factor(counter);
	if (fact == 0)
		fact = 1;
	curRate = (currentRate(counter) / (RATE_MAX_DIV * fact)) + RATE_START_COL;
	aveRate = (calcAverageRate(counter, QRUN) / (RATE_MAX_DIV * fact)) + RATE_START_COL;
	if (curRate > aveRate) {
		attr1 = PASSIVE_ATTR;
		attr2 = ACTIVE_ATTR;
		val1 = aveRate;
		val2 = curRate;
		}
	else {
		attr1 = ACTIVE_ATTR;
		attr2 = PASSIVE_ATTR;
		val1 = curRate;
		val2 = aveRate;
		}
	r.Set(v, 0, v, scrn.rig);
	VirtualFill(scrn.num, r, ' ', scrn.lo);
	VirtualPrint(scrn.num, v, 0, GIANT, BWHT, "%c", mPos==gs.pos ? 0x10 : ' ');
	GraphStatus(counter, v);
	VirtualPrint(scrn.num, v, 11, GIANT, BWHT, "%0*d", MACH_CNTR_LEN, counter);
	if (val1 > RATE_START_COL) {
		r.Set(v, RATE_START_COL, v, (short)val1);
		VirtualFill(scrn.num, r, BAR_CHAR, attr1);
		if (val1 <= RATE_START_COL)
			from = RATE_START_COL;
		else
			from = val1 + 1;
		r.Set(v, (short)from, v, (short)val2);
		VirtualFill(scrn.num, r, BAR_CHAR, attr2);
		}
	else if (val2 > RATE_START_COL) {
		r.Set(v, RATE_START_COL, v, (short)val2);
		VirtualFill(scrn.num, r, BAR_CHAR, attr2);
		}
	}


static void GraphUpdatePercComplete(short counter) {
	short	mPos, v, perc;
	Rect	r;

	mPos = MachineFind(counter);
	if (mPos<gs.loc || mPos>gs.loc+gs.num-1)
		return;
	v = mPos-gs.loc+3;
	perc = (percCompleted(counter) / 2) + PERC_START_COL;
	r.Set(v, 0, v, scrn.rig);
	VirtualFill(scrn.num, r, ' ', scrn.lo);
	VirtualPrint(scrn.num, v, 0, GIANT, BWHT, "%c", mPos==gs.pos ? 0x10 : ' ');
	GraphStatus(counter, v);
	VirtualPrint(scrn.num, v, 11, GIANT, BWHT, "%0*d", MACH_CNTR_LEN, counter);
	VirtualWrite(scrn.num, v, 18, GIANT, q_percCompleted(counter), BWHT);
	if (perc > PERC_START_COL) {
		r.Set(v, PERC_START_COL, v, perc);
		VirtualFill(scrn.num, r, BAR_CHAR, PASSIVE_ATTR);
		}
	}
