// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <alloc.h>
#include <dir.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\coord.h>
#include <common\keyboard.h>
#include <common\mouse.h>
#include <common\timer.h>

#include <seashell\color.h>
#include <seashell\dialog.h>
#include <seashell\error.h>
#include <seashell\help.h>
#include <seashell\event.h>
#include <seashell\menu.h>
#include <seashell\resclass.h>
#include <seashell\status.h>
#include <seashell\string.h>
#include <seashell\version.h>
#include <seashell\window.h>

#include "inc\pcs100.h"
#include "inc\pcs100.rh"
#include "inc\phelp.h"


static void	HelpDoAbout(void);
static void	PrefsDoExitToDOS(void);
static void	PrefsDoImportExport(void);
static void	PrefsDoNetwork(void);
static void	PrefsDoTiming(void);


void HelpDo(short item) {
	switch (item) {
		case HelpMENU_About:
			HelpDoAbout();
			break;
		case HelpMENU_Help:
			HelpGet();
			break;
		case HelpMENU_ScreenSaver:
			EventActivateSaver();
			break;
		case HelpMENU_Quit:
			doneFlag = true;
			break;
		}
	}


static void HelpDoAbout(void) {
	short		vsnum;
	Rect		r;
	Byte		attr;
	ResVersion	*r_ver;
	char		msgStr[80];

	vsnum = VirtualGetNew(AboutWINDOW, &r);
	HelpPush(HelpHELP_About, MenuItemGet(HelpMENU, HelpMENU_About));
	attr = ColorGet(WindowCOLOR, _COLOR_High);
	r_ver = (ResVersion *) VersionGet(PCS100_VERSION);
	VirtualPrint(vsnum, 0, 1, GIANT, attr, "%s %s", progName, r_ver->ver_str);
	VirtualWrite(vsnum, 0, r.Width()-strlen(r_ver->country)-1, GIANT, r_ver->country, attr);
#ifdef DEMO
	VirtualWrite(vsnum, 2, 1, GIANT, DEMO, attr);
	VirtualWrite(vsnum, 2, r.Width()-strlen(DEMO)-1, GIANT, DEMO, attr);
#endif
	VirtualWrite(vsnum, 4, 1, GIANT, r_ver->date, attr);
	sprintf(msgStr, StringGet(HelpSTRING, HelpSTRING_Free), coreleft());
	VirtualWrite(vsnum, 4, (r.Width()/2)-(strlen(msgStr)/2), GIANT, msgStr, attr);
	StatusPush(GlobalSTATUS, GlobalSTATUS_PressAnyKey);
	EventWaitKey();
	StatusPop();
	HelpPop();
	VirtualDispose(vsnum);
	}


void HelpInit(void) {
	if (host)
		MenuItemEnable(HelpMENU, HelpMENU_Prefs);
	else
		MenuItemDisable(HelpMENU, HelpMENU_Prefs);
	}


void LanguageDo(short item) {
	strcpy(data.rExt, StringGet(LanguageSTRING, item-1));
	doneFlag = restart = true;
	EventPost(EVT_KEY, kK_ESC);
	EventPost(EVT_KEY, kK_ESC);
	}


void LanguageInit(void) {
	short	num, i;
	char	name[MAXPATH], ext[MAXEXT];

	num = MenuItemCount(LanguageMENU);
	for (i=1; i<=num; i++) {
		strcpy(ext, StringGet(LanguageSTRING, i-1));
		sprintf(name, "%s%s%s", progPath, progName, ext);
		if (access(name, 0) == 0) {
			if (strcmp(ext, data.rExt) == 0) {
				MenuItemDisable(LanguageMENU, i);
				MenuItemCheck(LanguageMENU, i, true);
				}
			else {
				MenuItemEnable(LanguageMENU, i);
				MenuItemCheck(LanguageMENU, i, false);
				}
			}
		else {
			MenuItemDisable(LanguageMENU, i);
			MenuItemCheck(LanguageMENU, i, false);
			}
		}
	}


void PrefsDo(short item) {
	switch (item) {
		case PrefsMENU_Timing:
			PrefsDoTiming();
			break;
		case PrefsMENU_ImportExport:
			PrefsDoImportExport();
			break;
		case PrefsMENU_Network:
			PrefsDoNetwork();
			break;
		case PrefsMENU_ExitToDOS:
			PrefsDoExitToDOS();
			break;
		}
	}


static void PrefsDoExitToDOS(void) {
	char	*shell;
	short	vsnum, stat, val;
	Rect	r;

	if ((shell = getenv("COMSPEC")) == NULL)
		shell = "\\command.com";
	vsnum = VirtualGetNew(DOS_WINDOW, &r);
	VirtualFill(vsnum, GIANT_RECT, ' ', ColorGet(DOS_COLOR, _COLOR_Low));
	ScreenSynch();
	printf(StringGet(PrefsSTRING, PrefsSTRING_Exit), progName);
	VirtualShowCursor(vsnum, true);
	stat = system(shell);
	val = errno;
	VirtualDispose(vsnum);
	ScreenSynch();
	if (stat == -1) {
		switch (val) {
			case ENOENT:
				stat = PrefsERROR_ENOENT;
				break;
			case ENOMEM:
				stat = PrefsERROR_ENOMEM;
				break;
			case E2BIG:
				stat = PrefsERROR_E2BIG;
				break;
			case ENOEXEC:
				stat = PrefsERROR_ENOEXEC;
				break;
			}
		Error(Err_Recover, ErrorGet(PrefsERROR, stat));
		}
	}


