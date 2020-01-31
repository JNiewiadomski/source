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
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\coord.h>
#include <common\disk.h>
#include <common\getargs.h>
#include <common\general.h>
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
#include <seashell\list.h>
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
#include "job.h"
#include "query.h"
#include "archiv.h"


#define JOB_EXT		".JOB"
#define JOB_MAX		32760
#define JOB_STEP_SIZE	10
#define JOB_NONE	0x7FFF

#define JOB_SINGLE_MAX	13	/* max num of machines on single job screen */

typedef struct {
	long	totalCnt;	/* total count */
	long	curRate;	/* current rate (pieces/hour) */
	float	avRate;		/* average rate (pieces/hour) */
	long	rt;		/* run time (hours) */
	long	rtit;		/* run time + idle time (hours) */
	} STATS;

short		jobMax=0;
JobStruct	**job=NULL;

static struct {
	short	pos;		/* current position in item list */
	short	nextPos;	/* next active position in item list */
	short	loc;		/* location relative to top */
	short	max;		/* maximum number of items in list */
	short	num;		/* maximum number of items that fit on screen */
	short	mchPos;		/* offset into machine list */
	} js;

static short	jobSize=JOB_STEP_SIZE;
static char	jobName[MAXPATH];


static void	initJobScreenInfo(void);
static void	loadJobs(void);


static void	adjustJobPtrs(long jobNumber);
static void	deleteJob(short jPos, long oldJob);
static void	displayAllJobsScreen(QTYPE type);
static void	displayListJobs(short vsnum, short pos, short num, short attr);
static Boolean	doSaveArchivalJob(char *name, short jPos);
static Boolean	expandJobList(void);
static short	findJob(long jobNumber);
static short	getJobStatistics(long jobNumber, STATS *st);
static short	getShiftStatistics(long jobNumber, STATS *st);
static float	j_percComplete(long totalCount, long jobPreset);
static float	j_noIdleTime(long jobPreset, long totalPieces, long curRate);
static float	j_withIdleTime(long jobPreset, long totalPieces, float avRate);
static int	jobCompare(const void *elem1, const void *elem2);
static void	redisplayAllJobsScreen(Boolean update, QTYPE type);
static void	redisplaySingleJobScreen(QTYPE type);
static Boolean	saveArchivalJob(short jPos);
static void	saveJobs(void);


void JobAdjustMenu(void) {
	if (host)
		MenuItemEnable(JobMENU, JobMENU_Install);
	else
		MenuItemDisable(JobMENU, JobMENU_Install);
	if (jobMax > 0) {
		if (mchMax > 0) {
			MenuItemEnable(JobMENU, JobMENU_All);
			MenuItemEnable(JobMENU, JobMENU_Single);
			if (host) {
				MenuItemEnable(JobMENU, JobMENU_Close);
				MenuItemEnable(JobMENU, JobMENU_ExportOpen);
				MenuItemEnable(JobMENU, JobMENU_ExportClosed);
				}
			else {
				MenuItemDisable(JobMENU, JobMENU_Close);
				MenuItemDisable(JobMENU, JobMENU_ExportOpen);
				MenuItemDisable(JobMENU, JobMENU_ExportClosed);
				}
			}
		else {
			MenuItemDisable(JobMENU, JobMENU_All);
			MenuItemDisable(JobMENU, JobMENU_Single);
			MenuItemDisable(JobMENU, JobMENU_Close);
			MenuItemDisable(JobMENU, JobMENU_ExportOpen);
			MenuItemDisable(JobMENU, JobMENU_ExportClosed);
			}
		MenuItemEnable(JobMENU, JobMENU_List);
		}
	else {
		MenuItemDisable(JobMENU, JobMENU_All);
		MenuItemDisable(JobMENU, JobMENU_Single);
		MenuItemDisable(JobMENU, JobMENU_List);
		MenuItemDisable(JobMENU, JobMENU_Close);
		MenuItemDisable(JobMENU, JobMENU_ExportOpen);
		MenuItemDisable(JobMENU, JobMENU_ExportClosed);
		}
	}


void JobCleanUp(void) {
	short	i;

	for (i=0; i<jobMax; i++)
		if (job[i] != NULL)
			r_free(job[i]);
	r_free(job);
	}


void JobDoMenu(short item) {
	switch (item) {
		case JobMENU_All:
			monitorAllJobs();
			break;
		case JobMENU_Single:
			monitorSingleJob();
			break;
		case JobMENU_Install:
			installJobs();
			break;
		case JobMENU_List:
			listJobs();
			break;
		case JobMENU_Close:
			closeJob();
			break;
		case JobMENU_ExportOpen:
			exportOpenJobs();
			break;
		case JobMENU_ExportClosed:
			exportClosedJobs();
			break;
		}
	}


void JobInit(void) {
	loadJobs();
	initJobScreenInfo();
	}


void JobSubscribe(void) {
	short		handle, i, num, pos, j;
	struct stat	stat_buf;
	char		sig[sizeof(PCS100_SIG)];
	JobStruct	new_job;
	DWord		*unused;
	long		cur=JOB_NONE;
	static long	pubSubTime=0;

	if ((handle = open(jobName, O_RDONLY | O_BINARY)) == -1)
		return;
	fstat(handle, &stat_buf);
	if (pubSubTime != stat_buf.st_ctime) {
		pubSubTime = stat_buf.st_ctime;
		StatusPush(JobSTATUS, JobSTATUS_Loading);
		if (scrn.cur != SCREEN_NONE)
			cur = job[js.pos]->num;
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &num, sizeof(num));
			unused = (DWord *) r_malloc(sizeof(DWord)*num);
			for (i=0; i<num; i++) {
				read(handle, &new_job, sizeof(new_job));
				unused[i] = new_job.num;
				}
			for (i=jobMax-1; i >= 0; i--) {
				for (j=0; j<num; j++)
					if (job[i]->num == unused[j])
						break;
				if (j == num)
					deleteJob(i, cur);
				}
			r_free(unused);
			lseek(handle, 0L, SEEK_SET);
			read(handle, sig, sizeof(PCS100_SIG));
			read(handle, &num, sizeof(num));
			for (i=0; i<num; i++) {
				read(handle, &new_job, sizeof(new_job));
				if ((pos=findJob(new_job.num)) == JOB_NONE && expandJobList()) {
					pos = jobMax++;
					job[pos] = (JobStruct *) r_malloc(sizeof(JobStruct));
					adjustJobPtrs(cur);
					}
				memmove(job[pos], &new_job, sizeof(new_job));
				}
			if (jobMax > 1) {
				qsort(job, jobMax, sizeof(JobStruct *), jobCompare);
				adjustJobPtrs(cur);
				}
			}
		StatusPop();
		}
	close(handle);
	}



