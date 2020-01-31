// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

// PCS100 - PCS100 Network Supervisor Application
//
// Syntax:	PCS100 [/s] [/?] [message...]
//
// Description:	PCS100 Network Supervisor Application
//
// Status:	PCS100 may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//
// Options:	/?	Display help screen.
//		/s	Subscribe mode.
//
// History:	jn	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 4.0á4  9201.28  jn      Make work with new compiler and libraries.
// 4.0á2  9105.20  jn      Find program boot-up bug that causes lock-up.
// 4.0á1  9105.18  jn      Networking (LAN) capabilities.

#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <iostream.h>
#include <process.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\coord.h>
#include <common\keyboard.h>
#include <common\mouse.h>
#include <common\system.h>
#include <common\timer.h>
#include <common\ttool.h>

#include <seashell\clock.h>
#include <seashell\color.h>
#include <seashell\dialog.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\iu.h>
#include <seashell\list.h>
#include <seashell\menu.h>
#include <seashell\printer.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\status.h>
#include <seashell\string.h>
#include <seashell\version.h>
#include <seashell\window.h>

#include "inc\pcs100.rh"
#include "inc\pcs100.h"
#include "inc\phelp.h"
#include "inc\screen.h"
#include "inc\mach.h"
#include "inc\job.h"
#include "inc\shift.h"
#include "inc\graph.h"
#include "inc\query.h"
#include "inc\archiv.h"
#include "inc\scomm.h"
#include "inc\report.h"

#define PROGRAM_NAME	"PCS100"
#define CFG_EXT		".CFG"
#define HOST_EXT	".$$$"

class TAppTool : public TTool {
protected:
	Boolean	DoOption(char option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	TAppTool(const char *name, Boolean disable_break=true);
	~TAppTool();
	};

DataStruct	data;
Boolean		doneFlag=false, restart=false, host=true;
C_Timer		nextAutoSave;

static char		datName[MAXPATH], hostName[MAXPATH];
static DataStruct	origData;
static C_Timer		nextPubSub;
static short		hostHandle;
static C_Event		theEvent;
static TAppTool		app(PROGRAM_NAME);

static const char *help_text[] = {
		"\nPCS100 Network Supervisor.\n\n",
		"Syntax:  PCS100 [/?] [/s]\n\n",
		"Status:  PCS100 may return the following status codes:\n",
		"          0  No errors.\n",
		"          1  Syntax error.\n\n",
		"Options: /?  Displays this help text.\n",
		"         /s  Subscribe mode.\n"
		};

static void	AdjustMenus(void);
static void	CleanUp(void);
static short	DataCheckSum(void);
static void	DataLoad(void);
static void	DataSave(void);
static void	DataSubscribe(void);
static void	DoAutoSave(void);
static void	DoCommand(short id, short item);
static void	DoMainLoop(void);
static void	HostLogin(void);
static void	HostLogout(void);
static void	InitModules(void);
static void	InstallMenus(void);
static void	ShowCopyright(void);
static void	SystemTask(void);

main() {
	ResVersion	*r_ver;

	if (host)
		HostLogin();
	EquipInit();
	DataLoad();
	ResInit("%s%s%s", app.ProgPath(), app.ProgName(), data.rExt);
	r_ver = (ResVersion *) VersionGet(PCS100_VERSION);
	app.Copyright(r_ver->ver_str, "1987-1992");
	return (app.Run());
	}


TAppTool::TAppTool(const char *name, Boolean disable_break) : TTool(name, disable_break) {
	fOptList = "sS";
	fHelpNum = NUMELE(help_text);
	fHelpText = help_text;
	}


TAppTool::~TAppTool() {
	CleanUp();		// make sure ctrl-break handler is restored
	if (restart)
		execv(fProgName, _argv);
	}


Boolean TAppTool::DoOption(char option, const char * /*argument*/) {
	switch (option) {
		case 's':
		case 'S':
			host = false;
			return (true);
		}
	return (false);
	}


int TAppTool::DoWork(short /*argc*/, const char ** /*argv*/) {
	VirtualInit();
	ErrorInit(::CleanUp);
	theEvent.SetTask(SystemTask);
	StatusInit("%s%s", fDataPath, fProgName);
	ClockInit();
	PrinterInit(kK_CTRLP);
	HelpInit(kK_HELP);
	MenuInit(DoCommand);
	MenuBarPush(PCS100_BAR);
	ListInit();
	DialogInit(fDataPath);
	IUInit();
	atexit(::CleanUp);
	InitModules();
	InstallMenus();
	ShowCopyright();
	DoMainLoop();
	HelpPop();
	return (kExitOK);
	}


static void AdjustMenus(void) {
	ScreenAdjustMenu();
	MachineAdjustMenu();
	JobAdjustMenu();
	ShiftAdjustMenu();
	GraphAdjustMenu();
	}


static void CleanUp(void) {
	JobCleanUp();
	OperatorCleanUp();
	MachineCleanUp();
	DataSave();
	HostLogout();
	}


static short DataCheckSum(void) {
	register short	len, csum;
	register char	*buff;

	len = offsetof(DataStruct, cSum);
	csum = 0;
	buff = (char *) &data;
	while (len--)
		csum += *buff++;
	return (csum);
	}


static void DataLoad(void) {
	Boolean	loadError=true;
	short	handle, size;
	char	sig[sizeof(PCS100_SIG)];

	sprintf(datName, "%s%s%s", app.DataPath(), app.ProgName(), CFG_EXT);
	loadError = true;
	if ((handle = open(datName, O_RDONLY | O_BINARY)) != -1) {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			size = read(handle,  &data, sizeof(data));
			loadError = (size==sizeof(data) && DataCheckSum()==data.cSum) ? false : true;
			}
		close(handle);
		}
	if (loadError) {
		strcpy(data.rExt, RES_OUT_EXT);	// use default resource
		PrefsInitData();
		QueryInitData();
		CommInitData();
		}
	memmove(&origData, &data, sizeof(DataStruct));
	nextAutoSave.Reset(data.prefs.autoSave);
	nextPubSub.Reset(data.prefs.netPubSub);
	}


