// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <ctype.h>
#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\coord.h>
#include <common\general.h>
#include <common\getargs.h>
#include <common\jiffy.h>
#include <common\keyboard.h>
#include <common\system.h>

#include <seashell\color.h>
#include <seashell\dialog.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\form.h>
#include <seashell\help.h>
#include <seashell\iu.h>
#include <seashell\menu.h>
#include <seashell\printer.h>
#include <seashell\resource.h>
#include <seashell\status.h>
#include <seashell\string.h>
#include <seashell\window.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "mach.h"
#include "graph.h"
#include "query.h"

#define MACH_CLASS_LEN	1

#define MCH_EXT		".MCH"
#define M_CLASS		'A'

#define OPER_NAME_LEN	30

short		mchMax=0;
MachineStruct	*mach[MAX_MACHINES];

static char		mchName[MAXPATH];
static ScreenInfo	ms;


static void	MachineAdjustPointers(short counter);
static void	MachineAllScreenDisplay(QTYPE type);
static void	MachineAllScreenRedisplay(Boolean update, QTYPE type);
static int	MachineCompare(const void *elem1, const void *elem2);
static Boolean	MachineDelete(short counter);
static void	MachineInitScreen(void);
static void	MachineInstall(void);
static void	MachineList(void);
static void	MachineListDisplay(short vsnum, short pos, short num, short attr);
static void	MachineLoad(void);
static void	MachineMonitorAll(void);
static void	MachineSave(void);
static void	MachineSingleMonitor(void);
static void	MachineSingleScreenRedisplay(QTYPE type);

static char	*OperatorGetName(short operNumber);


void MachineAdjustMenu(void) {
	if (host)
		MenuItemEnable(MachineMENU, MachineMENU_Install);
	else
		MenuItemDisable(MachineMENU, MachineMENU_Install);
	if (mchMax > 0) {
		MenuItemEnable(MachineMENU, MachineMENU_All);
		MenuItemEnable(MachineMENU, MachineMENU_Single);
		MenuItemEnable(MachineMENU, MachineMENU_List);
		}
	else {
		MenuItemDisable(MachineMENU, MachineMENU_All);
		MenuItemDisable(MachineMENU, MachineMENU_Single);
		MenuItemDisable(MachineMENU, MachineMENU_List);
		}
	}


static void MachineAdjustPointers(short counter) {
	short mPos;

	resetQuery();
	if (mchMax > 0) {
		mPos = mchMax-1;
		while ((mPos > 0) && (mach[mPos]->counter != counter))
		mPos--;
		ms.nextPos = mPos;
		ms.num = (mchMax>ms.max) ? ms.max : mchMax;
		if (mPos < ms.loc)
			ms.loc = mPos;
		else if (mPos > ms.loc+ms.num-1)
			ms.loc = mPos-ms.num+1;
		else if (ms.loc+ms.num > mchMax)
			ms.loc = max(0, mchMax-ms.num);
		switch (scrn.cur) {
			case SCREEN_ALL_MCH:
				MachineAllScreenRedisplay(true, QRUN);
				break;
			case SCREEN_SNGL_MCH:
				MachineSingleScreenRedisplay(QRUN);
				break;
			case SCREEN_ALL_SMCH:
				MachineAllScreenRedisplay(true, QSHIFT);
				break;
			case SCREEN_SNGL_SMCH:
				MachineSingleScreenRedisplay(QSHIFT);
				break;
			case SCREEN_GRAPH:
				GraphAdjustPtrs(counter);
				break;
			}
		}
	else if (scrn.cur != SCREEN_NONE) {
		ScreenMonitorNothing();
		MachineInitScreen();
		}
	}


static void MachineAllScreenDisplay(QTYPE type) {
	short	i;

	if (type == QRUN)
		for (i=ms.loc; i<ms.loc+ms.num; i++)
			MachineAllScreenUpdate(mach[i]->counter);
	else
		for (i=ms.loc; i<ms.loc+ms.num; i++)
			MachineShiftAllScreenUpdate(mach[i]->counter);
	}


static void MachineAllScreenRedisplay(Boolean update, QTYPE type) {
	short	old;
	Rect	r;

	if ((ms.pos != ms.nextPos) || update) {
		old = ms.pos;
		ms.pos = ms.nextPos;
		if (old != MCH_NONE) {
			if (type == QRUN)
				MachineAllScreenUpdate(mach[old]->counter);
			else
				MachineShiftAllScreenUpdate(mach[old]->counter);
			}
		if (ms.pos < ms.loc) {
			ms.loc = ms.pos;
			MachineAllScreenDisplay(type);
			}
		else if (ms.pos > ms.loc+ms.num-1) {
			ms.loc = ms.pos-ms.num+1;
			MachineAllScreenDisplay(type);
			}
		else if (update)
			MachineAllScreenDisplay(type);
		else {
			if (type == QRUN)
				MachineAllScreenUpdate(mach[ms.pos]->counter);
			else
				MachineShiftAllScreenUpdate(mach[ms.pos]->counter);
			}
		if (update && (ms.num<ms.max)) {
			r.Set(ms.num+3, 0, scrn.bot, scrn.rig);
			VirtualFill(scrn.num, r, ' ', scrn.lo);
			}
		}
	}


void MachineAllScreenUpdate(short counter) {
	short	mPos, v, attr;
	Rect r;

	mPos = MachineFind(counter);
	if (mPos<ms.loc || mPos>ms.loc+ms.num-1)
		return;
	v = mPos-ms.loc+3;
	attr = (mPos==ms.pos) ? scrn.inv : scrn.hi;
	r.Set(v, 0, v, scrn.rig);
	VirtualFill(scrn.num, r, ' ', attr);
	VirtualPrint(scrn.num, v,  3, GIANT, attr, "%0*d", MACH_CNTR_LEN, counter);
	VirtualWrite(scrn.num, v, 12, GIANT, q_jobNumber(counter, QRUN), attr);
	VirtualWrite(scrn.num, v, 22, GIANT, q_status(counter), attr);
	VirtualWrite(scrn.num, v, 34, GIANT, q_totalCount(counter, QRUN), attr);
	VirtualWrite(scrn.num, v, 44, GIANT, q_currentRate(counter), attr);
	VirtualWrite(scrn.num, v, 53, GIANT, q_runTime(counter, QRUN), attr);
	VirtualWrite(scrn.num, v, 65, GIANT, q_idleTime(counter, QRUN), attr);
	}