static void PrefsDoImportExport(void) {
	char		expField[4], expRec[4];
	short		vsnum;
	Rect		r;
	DialogStruct	d1, d2;

	StatusPush(GlobalSTATUS, GlobalSTATUS_Dialog);
	HelpPush(PrefsHELP_ImportExport, MenuItemGet(PrefsMENU, PrefsMENU_ImportExport));
	vsnum = VirtualGetNew(ImportExportWINDOW, &r);
	d1.len = 3;
	d1.top = 0;
	d1.lef = 26;
	d1.prev = &d2;
	d1.next = &d2;
	sprintf(expField, "%d", data.prefs.exportField);
	d1.ans = expField;
	d2.len = 3;
	d2.top = 1;
	d2.lef = 26;
	d2.prev = &d1;
	d2.next = NULL;
	sprintf(expRec, "%d", data.prefs.exportRecord);
	d2.ans = expRec;
	DialogShowPrompts(vsnum, &d1);
	if (DialogEditor(vsnum, &d1) != kK_ESC) {
		data.prefs.exportField = atoi(expField);
		data.prefs.exportRecord = atoi(expRec);
		}
	VirtualDispose(vsnum);
	HelpPop();
	StatusPop();
	}


static void PrefsDoNetwork(void) {
	char		pubSub[4], subRetry[4];
	short		vsnum;
	Rect		r;
	DialogStruct	d1, d2;

	StatusPush(GlobalSTATUS, GlobalSTATUS_Dialog);
	HelpPush(PrefsHELP_Network, MenuItemGet(PrefsMENU, PrefsMENU_Network));
	vsnum = VirtualGetNew(PrefsWINDOW_Network, &r);
	d1.len = 3;
	d1.top = 0;
	d1.lef = 30;
	d1.prev = &d2;
	d1.next = &d2;
	sprintf(pubSub, "%ld", data.prefs.netPubSub/100L);
	d1.ans = pubSub;
	d2.len = 3;
	d2.top = 1;
	d2.lef = 30;
	d2.prev = &d1;
	d2.next = NULL;
	sprintf(subRetry, "%ld", data.prefs.netSubRetry/100L);
	d2.ans = subRetry;
	DialogShowPrompts(vsnum, &d1);
	if (DialogEditor(vsnum, &d1) != kK_ESC) {
		data.prefs.netPubSub = atol(pubSub) * 100L;
		data.prefs.netSubRetry = atol(subRetry) * 100L;
		}
	VirtualDispose(vsnum);
	HelpPop();
	StatusPop();
	}


static void PrefsDoTiming(void) {
	char		saverDelay[4], autoDelay[4];
	short		vsnum;
	Rect		r;
	DialogStruct	d1, d2;

	StatusPush(GlobalSTATUS, GlobalSTATUS_Dialog);
	HelpPush(PrefsHELP_Timing, MenuItemGet(PrefsMENU, PrefsMENU_Timing));
	vsnum = VirtualGetNew(TimingWINDOW, &r);
	d1.len = 3;
	d1.top = 0;
	d1.lef = 22;
	d1.prev = &d2;
	d1.next = &d2;
	sprintf(saverDelay, "%ld", data.prefs.screenSave/6000L);
	d1.ans = saverDelay;
	d2.len = 3;
	d2.top = 1;
	d2.lef = 22;
	d2.prev = &d1;
	d2.next = NULL;
	sprintf(autoDelay, "%ld", data.prefs.autoSave/6000L);
	d2.ans = autoDelay;
	DialogShowPrompts(vsnum, &d1);
	if (DialogEditor(vsnum, &d1) != kK_ESC) {
		data.prefs.screenSave = max(6000L, atol(saverDelay)*6000L);
		data.prefs.autoSave = max(6000L, atol(autoDelay)*6000L);
		EventSetSaverDelay(data.prefs.screenSave);
		nextAutoSave = JiffyTime() + data.prefs.autoSave;
		}
	VirtualDispose(vsnum);
	HelpPop();
	StatusPop();
	}


void PrefsInit(void) {
	EventSetSaverDelay(data.prefs.screenSave);
	}


void PrefsInitData(void) {
	data.prefs.autoSave = 102000L;		// 17 minutes
	data.prefs.screenSave = 180000L;	// 30 minutes
	data.prefs.exportField = ',';		// field separator
	data.prefs.exportRecord = 0x0D;		// record separator
	data.prefs.netPubSub = 30000L;		// 5 minutes (300 seconds)
	data.prefs.netSubRetry = 6000L;		// 1 minute (60 seconds)
	}