void addJob(long jobNumber) {
	short	jPos;
	long	oldJob;

	if (!expandJobList())
		return;
	jPos = 0;
	while ((jPos<jobMax) && (job[jPos]->num != jobNumber))
		jPos++;
	if (jPos >= jobMax) {
		oldJob = job[js.pos]->num;
		job[jobMax] = (JobStruct *) r_malloc(sizeof(JobStruct));
		job[jobMax]->num = jobNumber;
		strcpy(job[jobMax]->name, StringGet(JobSTRING, JobSTRING_Unnamed));
		job[jobMax]->total = 0;
		job[jobMax]->date = SysDate();
		jobMax++;
		qsort(job, jobMax, sizeof(JobStruct *), jobCompare);
		adjustJobPtrs(oldJob);
		saveJobs();
		}
	}


static void adjustJobPtrs(long jobNumber) {
	short	jPos;

	if (jobMax > 0) {
		if (mchMax > 0) {
			jPos = jobMax-1;
			while ((jPos > 0) && (job[jPos]->num != jobNumber))
				jPos--;
			js.nextPos = jPos;
			js.num = (jobMax>js.max) ? js.max : jobMax;
			if (jPos < js.loc)
				js.loc = jPos;
			else if (jPos > js.loc+js.num-1)
				js.loc = jPos-js.num+1;
			else if (js.loc+js.num > jobMax)
				js.loc = max(0, jobMax-js.num);
			switch (scrn.cur) {
				case SCREEN_ALL_JOB:
					redisplayAllJobsScreen(true, QRUN);
					break;
				case SCREEN_SNGL_JOB:
					redisplaySingleJobScreen(QRUN);
					break;
				case SCREEN_ALL_SJOB:
					redisplayAllJobsScreen(true, QSHIFT);
					break;
				case SCREEN_SNGL_SJOB:
					redisplaySingleJobScreen(QSHIFT);
					break;
				}
			}
		}
	else if (scrn.cur==SCREEN_ALL_JOB || scrn.cur==SCREEN_SNGL_JOB || scrn.cur==SCREEN_SNGL_SJOB || scrn.cur==SCREEN_ALL_SJOB) {
		ScreenMonitorNothing();
		initJobScreenInfo();
		}
	}


static void deleteJob(short jPos, long oldJob) {
	r_free(job[jPos]);
	while (jPos < jobMax) {
		job[jPos] = job[jPos+1];
		jPos++;
		}
	jobMax--;
	adjustJobPtrs(oldJob);
	}


static void displayListJobs(short vsnum, short pos, short num, short attr) {
	short		i, j;
	char		date[IU_DATE_LEN+1];
	Boolean		found;
	QueryJobStruct	*qj;

	for (i=0; i<num; i++) {
		VirtualPrint(vsnum, 2+i, 1, GIANT, attr, "%*ld", JOB_NUM_LEN, job[pos]->num);
		VirtualWrite(vsnum, 2+i, 11, JOB_NAME_LEN, job[pos]->name, attr);
		VirtualPrint(vsnum, 2+i, 43, GIANT, attr, "%*ld", JOB_TOT_LEN, job[pos]->total);
		IUIDate2Str(job[pos]->date, date);
		VirtualWrite(vsnum, 2+i, 53, IU_DATE_LEN, date, attr);
		for (j=0, found=false; j<MAX_MACHINES && !found; j++) {
			qj = getJobQuery(j);
			if (qj != NULL && qj->jobN == job[pos]->num)
				found = true;
			}
		VirtualWrite(vsnum, 2+i, 65, 6, found ? StringGet(JobSTRING, JobSTRING_Active) : "", attr);
		pos++;
		}
	}


static Boolean expandJobList(void) {
	short		jPos;
	JobStruct	**tempJob;

	if (jobMax < jobSize)
		return (true);
	if (jobSize < JOB_MAX) {
		jobSize = min(jobSize+JOB_STEP_SIZE, JOB_MAX);
		tempJob = (JobStruct **) r_malloc(sizeof(JobStruct *) * jobSize);
		for (jPos=0; jPos<jobMax; jPos++)
			tempJob[jPos] = job[jPos];
		r_free(job);
		job = tempJob;
		return (true);
		}
	Error(Err_Recover, ErrorGet(JobERROR, JobERROR_MaxCreated));
	return (false);
	}


static short findJob(long jobNumber) {
	short	i;

	for (i=0; i<jobMax; i++)
		if (job[i]->num == jobNumber)
			return (i);
	return (JOB_NONE);
	}


Boolean isAllJobsDisplayed(long jobNumber) {
	short	i;

	for (i=js.loc; i<js.loc+js.num; i++)
		if (job[i]->num == jobNumber)
			return (true);
	return (false);
	}


Boolean isSingleJobDisplayed(long jobNumber) {
	return (job[js.pos]->num == jobNumber ? true : false);
	}


static short getJobStatistics(long jobNumber, STATS *st) {
	short		jobMchCnt, i, counter;
	QueryJobStruct	*qPtr;
	long		ritime;
	float		thrs;

	st->totalCnt = st->curRate = st->rt = st->rtit = 0L;
	st->avRate = 0.0;
	jobMchCnt = 0;
	i = 0;
	while (i < mchMax) {
		counter = mach[i]->counter;
		qPtr = getJobQuery(counter);
		if (qPtr->jobN == jobNumber) {
			jobMchCnt++;
			st->totalCnt += qPtr->totalCnt;
			st->curRate += qPtr->cRate;
			st->rt = IULTimeAdd(st->rt, qPtr->prodTime);
			ritime = IULTimeAdd(qPtr->prodTime, qPtr->idleTime);
			st->rtit = IULTimeAdd(st->rtit, ritime);
			thrs = IULTime2Hrs(ritime);
			if (thrs > 0.0 && !isOffline(counter))
				st->avRate += (float)qPtr->totalCnt / thrs;
			}
		i++;
		}
	return (jobMchCnt);
	}


static short getShiftStatistics(long jobNumber, STATS *st) {
	short			shiftMchCnt, i, counter;
	QueryShiftStruct	*qPtr;
	long			ritime;
	float			thrs;

	st->totalCnt = st->curRate = st->rt = st->rtit = 0L;
	st->avRate = 0.0;
	shiftMchCnt = 0;
	i = 0;
	while (i < mchMax) {
		counter = mach[i]->counter;
		qPtr = getShiftQuery(counter);
		if (qPtr->jobN == jobNumber) {
			shiftMchCnt++;
			st->totalCnt += qPtr->shfTotal;
			st->curRate += qPtr->sCurRate;
			st->rt = IULTimeAdd(st->rt, qPtr->sPrdTime);
			ritime = IULTimeAdd(qPtr->sPrdTime, qPtr->sIdlTime);
			st->rtit = IULTimeAdd(st->rtit, ritime);
			thrs = IULTime2Hrs(ritime);
			if (thrs > 0.0 && !isOffline(counter))
				st->avRate += (float)qPtr->shfTotal / thrs;
			}
		i++;
		}
	return (shiftMchCnt);
	}


