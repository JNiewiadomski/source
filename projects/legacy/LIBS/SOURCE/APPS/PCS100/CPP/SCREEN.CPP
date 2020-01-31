// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <common\common.h>
#include <common\coord.h>
#include <common\getargs.h>
#include <common\jiffy.h>

#include <seashell\color.h>
#include <seashell\help.h>
#include <seashell\menu.h>
#include <seashell\string.h>
#include <seashell\window.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "mach.h"
#include "job.h"
#include "graph.h"


ScreenStruct	scrn;


void ScreenAdjustMenu(void) {
	if (scrn.cur == SCREEN_NONE) {
		MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(ScreenSTRING, ScreenSTRING_PrintNothing));
		MenuItemDisable(ReportMENU, ReportMENU_Print);
		}
	else
		MenuItemEnable(ReportMENU, ReportMENU_Print);
	if (host)
		MenuItemEnable(ReportMENU, ReportMENU_Delete);
	else
		MenuItemDisable(ReportMENU, ReportMENU_Delete);
	}


void ScreenDo(int ch) {
	switch (scrn.cur) {
		case SCREEN_ALL_MCH:
			MachineScreenDoAll(ch, QRUN);
			break;
		case SCREEN_SNGL_MCH:
			MachineScreenDoSingle(ch, QRUN);
			break;
		case SCREEN_ALL_JOB:
			doAllJobsScreen(ch, QRUN);
			break;
		case SCREEN_SNGL_JOB:
			doSingleJobScreen(ch, QRUN);
			break;
		case SCREEN_ALL_SMCH:
			MachineScreenDoAll(ch, QSHIFT);
			break;
		case SCREEN_SNGL_SMCH:
			MachineScreenDoSingle(ch, QSHIFT);
			break;
		case SCREEN_ALL_SJOB:
			doAllJobsScreen(ch, QSHIFT);
			break;
		case SCREEN_SNGL_SJOB:
			doSingleJobScreen(ch, QSHIFT);
			break;
		case SCREEN_GRAPH:
			GraphDoScreen(ch);
			break;
		}
	}


void ScreenInit(void) {
	Rect	r;

	scrn.bot = SCREEN_HEIGHT-3;
	scrn.rig = SCREEN_WIDTH-3;
	scrn.cur = SCREEN_NONE;
	scrn.num = VirtualGetNew(ScreenWINDOW, &r);
	HelpPush(ScreenNoneHELP, progName);
	scrn.lo = ColorGet(ScreenCOLOR, _COLOR_Low);
	scrn.hi = ColorGet(ScreenCOLOR, _COLOR_High);
	scrn.inv = ColorGet(ScreenCOLOR, _COLOR_InvLow);
	ScreenMonitorNothing();
	}


void ScreenMonitorNothing(void) {
	Point	p;

	scrn.cur = SCREEN_NONE;
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, StartUpVIDEO, p);
	HelpSet(ScreenNoneHELP, progName);
	ScreenTitleSet(progName);
	}


void ScreenPrint(void) {
	switch (scrn.cur) {
		case SCREEN_ALL_MCH:
			MachineScreenPrintAll();
			break;
		case SCREEN_SNGL_MCH:
			MachineScreenPrintSingle();
			break;
		case SCREEN_ALL_JOB:
			printAllJobsScreen();
			break;
		case SCREEN_SNGL_JOB:
			printSingleJobScreen();
			break;
		case SCREEN_ALL_SMCH:
			printShiftAllMchScreen();
			break;
		case SCREEN_SNGL_SMCH:
			printShiftSingleMchScreen();
			break;
		case SCREEN_ALL_SJOB:
			printShiftAllJobsScreen();
			break;
		case SCREEN_SNGL_SJOB:
			printShiftSingleJobScreen();
			break;
		}
	}


void ScreenTitleSet(char *title) {
	VirtualSetTitle(scrn.num, title);
	}