static int MachineCompare(const void *elem1, const void *elem2) {
	MachineStruct	**m1, **m2;

	if ((m1=(MachineStruct **)elem1) == NULL)
		return (1);
	if ((m2=(MachineStruct **)elem2) == NULL)
		return (-1);
	return ((**m1).counter - (**m2).counter);
	}


void MachineCleanUp(void) {
	short	i;

	for (i=0; i<mchMax; i++)
		if (mach[i] != NULL)
			r_free(mach[i]);
	}


static Boolean MachineDelete(short counter) {
	short	i;

	if ((i=MachineFind(counter)) != MCH_NONE) {
		r_free(mach[i]);
		mchMax--;
		while (i < mchMax) {
			mach[i] = mach[i+1];
			i++;
			}
		}
	return ((Boolean)(i != MCH_NONE));
	}


void MachineDoMenu(short item) {
	switch (item) {
		case MachineMENU_All:
			MachineMonitorAll();
			break;
		case MachineMENU_Single:
			MachineSingleMonitor();
			break;
		case MachineMENU_Install:
			MachineInstall();
			break;
		case MachineMENU_List:
			MachineList();
			break;
		}
	}


short MachineFind(short counter) {
	short	i;

	for (i=0; i<mchMax; i++)
		if (mach[i]->counter == counter)
			return (i);
	return (MCH_NONE);
	}


void MachineInit(void) {
	MachineLoad();
	MachineInitScreen();
	}


static void MachineInitScreen(void) {
	ms.pos = MCH_NONE;
	ms.nextPos = ms.loc = 0;
	ms.max = 18;
	ms.num = (mchMax > ms.max) ? ms.max : mchMax;
	}


static void MachineInstall(void) {
	char		name[MACH_NAME_LEN+1], date[IU_DATE_LEN+1], ans[MACH_CNTR_LEN+1], *pval;
	short		vsnum, attr, mPos, counter, i;
	Rect		r;
	Boolean		done, done1, dirty;
	int		ch;
	DialogStruct	d1, d2;
	MachineStruct	*mPtr;
	Dialog		dlg;

	if (scrn.cur != SCREEN_NONE)
		counter = mach[ms.pos]->counter;
	HelpPush(MachineHELP_Install, MenuItemGet(MachineMENU, MachineMENU_Install));
	StatusPush(DialogSTATUS, DialogSTATUS_Key);
	vsnum = VirtualGetNew(MachineWINDOW_Install, &r);
	attr = ColorGet(DialogCOLOR, _COLOR_Low);
	VirtualWrite(vsnum, 2, 33, GIANT, IUGetDatePrompt(), attr);
	VirtualWrite(vsnum, 4, 1, r.Width(), StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attr);
	d1.len = MACH_NAME_LEN;
	d1.top = 1;
	d1.lef = 15;
	d1.prev = &d2;
	d1.next = &d2;
	d2.len = IU_DATE_LEN;
	d2.top = 2;
	d2.lef = 17;
	d2.prev = &d1;
	d2.next = NULL;
	mPos = 0;
	done = dirty = false;
	while (!done) {
		VirtualPrint(vsnum, 0, r.Width()-2, MACH_CLASS_LEN, attr, "%c", ' ');
		itoa(mPos, ans, 10);
		ch = DialogStringEditor(vsnum, 0, 17, ans, MACH_CNTR_LEN);
		for (pval=ans; *pval; pval++)
			if (!isdigit(*pval))
			ch = 0;
			mPos = atoi(ans);
			switch (ch) {
				case K_ESC:
					if (dirty && (mchMax > 1)) {
						qsort(mach, mchMax, sizeof(MachineStruct *), MachineCompare);
						MachineAdjustPointers(counter);
						}
					done = true;
					break;
				case K_UP:
					mPos = (mPos==0) ? MAX_MACHINES-1 : mPos-1;
					break;
				case K_PGUP:
					if ((mPos-=10) < 0)
						mPos = 0;
					break;
				case K_DOWN:
					mPos = (mPos==MAX_MACHINES-1) ? 0 : mPos+1;
					break;
				case K_PGDN:
					if ((mPos+=10) > MAX_MACHINES-1)
						mPos = MAX_MACHINES-1;
					break;
				case K_RETURN:
					if (mPos<0 || mPos>MAX_MACHINES-1)
						mPos = 0;
					else {
						mPtr = ((i=MachineFind(mPos))==MCH_NONE) ? NULL : mach[i];
						if (mPtr == NULL) {
							*name = NULL;
							IUIDate2Str(SysDate(), date);
							d1.ans = name;
							d2.ans = date;
							dlg = (DialogEditor(vsnum, &d1)==K_ESC) ? D_Cancel : D_Ok;
							if (strlen(trim(name)) == 0)
								dlg = D_Delete;
							VirtualPrint(vsnum, 0, r.Width()-2, MACH_CLASS_LEN, attr, "%c", M_CLASS);
							}
						else {
							strcpy(name, (*mPtr).name);
							IUIDate2Str((*mPtr).date, date);
							d1.ans = name;
							d2.ans = date;
							DialogShowPrompts(vsnum, &d1);
							dlg = D_Cancel;
							VirtualPrint(vsnum, 0, r.Width()-2, MACH_CLASS_LEN, attr, "%c", mPtr->class_type);
							}
						do {
							done1 = true;
							switch (DialogOkModifyDeleteCancel(vsnum, 4, 1, dlg)) {
								case D_Ok:
									if (mPtr == NULL) {
										mPtr = (MachineStruct *) r_malloc(sizeof(MachineStruct));
										mach[mchMax++] = mPtr;
										mPtr->counter = mPos;
										createQuery(mPos);
										MachineAdjustPointers(counter);
										}
									mPtr->class_type = M_CLASS;
									strcpy(mPtr->name, trim(name));
									IUStr2IDate(date, &(mPtr->date));
									dirty = true;
									break;
								case D_Modify:
									dlg = (DialogEditor(vsnum, &d1)==K_ESC) ? D_Cancel : D_Ok;
									done1 = false;
									break;
								case D_Delete:
									if (MachineDelete(mPos)) {
										deleteQuery(mPos);
										MachineAdjustPointers(counter);
										dirty = true;
										}
									break;
								case D_Cancel:
									break;
								}
							} while (!done1);
						VirtualWrite(vsnum, 4, 1, r.Width(), StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attr);
						DialogClearPrompts(vsnum, &d1);
						mPos = (mPos==MAX_MACHINES-1) ? 0 : mPos+1;
						}
					break;
				}
		}
	VirtualDispose(vsnum);
	StatusPop();
	HelpPop();
	if (dirty) {
		MachineSave();
		QueryDataSave();
		}
	}