static void initJobScreenInfo(void) {
	js.pos = JOB_NONE;
	js.nextPos = js.loc = 0;
	js.max = 18;
	js.num = (jobMax>js.max) ? js.max : jobMax;
	js.mchPos = 0;
	}


void installJobs(void) {
	char		ans[JOB_NUM_LEN+1], name[JOB_NAME_LEN+1], total[JOB_TOT_LEN+1], date[IU_DATE_LEN+1], *pval;
	short		vsnum, attr, jPos;
	Rect		r;
	DialogStruct	d1, d2, d3;
	Boolean		done, dirty, done1;
	long		oldJob, jobNumber;
	int		ch;
	Dialog		dlg;

	if (scrn.cur != SCREEN_NONE)
		oldJob = job[js.pos]->num;
	HelpPush(JobHELP_Install, MenuItemGet(JobMENU, JobMENU_Install));
	StatusPush(JobSTATUS, JobSTATUS_Install);
	vsnum = VirtualGetNew(JobWINDOW_Install, &r);
	attr = ColorGet(DialogCOLOR, _COLOR_Low);
	VirtualWrite(vsnum, 3, 29, GIANT, IUGetDatePrompt(), attr);
	VirtualWrite(vsnum, 5, 1, r.Width(), StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attr);
	d1.len = JOB_NAME_LEN;
	d1.top = 1;
	d1.lef = 11;
	d1.prev = &d3;
	d1.next = &d2;
	d2.len = JOB_TOT_LEN;
	d2.top = 2;
	d2.lef = 12;
	d2.prev = &d1;
	d2.next = &d3;
	d3.len = IU_DATE_LEN;
	d3.top = 3;
	d3.lef = 17;
	d3.prev = &d2;
	d3.next = NULL;
	jPos = 0;
	done = dirty = false;
	while (!done) {
		if (jPos < jobMax)
			sprintf(ans, "%ld", job[jPos]->num);
		else
			*ans = '\0';
		ch = DialogStringEditor(vsnum, 0, 13, ans, JOB_NUM_LEN);
		for (pval=ans; *pval; pval++)
			if (!isdigit(*pval))
				ch = 0;
		jobNumber = atol(ans);
		sprintf(ans, "%ld", jobNumber);
		VirtualWrite(vsnum, 0, 13, GIANT, ans, attr);
		switch (ch) {
			case K_ESC:
				if (dirty && (jobMax > 1)) {
					qsort(job, jobMax, sizeof(JobStruct *), jobCompare);
					adjustJobPtrs(oldJob);
					}
				done = true;
				break;
			case K_UP:
			case K_PGUP:
				if (jPos > 0)
					jPos--;
				break;
			case K_DOWN:
			case K_PGDN:
				if (jPos < jobMax)
					jPos++;
				break;
			case K_RETURN:
				jPos = findJob(jobNumber);
				if (jPos == JOB_NONE) {
					strcpy(name, StringGet(JobSTRING, JobSTRING_Unnamed));
					sprintf(total, "%ld", 0L);
					IUIDate2Str(SysDate(), date);
					d1.ans = name;
					d2.ans = total;
					d3.ans = date;
					dlg = (DialogEditor(vsnum, &d1) == K_ESC) ? D_Cancel : D_Ok;
					if (atol(total) == 0)
						dlg = D_Delete;
					}
				else {
					strcpy(name, job[jPos]->name);
					sprintf(total, "%ld", job[jPos]->total);
					IUIDate2Str(job[jPos]->date, date);
					d1.ans = name;
					d2.ans = total;
					d3.ans = date;
					DialogShowPrompts(vsnum, &d1);
					dlg = D_Cancel;
					}
				do {
					done1 = true;
					dlg = DialogOkModifyDeleteCancel(vsnum, 5, 1, dlg);
					switch (dlg) {
						case D_Ok:
							if (expandJobList()) {
								if (jPos == JOB_NONE) {
									jPos = jobMax;
									job[jPos] = (JobStruct *) r_malloc(sizeof(JobStruct));
									jobMax++;
									}
								job[jPos]->num = jobNumber;
								strcpy(job[jPos]->name, trim(name));
								job[jPos]->total = atol(total);
								IUStr2IDate(date, &(job[jPos]->date));
								jPos++;
								adjustJobPtrs(oldJob);
								dirty = true;
								}
							break;
						case D_Modify:
							dlg = (DialogEditor(vsnum, &d1)==K_ESC) ? D_Cancel : D_Ok;
							done1 = false;
							break;
						case D_Delete:
							if (jPos != JOB_NONE) {
								deleteJob(jPos, oldJob);
								jPos = 0;
								dirty = true;
								}
							break;
						case D_Cancel:
							if (jPos < jobMax)
								jPos++;
							break;
						}
					} while (!done1);
				VirtualWrite(vsnum, 5, 1, 50, StringGet(GlobalSTRING, GlobalSTRING_EscToExit), attr);
				DialogClearPrompts(vsnum, &d1);
				break;
			}
		}
	VirtualDispose(vsnum);
	StatusPop();
	HelpPop();
	if (dirty)
		saveJobs();
	}


static float j_percComplete(long totalCount, long jobPreset) {
	float	perc;

	if (jobPreset > 0) {
		perc = (float)totalCount / (float)jobPreset * 100.0;
		if (perc > 999.9)
			perc = 999.9;
		}
	else
		perc = 0.0;
	return (perc);
	}


static float j_noIdleTime(long jobPreset, long totalPieces, long curRate) {
	float	nTime;

	if (curRate > 0) {
		nTime = ((float)jobPreset - (float)totalPieces) / (float)curRate;
		if (nTime < 0.0)
			nTime = 0.0;
		else if (nTime > 9999.999)
			nTime = 9999.999;
		}
	else
		nTime = 0.0;
	return (nTime);
	}


static float j_withIdleTime(long jobPreset, long totalPieces, float avRate) {
	float	wTime;

	if (avRate > 0.0) {
		wTime = ((float)jobPreset - (float)totalPieces) / avRate;
		if (wTime < 0.0)
			wTime = 0.0;
		else if (wTime > 9999.999)
			wTime = 9999.999;
		}
	else
		wTime = 0.0;
	return (wTime);
	}


static int jobCompare(const void *elem1, const void *elem2) {
	JobStruct	**j1, **j2;
	long		val;

	if ((j1=(JobStruct **)elem1) == NULL)
		return (1);
	if ((j2=(JobStruct **)elem2) == NULL)
		return (-1);
	val = (**j1).num - (**j2).num;
	if (val < 0)
		return (-1);
	else if (val > 0)
		return (1);
	return (0);
	}