static void DataSave(void) {
	short	handle;

	if (host && memcmp(&origData, &data, sizeof(DataStruct)) != 0) {
		StatusPush(PCS100_STATUS, PCS100_STATUS_Save);
		if ((handle = open(datName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) != -1) {
			data.cSum = DataCheckSum();
			write(handle, PCS100_SIG, sizeof(PCS100_SIG));
			memmove(&origData, &data, sizeof(data));
			write(handle, &origData, sizeof(origData));
			close(handle);
			}
		else
			MenuErrorPost(ErrorGet(PCS100_ERROR, PCS100_ERROR_NoSave), true);
		StatusPop();
		}
	}


static void DataSubscribe(void) {
	short		handle, size;
	char		sig[sizeof(PCS100_SIG)];
	DataStruct	new_data;
	struct stat	stat_buf;
	static long	pubSubTime=0;

	if ((handle = open(datName, O_RDONLY | O_BINARY)) == -1)
		return;
	fstat(handle, &stat_buf);
	if (pubSubTime != stat_buf.st_ctime) {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			size = read(handle,  &new_data, sizeof(new_data));
			if (size==sizeof(new_data) && DataCheckSum()==new_data.cSum)
				memmove(&data, &new_data, sizeof(DataStruct));
			}
		}
	close(handle);
	}


static void DoAutoSave(void) {
	ArchiveSave();
	QueryDataSave();
	DataSave();
	flushall();
	nextAutoSave.Reset(data.prefs.autoSave);
	}