static void MachineList(void) {
	char		date[IU_DATE_LEN+1], time[IU_TIME_LEN+1], *s;
	short		vsnum, attrHigh, mPos, newPos, mNum, i;
	Rect		r;
	Boolean		done;
	EventRecord	myEvent;

	HelpPush(MachineHELP_List, MenuItemGet(MachineMENU, MachineMENU_List));
	vsnum = VirtualGetNew(MachineWINDOW_List, &r);
	attrHigh = ColorGet(WindowCOLOR, _COLOR_High);
	mPos = MCH_NONE;
	newPos = 0;
	mNum = min(mchMax, r.Height()-2);
	if (mNum == mchMax)
		StatusPush(GlobalSTATUS, GlobalSTATUS_List);
	else
		StatusPush(GlobalSTATUS, GlobalSTATUS_ListPosition);
	done = false;
	while (!done) {
		if (mPos != newPos) {
			mPos = newPos;
			MachineListDisplay(vsnum, mPos, mNum, attrHigh);
			}
		while (!EventGetNext(EVT_EVERY, &myEvent))
			;
		switch (myEvent.what) {
			case EVT_KEY :
				switch (myEvent.message) {
					case K_ESC:
						done = true;
						break;
					case K_HOME:
						newPos = 0;
						break;
					case K_UP:
						if (mPos > 0)
							newPos = mPos-1;
						break;
					case K_PGUP:
						if (mPos > mNum-1)
							newPos = mPos-mNum+1;
						else
							newPos = 0;
						break;
					case K_DOWN:
						if (mPos < mchMax-mNum)
							newPos = mPos+1;
						break;
					case K_PGDN:
						if (mPos+mNum-1 < mchMax-mNum)
							newPos = mPos+mNum-1;
						else
							newPos = mchMax-mNum;
						break;
					case K_END:
						newPos = mchMax-mNum;
						break;
					default:
						if (myEvent.message == PrinterGetKey()) {
							s = FormGetFirst(MachineFORM_List);
							if (s && PrinterOpen()) {
								IUIDate2Str(SysDate(), date);
								IUITime2Str(SysTime(), time);
								PrinterWrite(s, date, time);
								PrinterWrite(FormGetNext());
								PrinterWrite(FormGetNext());
								for (i=0; i<mchMax; i++) {
									PrinterWrite("     %0*d  ", MACH_CNTR_LEN, mach[i]->counter);
									PrinterWrite("%-*s  ", MACH_NAME_LEN, mach[i]->name);
									IUIDate2Str(mach[i]->date, date);
									PrinterWrite("%s\n", date);
									}
								PrinterWrite("\f");
								PrinterClose();
								}
							}
						break;
					}
				break;
			}
		}
	StatusPop();
	VirtualDispose(vsnum);
	HelpPop();
	}


static void MachineListDisplay(short vsnum, short pos, short num, short attr) {
	short	i;
	char	temp[IU_DATE_LEN+1];

	for (i=0; i<num; i++) {
		VirtualPrint(vsnum, 2+i, 5, MACH_CNTR_LEN, attr, "%0*d", MACH_CNTR_LEN, mach[pos]->counter);
		VirtualWrite(vsnum, 2+i, 9, MACH_NAME_LEN, mach[pos]->name, attr);
		IUIDate2Str((*mach[pos]).date, temp);
		VirtualWrite(vsnum, 2+i, 41, IU_DATE_LEN, temp, attr);
		pos++;
		}
	}


static void MachineLoad(void) {
	short	handle, i;
	char	sig[sizeof(PCS100_SIG)];

	StatusPush(MachineSTATUS, MachineSTATUS_Loading);
	sprintf(mchName, "%s%s%s", dataPath, progName, MCH_EXT);
	if ((handle = open(mchName, O_RDONLY | O_BINARY)) != -1) {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &mchMax, sizeof(mchMax));
			for (i=0; i<mchMax; i++) {
				mach[i] = (MachineStruct *) r_malloc(sizeof(MachineStruct));
				read(handle, mach[i], sizeof(MachineStruct));
				}
			}
		else
			MenuErrorPost(ErrorGet(MachineERROR, MachineERROR_Load), true);
		close(handle);
		}
	StatusPop();
	}


static void MachineSave(void) {
	short	handle, i;

	if (!host)
		return;
	StatusPush(MachineSTATUS, MachineSTATUS_Saving);
	if ((handle = open(mchName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) != -1) {
		write(handle, PCS100_SIG, sizeof(PCS100_SIG));
		write(handle, &mchMax, sizeof(mchMax));
		for (i=0; i<mchMax; i++)
			write(handle, mach[i], sizeof(MachineStruct));
		close(handle);
		}
	else
		MenuErrorPost(ErrorGet(MachineERROR, MachineERROR_Save), true);
	StatusPop();
	}


void MachineShiftAllMonitor(void) {
	Point	p;

	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, ShiftVIDEO_AllMachine, p);
	scrn.cur = SCREEN_ALL_SMCH;
	HelpSet(ShiftHELP_AllMachines, MenuItemGet(ShiftMENU, ShiftMENU_AllMachines));
	MachineAllScreenRedisplay(true, QSHIFT);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(MachineSTRING, MachineSTRING_ShiftAll));
	ScreenTitleSet(MenuItemGet(ShiftMENU, ShiftMENU_AllMachines));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	}


void MachineShiftAllScreenUpdate(short counter) {
	short	mPos, v, attr;
	Rect	r;

	mPos = MachineFind(counter);
	if (mPos<ms.loc || mPos>ms.loc+ms.num-1)
		return;
	v = mPos-ms.loc+3;
	attr = (mPos==ms.pos) ? scrn.inv : scrn.hi;
	r.Set(v, 0, v, scrn.rig);
	VirtualFill(scrn.num, r, ' ', attr);
	VirtualPrint(scrn.num, v, 3, GIANT, attr, "%0*d", MACH_CNTR_LEN, counter);
	VirtualWrite(scrn.num, v, 12, GIANT, q_jobNumber(counter, QSHIFT), attr);
	VirtualWrite(scrn.num, v, 22, GIANT, q_status(counter), attr);
	VirtualWrite(scrn.num, v, 34, GIANT, q_totalCount(counter, QSHIFT), attr);
	VirtualWrite(scrn.num, v, 44, GIANT, q_averageRate(counter, QSHIFT), attr);
	VirtualWrite(scrn.num, v, 55, GIANT, q_runTime(counter, QSHIFT), attr);
	VirtualWrite(scrn.num, v, 67, GIANT, q_idleTime(counter, QSHIFT), attr);
	}