void listJobs(void) {
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];
	short		vsnum, attr, jPos, newPos, jNum, i, j;
	Rect		r;
	Boolean		done;
	EventRecord	myEvent;
	QueryJobStruct	*qj;

	HelpPush(JobHELP_List, MenuItemGet(JobMENU, JobMENU_List));
	if (jobMax > 10) {
		jNum = 10;
		StatusPush(JobSTATUS, JobSTATUS_ListPosition);
		}
	else {
		jNum = jobMax;
		StatusPush(JobSTATUS, JobSTATUS_List);
		}
	vsnum = VirtualGetNew(JobWINDOW_List, &r);
	attr = ColorGet(WindowCOLOR, _COLOR_High);
	jPos = JOB_NONE;
	newPos = 0;
	done = false;
	while (!done) {
		if (jPos != newPos) {
			jPos = newPos;
			displayListJobs(vsnum, jPos, jNum, attr);
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
						if (jPos > 0)
							newPos = jPos-1;
						break;
					case K_PGUP:
						if (jPos > jNum-1)
							newPos = jPos-jNum+1;
						else
							newPos = 0;
						break;
					case K_DOWN:
						if (jPos < jobMax-jNum)
							newPos = jPos+1;
						break;
					case K_PGDN:
						if (jPos+jNum-1 < jobMax-jNum)
							newPos = jPos+jNum-1;
						else
							newPos = jobMax-jNum;
						break;
					case K_END:
						newPos = jobMax-jNum;
						break;
					default:
						if (PrinterGetKey() == myEvent.message) {
							s = FormGetFirst(JobFORM_List);
							if (s && PrinterOpen()) {
								IUIDate2Str(SysDate(), date);
								IUITime2Str(SysTime(), time);
								PrinterWrite(s, date, time);
								PrinterWrite(FormGetNext());
								PrinterWrite(FormGetNext());
								for (i=0; i<jobMax; i++) {
									PrinterWrite("  %*ld   ", JOB_NUM_LEN, job[i]->num);
									PrinterWrite("%-*s  ", JOB_NAME_LEN, job[i]->name);
									PrinterWrite("%*ld  ", JOB_TOT_LEN, job[i]->total);
									IUIDate2Str(job[i]->date, date);
									PrinterWrite(date);
									for (j=0; j<MAX_MACHINES; j++) {
										qj = getJobQuery(j);
										if (qj != NULL && qj->jobN == job[i]->num) {
											PrinterWrite("  %s", JobSTRING, JobSTRING_Active);
											j = MAX_MACHINES;
											}
										}
									PrinterWrite("\n");
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
	StatusPop();
	HelpPop();
	}


static void loadJobs(void) {
	short	handle, i;
	char	sig[sizeof(PCS100_SIG)];

	StatusPush(JobSTATUS, JobSTATUS_Loading);
	sprintf(jobName, "%s%s%s", dataPath, progName, JOB_EXT);
	if ((handle = open(jobName, O_RDONLY | O_BINARY)) != -1) {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &jobMax, sizeof(jobMax));
			if (jobMax > 0) {
				jobSize = jobMax;
				job = (JobStruct **) r_malloc(sizeof(JobStruct *) * jobSize);
				for (i=0; i<jobMax; i++) {
					job[i] = (JobStruct *) r_malloc(sizeof(JobStruct));
					read(handle, job[i], sizeof(JobStruct));
					}
				}
			}
		else
			MenuErrorPost(ErrorGet(JobERROR, JobERROR_Load), true);
		close(handle);
		}
	if (job == NULL)
		job = (JobStruct **) r_malloc(sizeof(JobStruct *) * jobSize);
	StatusPop();
	}


static void saveJobs(void) {
	short	handle, i;

	StatusPush(JobSTATUS, JobSTATUS_Saving);
	if ((handle = open(jobName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) != -1) {
		write(handle, PCS100_SIG, sizeof(PCS100_SIG));
		write(handle, &jobMax, sizeof(jobMax));
		for (i=0; i<jobMax; i++)
			write(handle, job[i], sizeof(JobStruct));
		close(handle);
		}
	else
		MenuErrorPost(ErrorGet(JobERROR, JobERROR_Save), true);
	StatusPop();
	}


static void displayAllJobsScreen(QTYPE type) {
	short	i;

	if (type == QRUN)
		for (i=js.loc; i<js.loc+js.num; i++)
			updateAllJobsScreen(job[i]->num);
	else
		for (i=js.loc; i<js.loc+js.num; i++)
		updateShiftAllJobsScreen(job[i]->num);
	}


void doAllJobsScreen(int ch, QTYPE type) {
	Boolean	changed;

	changed = false;
	switch (ch) {
		case K_HOME:
			if (js.pos > 0) {
				js.nextPos = 0;
				changed = true;
				}
			break;
		case K_UP:
			if (js.pos > 0) {
				js.nextPos = js.pos-1;
				changed = true;
				}
			break;
		case K_PGUP:
			if (js.pos > js.num-1) {
				js.nextPos = js.pos-js.num+1;
				changed = true;
				}
			else if (js.pos > 0) {
				js.nextPos = 0;
				changed = true;
				}
			break;
		case K_DOWN:
			if (js.pos < jobMax-1) {
				js.nextPos = js.pos+1;
				changed = true;
				}
			break;
		case K_PGDN:
			if (js.pos+js.num-1 < jobMax) {
				js.nextPos = js.pos+js.num-1;
				changed = true;
				}
			else if (js.pos < jobMax-1) {
				js.nextPos = jobMax-1;
				changed = true;
				}
			break;
		case K_END:
			if (js.pos < jobMax-1) {
				js.nextPos = jobMax-1;
				changed = true;
				}
			break;
		case K_SPACE:
			if (jobMax > 0) {
				js.nextPos = (js.pos==jobMax-1) ? 0 : js.pos+1;
				changed = true;
				}
			break;
		case K_RETURN:
			if (type == QRUN)
				monitorSingleJob();
			else
				monitorShiftSingleJob();
			break;
		}
	if (changed)
		redisplayAllJobsScreen(false, type);
	}


void monitorAllJobs(void) {
	Point	p;

	scrn.cur = SCREEN_ALL_JOB;
	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, JobVIDEO_All, p);
	HelpSet(JobHELP_All, MenuItemGet(JobMENU, JobMENU_All));
	redisplayAllJobsScreen(true, QRUN);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(JobSTRING, JobSTRING_PrintAll));
	ScreenTitleSet(MenuItemGet(JobMENU, JobMENU_All));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	}


void printAllJobsScreen(void) {
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1], msgStr[80];
	short		i;
	STATS		st;
	long		jobNumber, jobPreset;

	s = FormGetFirst(JobFORM_All);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		for (i=0; i<jobMax; i++) {
			jobNumber = job[i]->num;
			jobPreset = job[i]->total;
			getJobStatistics(jobNumber, &st);
			PrinterWrite("%*ld  ", JOB_NUM_LEN, jobNumber);
			if (jobPreset) {
				sprintf(msgStr, "%5.1f  ", j_percComplete(st.totalCnt, jobPreset));
				msgStr[3] = IUGetDecimal();
				PrinterWrite(msgStr);
				}
			else
				PrinterWrite("%5s  ", StringGet(JobSTRING, JobSTRING_NA));
			PrinterWrite("%8ld  ", jobPreset);
			PrinterWrite("%8ld  ", st.totalCnt);
			PrinterWrite("%8ld ", st.curRate);
			if (jobPreset) {
				IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
				PrinterWrite("%s  ", msgStr);
				IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
				PrinterWrite("%s\n", msgStr);
				}
			else
				PrinterWrite("%10s  %10s\n", StringGet(JobSTRING, JobSTRING_NA), StringGet(JobSTRING, JobSTRING_NA));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	}


static void redisplayAllJobsScreen(Boolean update, QTYPE type) {
	short	old;
	Rect	r;

	if ((js.pos != js.nextPos) || update) {
		old = js.pos;
		js.pos = js.nextPos;
		if (old != JOB_NONE) {
			if (type == QRUN)
				updateAllJobsScreen(job[old]->num);
			else
				updateShiftAllJobsScreen(job[old]->num);
			}
		if (js.pos < js.loc) {
			js.loc = js.pos;
			displayAllJobsScreen(type);
			}
		else if (js.pos > js.loc+js.num-1) {
			js.loc = js.pos-js.num+1;
			displayAllJobsScreen(type);
			}
		else if (update)
			displayAllJobsScreen(type);
		else {
			if (type == QRUN)
				updateAllJobsScreen(job[js.pos]->num);
			else
				updateShiftAllJobsScreen(job[js.pos]->num);
			}
		if (update && (js.num<js.max)) {
			r.Set(3+js.num, 0, scrn.bot, scrn.rig);
			VirtualFill(scrn.num, r, ' ', scrn.lo);
			}
		}
	}


void updateAllJobsScreen(long jobNumber) {
	short	jPos, v, attr;
	Rect	r;
	long	jobPreset;
	char	msgStr[80];
	STATS	st;

	jPos = findJob(jobNumber);
	if (jPos<js.loc || jPos>js.loc+js.num-1)
		return;
	v = jPos-js.loc+3;
	attr = (jPos==js.pos) ? scrn.inv : scrn.hi;
	r.Set(v, 0, v, scrn.rig);
	VirtualFill(scrn.num, r, ' ', attr);
	jobPreset = job[jPos]->total;
	getJobStatistics(jobNumber, &st);
	VirtualPrint(scrn.num, v,  2, GIANT, attr, "%*ld", JOB_NUM_LEN, jobNumber);
	VirtualPrint(scrn.num, v, 21, GIANT, attr, "%8ld", jobPreset);
	VirtualPrint(scrn.num, v, 32, GIANT, attr, "%8ld", st.totalCnt);
	VirtualPrint(scrn.num, v, 43, GIANT, attr, "%8ld", st.curRate);
	if (jobPreset) {
		sprintf(msgStr, "%5.1f", j_percComplete(st.totalCnt, jobPreset));
		msgStr[3] = IUGetDecimal();
		VirtualWrite(scrn.num, v, 13, GIANT, msgStr, attr);
		IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
		VirtualWrite(scrn.num, v, 54, GIANT, msgStr, attr);
		IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
		VirtualWrite(scrn.num, v, 66, GIANT, msgStr, attr);
		}
	else {
		VirtualPrint(scrn.num, v, 13, GIANT, attr, "%5s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, v, 54, GIANT, attr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, v, 66, GIANT, attr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		}
	}


void doSingleJobScreen(int ch, QTYPE type) {
	Boolean	changed;

	changed = false;
	switch (ch) {
		case K_HOME:
			if (js.pos > 0) {
				js.nextPos = 0;
				changed = true;
				}
			break;
		case K_UP:
			if (js.pos > 0) {
				js.nextPos = js.pos-1;
				changed = true;
				}
			break;
		case K_DOWN:
			if (js.pos < jobMax-1) {
				js.nextPos = js.pos+1;
				changed = true;
				}
			break;
		case K_END:
			if (js.pos < jobMax-1) {
				js.nextPos = jobMax-1;
				changed = true;
				}
			break;
		case K_SPACE:
			if (jobMax > 1) {
				js.nextPos = (js.pos==jobMax-1) ? 0 : js.pos+1;
				changed = true;
				}
			break;
		case K_PGUP:
			if (js.mchPos > JOB_SINGLE_MAX-2) {
				js.mchPos -= JOB_SINGLE_MAX - 1;
				changed = true;
				}
			else if (js.mchPos > 0) {
				js.mchPos = 0;
				changed = true;
				}
			break;
		case K_PGDN:
			js.mchPos += JOB_SINGLE_MAX - 1;
			changed = true;
			break;
		case K_ESC:
			if (type == QRUN)
				monitorAllJobs();
			else
				monitorShiftAllJobs();
			break;
		}
	if (changed) {
		if (js.nextPos != js.pos)
			js.mchPos = 0;
		redisplaySingleJobScreen(type);
		}
	}


void monitorSingleJob(void) {
	Point	p;

	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, JobVIDEO_Single, p);
	scrn.cur = SCREEN_SNGL_JOB;
	HelpSet(JobHELP_Single, MenuItemGet(JobMENU, JobMENU_Single));
	redisplaySingleJobScreen(QRUN);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(JobSTRING, JobSTRING_PrintSingle));
	ScreenTitleSet(MenuItemGet(JobMENU, JobMENU_Single));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	js.mchPos = 0;
	}


void printSingleJobScreen(void) {
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1], msgStr[80], pcsLeft[15];
	long		jobNumber, jobPreset;
	short		i, counter;
	QueryJobStruct	*qj;
	STATS		st;

	s = FormGetFirst(JobFORM_Single);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		jobNumber = job[js.pos]->num;
		jobPreset = job[js.pos]->total;
		getJobStatistics(jobNumber, &st);
		if (jobPreset)
			sprintf(msgStr, "%5.1f", j_percComplete(st.totalCnt, jobPreset));
		else
			sprintf(msgStr, "%5s", StringGet(JobSTRING, JobSTRING_NA));
		PrinterWrite(FormGetNext(), JOB_NUM_LEN, jobNumber, msgStr, st.curRate);
		PrinterWrite(FormGetNext(), JOB_TOT_LEN, jobPreset, st.avRate);
		if (jobPreset)
			IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
		else
			sprintf(msgStr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		PrinterWrite(FormGetNext(), st.totalCnt, msgStr);
		if (jobPreset) {
			sprintf(pcsLeft, "%8ld", jobPreset-st.totalCnt);
			IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
			}
		else {
			sprintf(pcsLeft, "%8s", StringGet(JobSTRING, JobSTRING_NA));
			sprintf(msgStr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
			}
		PrinterWrite(FormGetNext(), pcsLeft, msgStr);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		i = 0;
		while (i < mchMax) {
			counter = mach[i]->counter;
			qj = getJobQuery(counter);
			if (qj->jobN == jobNumber) {
				PrinterWrite("%0*d      ", MACH_CNTR_LEN, counter);
				PrinterWrite("%s  ", q_status(counter));
				PrinterWrite("%s   ", q_currentRate(counter));
				PrinterWrite("%s ", q_averageRate(counter, QRUN));
				PrinterWrite(q_totalCount(counter, QRUN));
				PrinterWrite("%s  ", q_pcsTilDone(counter));
				PrinterWrite("%s  ", q_noIdleTime(counter));
				PrinterWrite("%s\n", q_withIdleTime(counter));
				}
			i++;
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	}


static void redisplaySingleJobScreen(QTYPE type) {
	js.pos = js.nextPos;
	if (type == QRUN)
		updateSingleJobScreen(job[js.pos]->num);
	else
		updateShiftSingleJobScreen(job[js.pos]->num);
	}


void updateSingleJobScreen(long jobNumber) {
	long		jobPreset;
	short		jobMchCnt, i, curNum, counter, mchNum, pos;
	char		msgStr[80];
	Rect		r;
	QueryJobStruct	*qj;
	STATS		st;

	if (jobNumber != job[js.pos]->num)
		return;
	jobMchCnt = getJobStatistics(jobNumber, &st);
	if (jobMchCnt > JOB_SINGLE_MAX) {
		mchNum = JOB_SINGLE_MAX;
		if (js.mchPos+mchNum > jobMchCnt)
			js.mchPos = jobMchCnt - mchNum;
		}
	else {
		mchNum = jobMchCnt;
		js.mchPos = 0;
		r.Set(8+jobMchCnt, 0, scrn.bot, scrn.rig);
		VirtualFill(scrn.num, r, ' ', scrn.lo);
		}
	curNum = 0;
	i = js.mchPos;
	while (curNum < mchNum) {
		counter = mach[i]->counter;
		qj = getJobQuery(counter);
		if (qj->jobN == jobNumber) {
			pos = 8 + curNum;
			VirtualPrint(scrn.num, pos,  0, GIANT, scrn.hi, "%0*d", MACH_CNTR_LEN, counter);
			VirtualWrite(scrn.num, pos,  8, GIANT, q_status(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 20, GIANT, q_currentRate(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 29, GIANT, q_averageRate(counter, QRUN), scrn.hi);
			VirtualWrite(scrn.num, pos, 36, GIANT, q_totalCount(counter, QRUN), scrn.hi);
			VirtualWrite(scrn.num, pos, 44, GIANT, q_pcsTilDone(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 56, GIANT, q_noIdleTime(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 68, GIANT, q_withIdleTime(counter), scrn.hi);
			curNum++;
			}
		i++;
		}
	jobPreset = job[js.pos]->total;
	VirtualPrint(scrn.num, 0,  7, GIANT, scrn.hi, "%*ld", JOB_NUM_LEN, jobNumber);
	VirtualPrint(scrn.num, 0, 69, GIANT, scrn.hi, "%8ld", st.curRate);
	VirtualPrint(scrn.num, 1, 17, GIANT, scrn.hi, "%*lu", JOB_TOT_LEN, jobPreset);
	VirtualPrint(scrn.num, 1, 69, GIANT, scrn.hi, "%8.0f", st.avRate);
	VirtualPrint(scrn.num, 2, 15, GIANT, scrn.hi, "%10ld", st.totalCnt);
	if (jobPreset) {
		sprintf(msgStr, "%5.1f", j_percComplete(st.totalCnt, jobPreset));
		msgStr[3] = IUGetDecimal();
		VirtualWrite(scrn.num, 0, 19, GIANT, msgStr, scrn.hi);
		IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
		VirtualPrint(scrn.num, 2, 67, GIANT, scrn.hi, msgStr);
		VirtualPrint(scrn.num, 3, 17, GIANT, scrn.hi, "%8ld", jobPreset-st.totalCnt);
		IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
		VirtualPrint(scrn.num, 3, 67, GIANT, scrn.hi, msgStr);
		}
	else {
		VirtualPrint(scrn.num, 0, 19, GIANT, scrn.hi, "%5s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, 2, 67, GIANT, scrn.hi, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, 3, 17, GIANT, scrn.hi, "%8s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, 3, 67, GIANT, scrn.hi, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		}
	}


void monitorShiftAllJobs(void) {
	Point	p;

	scrn.cur = SCREEN_ALL_SJOB;
	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, ShiftVIDEO_AllJob, p);
	HelpSet(ShiftHELP_AllJobs, MenuItemGet(ShiftMENU, ShiftMENU_AllJobs));
	redisplayAllJobsScreen(true, QSHIFT);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(JobSTRING, JobSTRING_PrintAllShift));
	ScreenTitleSet(MenuItemGet(ShiftMENU, ShiftMENU_AllJobs));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	}


void updateShiftAllJobsScreen(long jobNumber) {
	short	jPos, v, attr;
	Rect	r;
	long	jobPreset;
	char	msgStr[80];
	STATS	st;

	jPos = findJob(jobNumber);
	if (jPos<js.loc || jPos>js.loc+js.num-1)
		return;
	v = jPos-js.loc+3;
	attr = (jPos==js.pos) ? scrn.inv : scrn.hi;
	r.Set(v, 0, v, scrn.rig);
	VirtualFill(scrn.num, r, ' ', attr);
	jobPreset = job[jPos]->total;
	getShiftStatistics(jobNumber, &st);
	VirtualPrint(scrn.num, v,  2, GIANT, attr, "%*ld", JOB_NUM_LEN, jobNumber);
	VirtualPrint(scrn.num, v, 21, GIANT, attr, "%8ld", jobPreset);
	VirtualPrint(scrn.num, v, 32, GIANT, attr, "%8ld", st.totalCnt);
	VirtualPrint(scrn.num, v, 43, GIANT, attr, "%8ld", st.curRate);
	if (jobPreset) {
		sprintf(msgStr, "%5.1f", j_percComplete(st.totalCnt, jobPreset));
		msgStr[3] = IUGetDecimal();
		VirtualWrite(scrn.num, v, 13, GIANT, msgStr, attr);
		IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
		VirtualWrite(scrn.num, v, 54, GIANT, msgStr, attr);
		IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
		VirtualWrite(scrn.num, v, 66, GIANT, msgStr, attr);
		}
	else {
		VirtualPrint(scrn.num, v, 13, GIANT, attr, "%5s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, v, 54, GIANT, attr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, v, 66, GIANT, attr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		}
	}


void printShiftAllJobsScreen(void) {
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1], msgStr[80];
	short		i;
	long		jobNumber, jobPreset;
	STATS		st;

	s = FormGetFirst(ShiftFORM_AllJob);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		for (i=0; i<jobMax; i++) {
			jobNumber = job[i]->num;
			jobPreset = job[i]->total;
			getShiftStatistics(jobNumber, &st);
			PrinterWrite("%*ld  ", JOB_NUM_LEN, jobNumber);
			if (jobPreset) {
				sprintf(msgStr, "%5.1f  ", j_percComplete(st.totalCnt, jobPreset));
				msgStr[3] = IUGetDecimal();
				PrinterWrite(msgStr);
				}
			else
				PrinterWrite("%5s  ", StringGet(JobSTRING, JobSTRING_NA));
			PrinterWrite("%8ld  ", jobPreset);
			PrinterWrite("%8ld  ", st.totalCnt);
			PrinterWrite("%8ld ", st.curRate);
			if (jobPreset) {
				IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
				PrinterWrite("%s  ", msgStr);
				IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
				PrinterWrite("%s\n", msgStr);
				}
			else
				PrinterWrite("%10s  %10s\n", StringGet(JobSTRING, JobSTRING_NA), StringGet(JobSTRING, JobSTRING_NA));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	}


void monitorShiftSingleJob(void) {
	Point	p;

	VirtualFill(scrn.num, GIANT_RECT, ' ', scrn.lo);
	p.Set(0, 0);
	VirtualGetVideo(scrn.num, ShiftVIDEO_SingleJob, p);
	scrn.cur = SCREEN_SNGL_SJOB;
	HelpSet(ShiftHELP_SingleJob, MenuItemGet(ShiftMENU, ShiftMENU_SingleJob));
	redisplaySingleJobScreen(QSHIFT);
	MenuItemSet(ReportMENU, ReportMENU_Print, StringGet(JobSTRING, JobSTRING_PrintSingleShift));
	ScreenTitleSet(MenuItemGet(ShiftMENU, ShiftMENU_SingleJob));
	MenuItemEnable(ReportMENU, ReportMENU_Print);
	js.mchPos = 0;
	}


void updateShiftSingleJobScreen(long jobNumber) {
	long			jobPreset;
	short			jobMchCnt, i, curNum, counter, mchNum, pos;
	char			msgStr[80];
	Rect			r;
	QueryShiftStruct	*qs;
	STATS			st;

	if (jobNumber != job[js.pos]->num)
		return;
	jobMchCnt = getShiftStatistics(jobNumber, &st);
	if (jobMchCnt > JOB_SINGLE_MAX) {
		mchNum = JOB_SINGLE_MAX;
		if (js.mchPos+mchNum > jobMchCnt)
			js.mchPos = jobMchCnt - mchNum;
		}
	else {
		mchNum = jobMchCnt;
		js.mchPos = 0;
		r.Set(8+jobMchCnt, 0, scrn.bot, scrn.rig);
		VirtualFill(scrn.num, r, ' ', scrn.lo);
		}
	curNum = 0;
	i = js.mchPos;
	while (curNum < mchNum) {
		counter = mach[i]->counter;
		qs = getShiftQuery(counter);
		if (qs->jobN == jobNumber) {
			pos = 8 + curNum;
			VirtualPrint(scrn.num, pos,  0, GIANT, scrn.hi, "%0*d", MACH_CNTR_LEN, counter);
			VirtualWrite(scrn.num, pos,  8, GIANT, q_status(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 20, GIANT, q_currentRate(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 29, GIANT, q_averageRate(counter, QSHIFT), scrn.hi);
			VirtualWrite(scrn.num, pos, 36, GIANT, q_totalCount(counter, QSHIFT), scrn.hi);
			VirtualWrite(scrn.num, pos, 44, GIANT, q_pcsTilDone(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 56, GIANT, q_noIdleTime(counter), scrn.hi);
			VirtualWrite(scrn.num, pos, 68, GIANT, q_withIdleTime(counter), scrn.hi);
			curNum++;
			}
		i++;
		}
	jobPreset = job[js.pos]->total;
	VirtualPrint(scrn.num, 0,  7, GIANT, scrn.hi, "%*ld", JOB_NUM_LEN, jobNumber);
	VirtualPrint(scrn.num, 0, 69, GIANT, scrn.hi, "%8ld", st.curRate);
	VirtualPrint(scrn.num, 1, 17, GIANT, scrn.hi, "%*lu", JOB_TOT_LEN, jobPreset);
	VirtualPrint(scrn.num, 1, 69, GIANT, scrn.hi, "%8.0f", st.avRate);
	VirtualPrint(scrn.num, 2, 15, GIANT, scrn.hi, "%10ld", st.totalCnt);
	if (jobPreset) {
		sprintf(msgStr, "%5.1f", j_percComplete(st.totalCnt, jobPreset));
		msgStr[3] = IUGetDecimal();
		VirtualWrite(scrn.num, 0, 19, GIANT, msgStr, scrn.hi);
		IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
		VirtualPrint(scrn.num, 2, 67, GIANT, scrn.hi, msgStr);
		VirtualPrint(scrn.num, 3, 17, GIANT, scrn.hi, "%8ld", jobPreset-st.totalCnt);
		IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
		VirtualPrint(scrn.num, 3, 67, GIANT, scrn.hi, msgStr);
		}
	else {
		VirtualPrint(scrn.num, 0, 19, GIANT, scrn.hi, "%5s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, 2, 67, GIANT, scrn.hi, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, 3, 17, GIANT, scrn.hi, "%8s", StringGet(JobSTRING, JobSTRING_NA));
		VirtualPrint(scrn.num, 3, 67, GIANT, scrn.hi, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		}
	}


void printShiftSingleJobScreen(void) {
	char			*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1], msgStr[80], pcsLeft[15];
	long			jobNumber, jobPreset;
	short			i, counter;
	QueryShiftStruct	*qs;
	STATS			st;

	s = FormGetFirst(ShiftFORM_SingleJob);
	if (s && PrinterOpen()) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		PrinterWrite(s, date, time);
		jobNumber = job[js.pos]->num;
		jobPreset = job[js.pos]->total;
		getShiftStatistics(jobNumber, &st);
		if (jobPreset)
			sprintf(msgStr, "%5.1f", j_percComplete(st.totalCnt, jobPreset));
		else
			sprintf(msgStr, "%5s", StringGet(JobSTRING, JobSTRING_NA));
		PrinterWrite(FormGetNext(), JOB_NUM_LEN, jobNumber, msgStr, st.curRate);
		PrinterWrite(FormGetNext(), JOB_TOT_LEN, jobPreset, st.avRate);
		if (jobPreset)
			IULTime2Str(IUHrs2LTime(j_noIdleTime(jobPreset, st.totalCnt, st.curRate)), msgStr);
		else
			sprintf(msgStr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
		PrinterWrite(FormGetNext(), st.totalCnt, msgStr);
		if (jobPreset) {
			sprintf(pcsLeft, "%8ld", jobPreset-st.totalCnt);
			IULTime2Str(IUHrs2LTime(j_withIdleTime(jobPreset, st.totalCnt, st.avRate)), msgStr);
			}
		else {
			sprintf(pcsLeft, "%8s", StringGet(JobSTRING, JobSTRING_NA));
			sprintf(msgStr, "%10s", StringGet(JobSTRING, JobSTRING_NA));
			}
		PrinterWrite(FormGetNext(), pcsLeft, msgStr);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		i = 0;
		while (i < mchMax) {
			counter = mach[i]->counter;
			qs = getShiftQuery(counter);
			if (qs->jobN == jobNumber) {
				PrinterWrite("%0*d      ", MACH_CNTR_LEN, counter);
				PrinterWrite("%s  ", q_status(counter));
				PrinterWrite("%s   ", q_currentRate(counter));
				PrinterWrite("%s ", q_averageRate(counter, QSHIFT));
				PrinterWrite(q_totalCount(counter, QSHIFT));
				PrinterWrite("%s  ", q_pcsTilDone(counter));
				PrinterWrite("%s  ", q_noIdleTime(counter));
				PrinterWrite("%s\n", q_withIdleTime(counter));
				}
			i++;
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	}


void closeJob(void) {
	short		vsnum, i;
	Rect		r;
	ListStruct	ls;
	Boolean		done;

	HelpPush(JobHELP_Close, MenuItemGet(JobMENU, JobMENU_Close));
	StatusPush(JobSTATUS, JobSTATUS_CloseList);
	vsnum = VirtualGetNew(JobWINDOW_Close, &r);
	ls.vsnum = vsnum;
	ls.count = jobMax;
	ls.list = (char **) r_malloc(sizeof(char *) * ls.count);
	for (i=0; i<ls.count; i++) {
		ls.list[i] = (char *) r_malloc(12+strlen(job[i]->name));
		sprintf(ls.list[i], "%8lu - %s", job[i]->num, job[i]->name);
		}
	ls.top = 0;
	ls.lef = 0;
	ls.height = r.Height();
	ls.width = r.Width()-2;
	ls.loc = 0;
	ls.pos = 0;
	done = false;
	while (!done) {
		switch (ListScroll(&ls)) {
			case K_ESC:
				done = true;
				break;
			case K_RETURN:
				done = saveArchivalJob(ls.pos);
				break;
			}
		}
	for (i=0; i<ls.count; i++)
		r_free(ls.list[i]);
	r_free(ls.list);
	VirtualDispose(vsnum);
	StatusPop();
	HelpPop();
	}


static Boolean saveArchivalJob(short jPos) {
	char	*name, path[MAXPATH], dir[MAXDIR], file[MAXFILE];

	if ((name = DialogOpen(OpenFilterSTRING_Closed, true)) == NULL)
		return (false);
	fnsplit(name, path, dir, NULL, NULL);
	fnsplit(StringGet(OpenFilterSTRING_Closed, 0), NULL, NULL, file, NULL);
	strcat(path, dir);
	strcat(path, file);
	StatusPush(StatusGet(JobSTATUS, JobSTATUS_Closing), job[jPos]->num);
	if (doSaveArchivalJob(path, jPos)) {
		transferArchJob(path, job[jPos]->num);
		transferArchShift(path, job[jPos]->num);
		saveClosedJob(job[jPos]->num);
		deleteJob(jPos, job[js.pos]->num);
		saveJobs();
		}
	StatusPop();
	return (true);
	}


static Boolean doSaveArchivalJob(char *name, short jPos) {
	short		handle, ajNum, i;
	JobStruct	**aJob;
	char		path[MAXPATH], sig[sizeof(PCS100_SIG)];

	sprintf(path, "%s%s", name, JOB_EXT);
	if ((handle = open(path, O_RDONLY | O_BINARY)) == -1) {
		ajNum = 1;
		aJob = (JobStruct **) r_malloc(sizeof(JobStruct *) * ajNum);
		}
	else {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &ajNum, sizeof(ajNum));
			ajNum++;
			aJob = (JobStruct **) r_malloc(sizeof(JobStruct *) * ajNum);
			for (i=0; i<ajNum-1; i++) {
				aJob[i] = (JobStruct *) r_malloc(sizeof(JobStruct));
				read(handle, aJob[i], sizeof(JobStruct));
				if (job[jPos]->num == aJob[i]->num) {
					r_free(aJob[i]);
					i--;
					ajNum--;
					}
				}
			close(handle);
			}
		else {
			ajNum = 1;
			aJob = (JobStruct **) r_malloc(sizeof(JobStruct *) * ajNum);
			MenuErrorPost(ErrorGet(JobERROR, JobERROR_Load), true);
			}
		}
	aJob[ajNum-1] = (JobStruct *) r_malloc(sizeof(JobStruct));
	memmove(aJob[ajNum-1], job[jPos], sizeof(JobStruct));
	aJob[ajNum-1]->date = SysDate();
	if ((handle = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) != -1) {
		qsort(aJob, ajNum, sizeof(JobStruct *), jobCompare);
		write(handle, PCS100_SIG, sizeof(PCS100_SIG));
		write(handle, &ajNum, sizeof(ajNum));
		for (i=0; i<ajNum; i++) {
			write(handle, aJob[i], sizeof(JobStruct));
			r_free(aJob[i]);
			}
		r_free(aJob);
		close(handle);
		}
	else {
		MenuErrorPost(ErrorGet(JobERROR, JobERROR_Save), true);
		return (false);
		}
	return (true);
	}


void exportOpenJobs(void) {
	char	*prompt, *name;

	prompt = StringGet(JobSTRING, JobSTRING_ExportOpenJob);
	if ((name = DialogSave(SaveFilterSTRING_Open, "OJOBS.ASC", prompt)) == NULL)
		return;
	exportOpenArchJobs(name);
	prompt = StringGet(JobSTRING, JobSTRING_ExportOpenShift);
	if ((name = DialogSave(SaveFilterSTRING_Open, "OSHIFTS.ASC", prompt)) == NULL)
		return;
	exportOpenArchShifts(name);
	}


void exportClosedJobs(void) {
	char	*name, nameFrom[MAXPATH], *nameTo, *prompt, dir[MAXDIR], file[MAXFILE];

	while (true) {
		if ((name = DialogOpen(OpenFilterSTRING_Closed, false)) == NULL)
			break;
		fnsplit(name, nameFrom, dir, NULL, NULL);
		fnsplit(StringGet(OpenFilterSTRING_Closed, 0), NULL, NULL, file, NULL);
		strcat(nameFrom, dir);
		strcat(nameFrom, file);
		prompt = StringGet(JobSTRING, JobSTRING_ExportClosedJob);
		if ((nameTo = DialogSave(SaveFilterSTRING_Closed, "CJOBS.ASC", prompt)) == NULL)
			continue;
		exportClosedArchJobs(nameFrom, nameTo);
		prompt = StringGet(JobSTRING, JobSTRING_ExportClosedShift);
		if ((nameTo = DialogSave(SaveFilterSTRING_Closed, "CSHIFTS.ASC", prompt)) == NULL)
			continue;
		exportClosedArchShifts(nameFrom, nameTo);
		break;
		}
	}