static void DoCommand(short id, short item) {
	switch (id) {
		case HelpMENU:
			HelpDo(item);
			break;
		case LanguageMENU:
			LanguageDo(item);
			break;
		case PrefsMENU:
			PrefsDo(item);
			break;
		case MachineMENU:
			MachineDoMenu(item);
			break;
		case OperatorMENU:
			OperatorDoMenu(item);
			break;
		case JobMENU:
			JobDoMenu(item);
			break;
		case ShiftMENU:
			ShiftDoMenu(item);
			break;
		case GraphMENU:
			GraphDoMenu(item);
			break;
		case ReportMENU:
			DoReportMenu(item);
			break;
		case GenerateMENU:
			GenerateDo(item);
			break;
		case PortMENU:
			CommPortDo(item);
			break;
		case SetupMENU:
			CommSetupDo(item);
			break;
		case BaudMENU:
			CommBaudDo(item);
			break;
		case ParityMENU:
			CommParityDo(item);
			break;
		case DataMENU:
			CommDataDo(item);
			break;
		case StopMENU:
			CommStopDo(item);
			break;
		case QueryMENU:
			QueryDoMenu(item);
			break;
		}
	}


static void DoMainLoop(void) {
	short	ch;

	while (!doneFlag) {
		if (theEvent.GetNext(EVT_EVERY))
			switch (theEvent.What()) {
				case EVT_KEY:
					ch = theEvent.Message();
					if (MenuIsAlt(ch)) {
						AdjustMenus();
						MenuSelect(ch);
						}
					else if (MenuIsCtrl(ch)) {
						AdjustMenus();
						MenuKey(ch);
						}
					else if ((ch>='A' && ch<='Z') || (ch>='a' && ch<='z')) {
						AdjustMenus();
						MenuSelect(toupper(ch));
						}
					else
						ScreenDo(ch);
					break;
				}
		}
	}


static void DoPubSub(void) {
	if (host) {
		QueryDataSave();
		nextPubSub.Reset(data.prefs.netPubSub);
		}
	else {
		DataSubscribe();
		MachineSubscribe();
		OperatorSubscribe();
		JobSubscribe();
		if (QuerySubscribe())
			nextPubSub.Reset(data.prefs.netPubSub);
		else
			nextPubSub.Reset(data.prefs.netSubRetry);
		}
	}


// HostLogin attempts to create the HOST data file.  If someone else has
// already opened this file and is currently running as a host, we return
// false.  Otherwise we create/open the file and return true.

static void HostLogin(void) {
	sprintf(hostName, "%s%s%s", app.DataPath(), app.ProgName(), HOST_EXT);
	if ((hostHandle = sopen(hostName, O_WRONLY | O_CREAT, SH_DENYRW, S_IREAD | S_IWRITE)) == -1)
		host = false;
	}


// HostLogout verifies that we are the host.  If not, it exists.  Otherwise
// the host file is closed and deleted.

static void HostLogout(void) {
	if (!host)
		return;
	close(hostHandle);
	remove(hostName);
	}


static void InitModules(void) {
	HelpInit();
	LanguageInit();
	PrefsInit();
	MachineInit();
	OperatorInit();
	JobInit();
	GraphInit();
	QueryInit();
	ArchiveInit();
	CommInit();
	ScreenInit();
	}


static void InstallMenus(void) {
	HelpSetKey(MenuItemGetEquiv(HelpMENU, HelpMENU_Help));
	PrinterSetKey(MenuItemGetEquiv(ReportMENU, ReportMENU_Print));
	StatusSet(PCS100_STATUS, PCS100_STATUS_MenuAccess);
	}


static void ShowCopyright(void) {
	short	vsnum;
	Rect	r;
	C_Timer	time(3000L);

	vsnum = VirtualGetNew(CopyrightWINDOW, &r);
	while (!time.Elapsed() && !theEvent.Avail(EVT_EVERY))
		;
	VirtualDispose(vsnum);
	}


static void SystemTask(void) {
	if (getQueryState())
		updateQuery();
	if (clockChanged) {
		StatusPrint(SCREEN_WIDTH-8, clockStr);
		clockChanged = false;
		}
	if (nextAutoSave.Elapsed())
		DoAutoSave();
	if (nextPubSub.Elapsed())
		DoPubSub();
	ScreenSynch();
	}