void MachineShiftSingleMonitor(void) {
	Point	p;

	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(1, 0);
	VirtualGetVideo(scrn.num, ShiftVIDEO_SingleMachine, p);
	scrn.cur = SCREEN_SNGL_SMCH;
	HelpSet(ShiftHELP_SingleMachine, MenuItemGet(ShiftMENU, ShiftMENU_SingleMachine));
	MachineSingleScreenRedisplay(QSHIFT);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(MachineSTRING, MachineSTRING_ShiftSingle));
	ScreenTitleSet(MenuItemGet(ShiftMENU, ShiftMENU_SingleMachine));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	}


void MachineShiftSingleScreenUpdate(short counter) {
	char	date[IU_DATE_LEN+1];

	if (counter != mach[ms.pos]->counter)
		return;
	VirtualWrite(scrn.num, 1, 19, MACH_NAME_LEN, mach[ms.pos]->name, scrn.hi);
	IUIDate2Str(mach[ms.pos]->date, date);
	VirtualWrite(scrn.num, 1, 66, IU_DATE_LEN, date, scrn.hi);
	VirtualWrite(scrn.num, 2, 19, OPER_NAME_LEN, OperatorGetName(atoi(q_opNumber(counter, QRUN))), scrn.hi);
	VirtualPrint(scrn.num, 6, 25, MACH_CNTR_LEN, scrn.hi, "%0*d", MACH_CNTR_LEN, counter);
	VirtualWrite(scrn.num, 6, 66, GIANT, q_status(counter), scrn.hi);
	VirtualWrite(scrn.num, 7, 19, GIANT, q_jobNumber(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 7, 66, GIANT, q_opNumber(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 8, 19, GIANT, q_dateStart(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 8, 29, GIANT, q_timeStart(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 8, 69, GIANT, q_shftNmbr(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 14, 19, GIANT, q_runTime(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 14, 66, GIANT, q_totalCount(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 15, 19, GIANT, q_idleTime(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 15, 66, GIANT, q_batchCnt(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 16, 19, GIANT, q_setupTime(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 16, 66, GIANT, q_setupCount(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 17, 19, GIANT, q_totalTime(counter, QSHIFT), scrn.hi);
	VirtualWrite(scrn.num, 17, 68, GIANT, q_averageRate(counter, QSHIFT), scrn.hi);
	}


static void MachineSingleMonitor(void) {
	Point	p;

	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, MachineVIDEO_Single, p);
	scrn.cur = SCREEN_SNGL_MCH;
	MachineSingleScreenRedisplay(QRUN);
	HelpSet(MachineHELP_Single, MenuItemGet(MachineMENU, MachineMENU_Single));
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(MachineSTRING, MachineSTRING_Single));
	ScreenTitleSet(MenuItemGet(MachineMENU, MachineMENU_Single));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	}


static void MachineSingleScreenRedisplay(QTYPE type) {
	ms.pos = ms.nextPos;
	if (type == QRUN)
		MachineSingleScreenUpdate(mach[ms.pos]->counter);
	else
		MachineShiftSingleScreenUpdate(mach[ms.pos]->counter);
	}


void MachineSingleScreenUpdate(short counter) {
	if (counter != mach[ms.pos]->counter)
		return;
	VirtualWrite(scrn.num, 0, 10, 25, mach[ms.pos]->name, scrn.hi);
	VirtualWrite(scrn.num, 0, 47, OPER_NAME_LEN, OperatorGetName(atoi(q_opNumber(counter, QRUN))), scrn.hi);
	VirtualPrint(scrn.num, 2, 25, GIANT, scrn.hi, "%0*d", MACH_CNTR_LEN, counter);
	VirtualWrite(scrn.num, 2, 66, GIANT, q_status(counter), scrn.hi);
	VirtualWrite(scrn.num, 3, 19, GIANT, q_jobNumber(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 3, 66, GIANT, q_opNumber(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 4, 19, GIANT, q_dateStart(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 4, 29, GIANT, q_timeStart(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 4, 69, GIANT, q_shftNmbr(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 7, 19, GIANT, q_pcsRqrd(counter), scrn.hi);
	VirtualWrite(scrn.num, 7, 67, GIANT, q_outDelay(counter), scrn.hi);
	VirtualWrite(scrn.num, 8, 23, GIANT, q_preset(counter), scrn.hi);
	VirtualWrite(scrn.num, 8, 67, GIANT, q_outDuration(counter), scrn.hi);
	VirtualWrite(scrn.num, 9, 25, GIANT, q_pieces(counter), scrn.hi);
	VirtualWrite(scrn.num, 9, 67, GIANT, q_outType(counter), scrn.hi);
	VirtualWrite(scrn.num, 12, 14, GIANT, q_runTime(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 12, 43, GIANT, q_batchCnt(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 12, 69, GIANT, q_totalCount(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 13, 14, GIANT, q_idleTime(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 13, 47, GIANT, q_downCnt(counter), scrn.hi);
	VirtualWrite(scrn.num, 13, 69, GIANT, q_setupCount(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 14, 14, GIANT, q_setupTime(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 14, 47, GIANT, q_preset(counter), scrn.hi);
	VirtualWrite(scrn.num, 14, 71, GIANT, q_currentRate(counter), scrn.hi);
	VirtualWrite(scrn.num, 15, 14, GIANT, q_totalTime(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 15, 71, GIANT, q_averageRate(counter, QRUN), scrn.hi);
	VirtualWrite(scrn.num, 18, 21, GIANT, q_pcsTilDone(counter), scrn.hi);
	VirtualWrite(scrn.num, 19, 27, GIANT, q_idle(counter), scrn.hi);
	VirtualWrite(scrn.num, 19, 66, GIANT, q_noIdleTime(counter), scrn.hi);
	VirtualWrite(scrn.num, 20, 28, GIANT, q_percCompleted(counter), scrn.hi);
	VirtualWrite(scrn.num, 20, 66, GIANT, q_withIdleTime(counter), scrn.hi);
	}


void MachineSubscribe(void) {
	short		handle, i, num, pos, cur=MCH_NONE;
	struct stat	stat_buf;
	char		sig[sizeof(PCS100_SIG)], unused[MAX_MACHINES], *s;
	MachineStruct	new_mach;
	static long	pubSubTime=0;

	if ((handle = open(mchName, O_RDONLY | O_BINARY)) == -1)
		return;
	fstat(handle, &stat_buf);
	if (pubSubTime != stat_buf.st_ctime) {
		pubSubTime = stat_buf.st_ctime;
		StatusPush(MachineSTATUS, MachineSTATUS_Loading);
		if (scrn.cur != SCREEN_NONE)
			cur = mach[ms.pos]->counter;
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			memset(unused, '\0', MAX_MACHINES);
			for (i=0; i<mchMax; i++)
				unused[mach[i]->counter] = M_CLASS;
			read(handle, &num, sizeof(num));
			for (i=0; i<num; i++) {
				read(handle, &new_mach, sizeof(new_mach));
				if ((pos=MachineFind(new_mach.counter)) == MCH_NONE) {
					pos = mchMax++;
					mach[pos] = (MachineStruct *) r_malloc(sizeof(MachineStruct));
					createQuery(new_mach.counter);
					MachineAdjustPointers(cur);
					}
				memmove(mach[pos], &new_mach, sizeof(new_mach));
				unused[new_mach.counter] = '\0';
				}
			for (i=0, s=unused; i<MAX_MACHINES; i++, s++)
				if (*s==M_CLASS && MachineDelete(i)) {
					deleteQuery(i);
					MachineAdjustPointers(cur);
					}
			if (mchMax > 1) {
				qsort(mach, mchMax, sizeof(MachineStruct *), MachineCompare);
				MachineAdjustPointers(cur);
				}
			}
		StatusPop();
		}
	close(handle);
	}












void MachineScreenDoAll(int ch, QTYPE type)
  {
    Boolean changed;

    changed = false;
    switch (ch) {
      case K_HOME   : if (ms.pos > 0) {
                        ms.nextPos = 0;
                        changed = true;
                        }
                      break;
      case K_UP     : if (ms.pos > 0) {
                        ms.nextPos = ms.pos-1;
                        changed = true;
                        }
                      break;
      case K_PGUP   : if (ms.pos > ms.num-1) {
                        ms.nextPos = ms.pos-ms.num+1;
                        changed = true;
                        }
                      else if (ms.pos > 0) {
                        ms.nextPos = 0;
                        changed = true;
                        }
                      break;
      case K_DOWN   : if (ms.pos < mchMax-1) {
                        ms.nextPos = ms.pos+1;
                        changed = true;
                        }
                      break;
      case K_PGDN   : if (ms.pos+ms.num-1 < mchMax) {
                        ms.nextPos = ms.pos+ms.num-1;
                        changed = true;
                        }
                      else if (ms.pos < mchMax-1) {
                        ms.nextPos = mchMax-1;
                        changed = true;
                        }
                      break;
      case K_END    : if (ms.pos < mchMax-1) {
                        ms.nextPos = mchMax-1;
                        changed = true;
                        }
                      break;
      case K_SPACE  : if (mchMax > 1) {
                        ms.nextPos = (ms.pos==mchMax-1) ? 0 : ms.pos+1;
                        changed = true;
                        }
                      break;
      case K_RETURN : if (type == QRUN)
                        MachineSingleMonitor();
                      else
                        MachineShiftSingleMonitor();
                      break;
      }
    if (changed)
      MachineAllScreenRedisplay(false, type);
    }


static void MachineMonitorAll(void) {
	Point	p;

	scrn.cur = SCREEN_ALL_MCH;
	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, MachineVIDEO_All, p);
	HelpSet(MachineHELP_All, MenuItemGet(MachineMENU, MachineMENU_All));
	MachineAllScreenRedisplay(true, QRUN);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(MachineSTRING, MachineSTRING_All));
	ScreenTitleSet(MenuItemGet(MachineMENU, MachineMENU_All));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	}


Boolean MachineIsAllDisplayed(short counter) {
	short i;

	for (i=ms.loc; i<ms.loc+ms.num; i++)
		if (mach[i]->counter == counter)
			return (true);
	return (false);
	}


void MachineScreenPrintAll(void) {
	short	i, counter;
	char	*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	s = FormGetFirst(MachineFORM_All);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		for (i=0; i<mchMax; i++) {
			counter = mach[i]->counter;
			PrinterWrite("%02d       ", counter);
			PrinterWrite("%s  ", q_jobNumber(counter, QRUN));
			PrinterWrite("%s  ", q_status(counter));
			PrinterWrite("%s  ", q_totalCount(counter, QRUN));
			PrinterWrite("%s   ", q_currentRate(counter));
			PrinterWrite("%s  ", q_runTime(counter, QRUN));
			PrinterWrite("%s\n", q_idleTime(counter, QRUN));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	}


void MachineScreenDoSingle(int ch, QTYPE type) {
	Boolean changed;

	changed = false;
	switch (ch) {
		case K_HOME:
			if (ms.pos > 0) {
				ms.nextPos = 0;
				changed = true;
				}
			break;
		case K_UP:
			if (ms.pos > 0) {
				ms.nextPos = ms.pos-1;
				changed = true;
				}
			break;
		case K_PGUP:
			if (ms.pos > ms.num-1) {
				ms.nextPos = ms.pos-ms.num+1;
				changed = true;
				}
			else if (ms.pos > 0) {
				ms.nextPos = 0;
				changed = true;
				}
			break;
		case K_DOWN:
			if (ms.pos < mchMax-1) {
				ms.nextPos = ms.pos+1;
				changed = true;
				}
			break;
		case K_PGDN:
			if (ms.pos+ms.num-1 < mchMax) {
				ms.nextPos = ms.pos+ms.num-1;
				changed = true;
				}
			else if (ms.pos < mchMax-1) {
				ms.nextPos = mchMax-1;
				changed = true;
				}
			break;
		case K_END:
			if (ms.pos < mchMax-1) {
				ms.nextPos = mchMax-1;
				changed = true;
				}
			break;
		case K_SPACE:
			if (mchMax > 1) {
				ms.nextPos = (ms.pos==mchMax-1) ? 0 : ms.pos+1;
				changed = true;
				}
			break;
		case K_ESC:
			if (type == QRUN)
				MachineMonitorAll();
			else
				MachineShiftAllMonitor();
			break;
		}
	if (changed)
		MachineSingleScreenRedisplay(type);
	}


Boolean MachineIsSingleDisplayed(short counter) {
	return (mach[ms.pos]->counter == counter ? true : false);
	}


void MachineScreenPrintSingle(void) {
	short	counter;
	char	*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	s = FormGetFirst(MachineFORM_Single);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		IUIDate2Str(mach[ms.pos]->date, date);
		PrinterWrite(FormGetNext(), MACH_NAME_LEN, mach[ms.pos]->name, date);
		counter = mach[ms.pos]->counter;
		PrinterWrite(FormGetNext(), OPER_NAME_LEN, OperatorGetName(atoi(q_opNumber(counter, QRUN))));
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext(), MACH_CNTR_LEN, mach[ms.pos]->counter, q_status(counter));
		PrinterWrite(FormGetNext(), q_jobNumber(counter, QRUN), q_opNumber(counter, QRUN));
		PrinterWrite(FormGetNext(), q_dateStart(counter, QRUN), q_timeStart(counter, QRUN), q_shftNmbr(counter, QRUN));
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext(), q_pcsRqrd(counter), q_outDelay(counter));
		PrinterWrite(FormGetNext(), q_preset(counter), q_outDuration(counter));
		PrinterWrite(FormGetNext(), q_pieces(counter), q_outType(counter));
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext(), q_runTime(counter, QRUN), q_batchCnt(counter, QRUN), q_totalCount(counter, QRUN));
		PrinterWrite(FormGetNext(), q_idleTime(counter, QRUN), q_downCnt(counter), q_setupCount(counter, QRUN));
		PrinterWrite(FormGetNext(), q_setupTime(counter, QRUN), q_preset(counter), q_currentRate(counter));
		PrinterWrite(FormGetNext(), q_totalTime(counter, QRUN), q_averageRate(counter, QRUN));
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext(), q_pcsTilDone(counter));
		PrinterWrite(FormGetNext(), q_idle(counter), q_noIdleTime(counter));
		PrinterWrite(FormGetNext(), q_percCompleted(counter), q_withIdleTime(counter));
		PrinterWrite("\f");
		PrinterClose();
		}
	}


void printShiftAllMchScreen(void) {
	short	i, counter;
	char	*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	s = FormGetFirst(ShiftFORM_AllMachine);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		for (i=0; i<mchMax; i++) {
			counter = mach[i]->counter;
			PrinterWrite("%02d       ", counter);
			PrinterWrite("%s  ", q_jobNumber(counter, QSHIFT));
			PrinterWrite("%s  ", q_status(counter));
			PrinterWrite("%s  ", q_totalCount(counter, QSHIFT));
			PrinterWrite("%s     ", q_averageRate(counter, QSHIFT));
			PrinterWrite("%s    ", q_runTime(counter, QSHIFT));
			PrinterWrite("%s\n", q_idleTime(counter, QSHIFT));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	}


void printShiftSingleMchScreen(void) {
	short	counter;
	char	*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	s = FormGetFirst(ShiftFORM_SingleMachine);
	if (s && PrinterOpen()) {
		counter = mach[ms.pos]->counter;
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		IUIDate2Str(mach[ms.pos]->date, date);
		PrinterWrite(FormGetNext(), MACH_NAME_LEN, mach[ms.pos]->name, date);
		PrinterWrite(FormGetNext(), OPER_NAME_LEN, OperatorGetName(atoi(q_opNumber(counter, QRUN))));
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext(), MACH_CNTR_LEN, counter, q_status(counter));
		PrinterWrite(FormGetNext(), q_jobNumber(counter, QSHIFT), q_opNumber(counter, QSHIFT));
		PrinterWrite(FormGetNext(), q_dateStart(counter, QSHIFT), q_timeStart(counter, QSHIFT), q_shftNmbr(counter, QSHIFT));
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext(), q_runTime(counter, QSHIFT), q_totalCount(counter, QSHIFT));
		PrinterWrite(FormGetNext(), q_idleTime(counter, QSHIFT), q_batchCnt(counter, QSHIFT));
		PrinterWrite(FormGetNext(), q_setupTime(counter, QSHIFT), q_setupCount(counter, QSHIFT));
		PrinterWrite(FormGetNext(), q_totalTime(counter, QSHIFT), q_averageRate(counter, QSHIFT));
		PrinterWrite("\f");
		PrinterClose();
		}
	}









// *******************
// * Operator Module *
// *******************

#define OPER_NUM_LEN	4

#define OPER_STEP_SIZE	10
#define OPER_NONE	0x7FFF
#define OPER_MAX	10000

#define O_EXT		".OPR"

struct OperStruct {
	short	num;
	char	name[OPER_NAME_LEN+1];
	Word	date;
	};

static short		operMax=0;
static char		operName[MAXPATH], *operNone;
static short		operSize=OPER_STEP_SIZE;
static OperStruct	**oper=NULL;


static void	OperatorAdjustMenu(void);
static int	OperatorCompare(const void *elem1, const void *elem2);
static void	OperatorDelete(short oPos);
static short	OperatorFind(short operNumber);
static void	OperatorInstall(void);
static void	OperatorList(void);
static void	OperatorListDisplay(short vsnum, short pos, short num, short attr);
static Boolean	OperatorListExpand(void);
static void	OperatorLoad(void);
static void	OperatorSave(void);


static void OperatorAdjustMenu(void) {
	if (host)
		MenuItemEnable(OperatorMENU, OperatorMENU_Install);
	else
		MenuItemDisable(OperatorMENU, OperatorMENU_Install);
	if (operMax > 0)
		MenuItemEnable(OperatorMENU, OperatorMENU_List);
	else
		MenuItemDisable(OperatorMENU, OperatorMENU_List);
	}


void OperatorCleanUp(void) {
	short	i;

	if (operNone != NULL)
		r_free(operNone);
	for (i=0; i<operMax; i++)
		if (oper[i] != NULL)
			r_free(oper[i]);
	r_free(oper);
	}


static int OperatorCompare(const void *elem1, const void *elem2) {
	OperStruct **o1, **o2;

	if ((o1=(OperStruct **)elem1) == NULL)
		return (1);
	if ((o2=(OperStruct **)elem2) == NULL)
		return (-1);
	return ((**o1).num - (**o2).num);
	}


static void OperatorDelete(short oPos) {
	r_free(oper[oPos]);
	while (oPos < operMax) {
		oper[oPos] = oper[oPos+1];
		oPos++;
		}
	operMax--;
	}


void OperatorDoMenu(short item) {
	switch (item) {
		case OperatorMENU_Install:
			OperatorInstall();
			break;
		case OperatorMENU_List:
			OperatorList();
			break;
		}
	}


static short OperatorFind(short operNumber) {
	short	i;

	for (i=0; i<operMax; i++)
		if (oper[i]->num == operNumber)
			return (i);
	return (OPER_NONE);
	}


static char *OperatorGetName(short operNumber) {
	short	oPos;

	oPos = OperatorFind(operNumber);
	if (oPos == OPER_NONE)
		return (operNone);
	else
		return (oper[oPos]->name);
	}


void OperatorInit(void) {
	OperatorLoad();
	OperatorAdjustMenu();
	}


static void OperatorInstall(void) {
	char		ans[OPER_NUM_LEN+1], name[OPER_NAME_LEN+1], date[IU_DATE_LEN+1], *pval;
	short		vsnum, attr, oPos, operNumber;
	Rect		r;
	DialogStruct	d1, d2;
	Boolean		done, dirty, done1;
	int		ch;
	Dialog		dlg;

	HelpPush(OperatorHELP_Install, MenuItemGet(OperatorMENU, OperatorMENU_Install));
	vsnum = VirtualGetNew(OperatorWINDOW_Install, &r);
	attr = ColorGet(DialogCOLOR, _COLOR_Low);
	VirtualWrite(vsnum, 2, 34, GIANT, IUGetDatePrompt(), attr);
	VirtualWrite(vsnum, 4, 1, r.Width(), StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attr);
	d1.len = OPER_NAME_LEN;
	d1.top = 1;
	d1.lef = 16;
	d1.prev = &d2;
	d1.next = &d2;
	d2.len = IU_DATE_LEN;
	d2.top = 2;
	d2.lef = 17;
	d2.prev = &d1;
	d2.next = NULL;
	oPos = 0;
	done = dirty = false;
	while (!done) {
		if (oPos < operMax)
			sprintf(ans, "%d", oper[oPos]->num);
		else
			*ans = '\0';
		ch = DialogStringEditor(vsnum, 0, 18, ans, OPER_NUM_LEN);
		for (pval=ans; *pval; pval++)
			if (!isdigit(*pval))
		ch = 0;
		operNumber = atoi(ans);
		sprintf(ans, "%d", operNumber);
		VirtualWrite(vsnum, 0, 18, GIANT, ans, attr);
		switch (ch) {
			case K_ESC:
				if (dirty && (operMax > 1))
					qsort(oper, operMax, sizeof(OperStruct *), OperatorCompare);
				done = true;
				break;
			case K_UP:
			case K_PGUP:
				if (oPos > 0)
					oPos--;
				break;
			case K_DOWN:
			case K_PGDN:
				if (oPos < operMax)
					oPos++;
				break;
			case K_RETURN:
				oPos = OperatorFind(operNumber);
				if (oPos == OPER_NONE) {
					sprintf(name, "");
					IUIDate2Str(SysDate(), date);
					d1.ans = name;
					d2.ans = date;
					dlg = (DialogEditor(vsnum, &d1) == K_ESC) ? D_Cancel : D_Ok;
					if (strlen(trim(name)) == 0)
						dlg = D_Delete;
					}
				else {
					strcpy(name, oper[oPos]->name);
					IUIDate2Str(oper[oPos]->date, date);
					d1.ans = name;
					d2.ans = date;
					DialogShowPrompts(vsnum, &d1);
					dlg = D_Cancel;
					}
				do {
					done1 = true;
					dlg = DialogOkModifyDeleteCancel(vsnum, 4, 1, dlg);
					switch (dlg) {
						case D_Ok:
							if (OperatorListExpand()) {
								if (oPos == OPER_NONE) {
									oPos = operMax;
									oper[oPos] = (OperStruct *) r_malloc(sizeof(OperStruct));
									operMax++;
									}
								oper[oPos]->num = operNumber;
								strcpy(oper[oPos]->name, trim(name));
								IUStr2IDate(date, &(oper[oPos]->date));
								oPos++;
								dirty = true;
								}
							break;
						case D_Modify:
							dlg = (DialogEditor(vsnum, &d1)==K_ESC) ? D_Cancel : D_Ok;
							done1 = false;
							break;
						case D_Delete:
							if (oPos != OPER_NONE) {
								OperatorDelete(oPos);
								oPos = 0;
								dirty = true;
								}
							break;
						case D_Cancel:
							if (oPos < operMax)
								oPos++;
							break;
						}
					} while (!done1);
				VirtualWrite(vsnum, 4, 1, 50, StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attr);
				DialogClearPrompts(vsnum, &d1);
				break;
			}
		}
	VirtualDispose(vsnum);
	HelpPop();
	if (dirty) {
		OperatorSave();
		OperatorAdjustMenu();
		}
	}


static void OperatorList(void) {
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];
	short		vsnum, attrLow, attrHigh, oPos, newPos, oNum, i;
	Rect		r;
	Boolean		done;
	EventRecord	myEvent;

	HelpPush(OperatorHELP_List, MenuItemGet(OperatorMENU, OperatorMENU_List));
	vsnum = VirtualGetNew(OperatorWINDOW_List, &r);
	attrLow = ColorGet(WindowCOLOR, _COLOR_Low);
	attrHigh = ColorGet(WindowCOLOR, _COLOR_High);
	oPos = OPER_NONE;
	newPos = 0;
	VirtualWrite(vsnum, r.Height()-1, 1, GIANT, StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attrLow);
	if (operMax > 10) {
		oNum = 10;
		VirtualWrite(vsnum, r.Height()-1, 29, GIANT, StringGet(GlobalSTRING, GlobalSTRING_CursorKeys), attrLow);
		}
	else
		oNum = operMax;
	done = false;
	while (!done) {
		if (oPos != newPos) {
			oPos = newPos;
			OperatorListDisplay(vsnum, oPos, oNum, attrHigh);
			}
		while (!EventGetNext(EVT_EVERY, &myEvent))
			;
		switch (myEvent.what) {
			case EVT_KEY:
				switch (myEvent.message) {
					case K_ESC:
						done = true;
						break;
					case K_HOME:
						newPos = 0;
						break;
					case K_UP:
						if (oPos > 0)
							newPos = oPos-1;
						break;
					case K_PGUP:
						if (oPos > oNum-1)
							newPos = oPos-oNum+1;
						else
							newPos = 0;
						break;
					case K_DOWN:
						if (oPos < operMax-oNum)
							newPos = oPos+1;
						break;
					case K_PGDN:
						if (oPos+oNum-1 < operMax-oNum)
							newPos = oPos+oNum-1;
						else
							newPos = operMax-oNum;
						break;
					case K_END:
						newPos = operMax-oNum;
						break;
					default:
						if (PrinterGetKey() == myEvent.message) {
							s = FormGetFirst(OperatorFORM_List);
							if (s && PrinterOpen()) {
								IUIDate2Str(SysDate(), date);
								IUITime2Str(SysTime(), time);
								PrinterWrite(s, date, time);
								PrinterWrite(FormGetNext());
								PrinterWrite(FormGetNext());
								for (i=0; i<operMax; i++) {
									PrinterWrite("  %*d   ", OPER_NUM_LEN, oper[i]->num);
									PrinterWrite("%-*s  ", OPER_NAME_LEN, oper[i]->name);
									IUIDate2Str(oper[i]->date, date);
									PrinterWrite("%s\n", date);
									}
								PrinterWrite("\f");
								PrinterClose();
								}
							}
						break;
					}
				break;
			}
		}
	VirtualDispose(vsnum);
	HelpPop();
	}


static void OperatorListDisplay(short vsnum, short pos, short num, short attr) {
	short	i;
	char	date[IU_DATE_LEN+1];

	for (i=0; i<num; i++) {
		VirtualPrint(vsnum, 2+i, 1, GIANT, attr, "%*d", OPER_NUM_LEN, oper[pos]->num);
		VirtualWrite(vsnum, 2+i, 13, OPER_NAME_LEN, oper[pos]->name, attr);
		IUIDate2Str(oper[pos]->date, date);
		VirtualWrite(vsnum, 2+i, 46, IU_DATE_LEN, date, attr);
		pos++;
		}
	}


static Boolean OperatorListExpand(void) {
	short		oPos;
	OperStruct	**tempOper;

	if (operMax < operSize)
		return (true);
	if (operSize < OPER_MAX) {
		operSize = min(operSize+OPER_STEP_SIZE, OPER_MAX);
		tempOper = (OperStruct **) r_malloc(sizeof(OperStruct *) * operSize);
		for (oPos=0; oPos<operMax; oPos++)
			tempOper[oPos] = oper[oPos];
		r_free(oper);
		oper = tempOper;
		return (true);
		}
	Error(Err_Recover, StringGet(OperatorSTRING, OperatorSTRING_MaxCreated));
	return (false);
	}


static void OperatorLoad(void) {
	short	handle, i;
	char	sig[sizeof(PCS100_SIG)], *s;

	StatusPush(OperatorSTATUS, OperatorSTATUS_Loading);
	sprintf(operName, "%s%s%s", dataPath, progName, O_EXT);
	if ((handle = open(operName, O_RDONLY | O_BINARY)) != -1) {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &operMax, sizeof(operMax));
			if (operMax > 0) {
				operSize = operMax;
				oper = (OperStruct **) r_malloc(sizeof(OperStruct *) * operSize);
				for (i=0; i<operMax; i++) {
					oper[i] = (OperStruct *) r_malloc(sizeof(OperStruct));
					read(handle, oper[i], sizeof(OperStruct));
					}
				}
			}
		else
			MenuErrorPost(ErrorGet(OperatorERROR, OperatorERROR_Load), true);
		close(handle);
		}
	if (oper == NULL)
		oper = (OperStruct **) r_malloc(sizeof(OperStruct *) * operSize);
	s = StringGet(OperatorSTRING, OperatorSTRING_Unnamed);
	operNone = (char *) r_malloc(strlen(s)+1);
	strcpy(operNone, s);
	StatusPop();
	}


static void OperatorSave(void) {
	short	handle, i;

	if (!host)
		return;
	StatusPush(OperatorSTATUS, OperatorSTATUS_Saving);
	if ((handle = open(operName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) != -1) {
		write(handle, PCS100_SIG, sizeof(PCS100_SIG));
		write(handle, &operMax, sizeof(operMax));
		for (i=0; i<operMax; i++)
			write(handle, oper[i], sizeof(OperStruct));
		close(handle);
		}
	else
		MenuErrorPost(ErrorGet(OperatorERROR, OperatorERROR_Save), true);
	StatusPop();
	}


void OperatorSubscribe(void) {
	short		handle, i, num, pos, j;
	struct stat	stat_buf;
	char		sig[sizeof(PCS100_SIG)];
	OperStruct	new_oper;
	DWord		*unused;
	static long	pubSubTime=0;

	if ((handle = open(operName, O_RDONLY | O_BINARY)) == -1)
		return;
	fstat(handle, &stat_buf);
	if (pubSubTime != stat_buf.st_ctime) {
		pubSubTime = stat_buf.st_ctime;
		StatusPush(OperatorSTATUS, OperatorSTATUS_Loading);
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &num, sizeof(num));
			unused = (DWord *) r_malloc(sizeof(DWord)*num);
			for (i=0; i<num; i++) {
				read(handle, &new_oper, sizeof(new_oper));
				unused[i] = new_oper.num;
				}
			for (i=operMax-1; i >= 0; i--) {
				for (j=0; j<num; j++)
					if (oper[i]->num == unused[j])
						break;
				if (j == num)
					OperatorDelete(i);
				}
			r_free(unused);
			lseek(handle, 0L, SEEK_SET);
			read(handle, sig, sizeof(PCS100_SIG));
			read(handle, &num, sizeof(num));
			for (i=0; i<num; i++) {
				read(handle, &new_oper, sizeof(new_oper));
				if ((pos=OperatorFind(new_oper.num)) == OPER_NONE && OperatorListExpand()) {
					pos = operMax++;
					oper[pos] = (OperStruct *) r_malloc(sizeof(OperStruct));
					}
				memmove(oper[pos], &new_oper, sizeof(new_oper));
				}
			if (operMax > 1)
				qsort(oper, operMax, sizeof(OperStruct *), OperatorCompare);
			OperatorAdjustMenu();
			}
		StatusPop();
		}
	close(handle);
	}
