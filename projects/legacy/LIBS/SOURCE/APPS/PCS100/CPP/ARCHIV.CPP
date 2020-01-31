// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\coord.h>
#include <common\getargs.h>
#include <common\jiffy.h>
#include <common\keyboard.h>
#include <common\system.h>

#include <seashell\color.h>
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

#include <ctree\ctport.h>
#include <ctree\tc\ctoptn.h>
#include <ctree\ctstrc.h>
#include <ctree\ctisam.h>
#include <ctree\ctifil.h>
#include <ctree\cterrc.h>
#include <ctree\ctdecl.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "mach.h"
#include "job.h"
#include "query.h"
#include "archiv.h"

#define OFF		0
#define ON		1

typedef struct {
	TEXT	delFlag;
	DWord	jobN;
	} ClosedJobStruct;

#define EXT_SIZE	1024		/* block size of disk allocation */

#define QRUN_TAIL	'J'
#define QSHIFT_TAIL	'S'
#define QCLOSE_TAIL	'C'

#define QRUN_NUM_IDX	4
#define QSHIFT_NUM_IDX	4
#define JCLOSED_NUM_IDX	1

#define ISAM_BUFS	6	/* number of index file buffers */
#define ISAM_FILS	17	/* max number of files open simultaneously */
#define ISAM_SECT	4	/* number of node sectors */

#define QRUN_SIZE_ADR		sizeof(((QueryJobStruct *)0)->adr)
#define QRUN_SIZE_JOBN		sizeof(((QueryJobStruct *)0)->jobN)
#define QRUN_SIZE_SHIFTN	sizeof(((QueryJobStruct *)0)->shiftN)
#define QRUN_SIZE_OPERN		sizeof(((QueryJobStruct *)0)->operN)
#define QRUN_SIZE_KEY		(QRUN_SIZE_ADR + QRUN_SIZE_JOBN)

#define QSHIFT_SIZE_ADR		QRUN_SIZE_ADR
#define QSHIFT_SIZE_JOBN	QRUN_SIZE_JOBN
#define QSHIFT_SIZE_SHIFTN	QRUN_SIZE_SHIFTN
#define QSHIFT_SIZE_OPERN	QRUN_SIZE_OPERN
#define QSHIFT_SIZE_KEY		(QSHIFT_SIZE_ADR + QSHIFT_SIZE_JOBN + QSHIFT_SIZE_SHIFTN)

#define JCLOSED_SIZE		sizeof(ClosedJobStruct)
#define JCLOSED_SIZE_JOBN	sizeof(((ClosedJobStruct *)0)->jobN)

#define QRUN_LOC_ADR		offsetof(QueryJobStruct, adr)
#define QRUN_LOC_JOBN		offsetof(QueryJobStruct, jobN)
#define QRUN_LOC_SHIFTN		offsetof(QueryJobStruct, shiftN)
#define QRUN_LOC_OPERN		offsetof(QueryJobStruct, operN)

#define QSHIFT_LOC_ADR		offsetof(QueryShiftStruct, adr)
#define QSHIFT_LOC_JOBN		offsetof(QueryShiftStruct, jobN)
#define QSHIFT_LOC_SHIFTN	offsetof(QueryShiftStruct, shiftN)
#define QSHIFT_LOC_OPERN	offsetof(QueryShiftStruct, operN)

#define JCLOSED_LOC_JOBN	offsetof(ClosedJobStruct, jobN)

/*
 * cur_job_text and cur_shift_text is the buffer for existing data.
 * new_job_text and new_shift_text is the buffer for new/updated data.
 */

static TEXT cur_job_text[QRUN_SIZE],		new_job_text[QRUN_SIZE];
static TEXT cur_shift_text[QSHIFT_SIZE],	new_shift_text[QSHIFT_SIZE];
static TEXT cur_jclosed_text[JCLOSED_SIZE],	new_jclosed_text[JCLOSED_SIZE];

static ISEG QJobSeg[] = {
	{ QRUN_LOC_ADR,   QRUN_SIZE_ADR,   REGSEG },
	{ QRUN_LOC_JOBN,  QRUN_SIZE_JOBN,  INTSEG },
	{ QRUN_LOC_OPERN, QRUN_SIZE_OPERN, INTSEG }
	};

static ISEG QShiftSeg[] = {
	{ QSHIFT_LOC_ADR,    QSHIFT_SIZE_ADR,    REGSEG },
	{ QSHIFT_LOC_JOBN,   QSHIFT_SIZE_JOBN,   INTSEG },
	{ QSHIFT_LOC_SHIFTN, QSHIFT_SIZE_SHIFTN, REGSEG },
	{ QSHIFT_LOC_OPERN,  QSHIFT_SIZE_OPERN,  INTSEG }
	};

static ISEG JClosedSeg[] = {
	{ JCLOSED_LOC_JOBN, JCLOSED_SIZE_JOBN, INTSEG }
	};

static IIDX QJobIndices[] = {
	{				/* 1st index */
		QRUN_SIZE_ADR+4,	/* key length */
		KTYPE_L2R,		/* key type */
		ON,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		QJobSeg			/* pointer to segment array */
		},
	{				/* 2nd index */
		QRUN_SIZE_JOBN+4,	/* key length */
		KTYPE_L2R,		/* key type */
		ON,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		&QJobSeg[1]		/* pointer to segment array */
		},
	{				/* 3rd index */
		QRUN_SIZE_OPERN+4,	/* key length */
		KTYPE_L2R,		/* key type */
		ON,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		&QJobSeg[2]		/* pointer to segment array */
		},
	{				/* 4th index */
		QRUN_SIZE_KEY,		/* key length */
		KTYPE_L2R,		/* key type */
		OFF,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		2,			/* number of key segments */
		QJobSeg			/* pointer to segment array */
		}
	};

static IIDX QShiftIndices[] = {
	{				/* 1st index */
		QSHIFT_SIZE_ADR+4,	/* key length */
		KTYPE_L2R,		/* key type */
		ON,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		QShiftSeg		/* pointer to segment array */
		},
	{				/* 2nd index */
		QSHIFT_SIZE_JOBN+4,	/* key length */
		KTYPE_L2R,		/* key type */
		ON,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		&QShiftSeg[1]		/* pointer to segment array */
		},
	{				/* 3rd index */
		QSHIFT_SIZE_OPERN+4,	/* key length */
		KTYPE_L2R,		/* key type */
		ON,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		&QShiftSeg[3]		/* pointer to segment array */
		},
	{				/* 4th index */
		QSHIFT_SIZE_KEY,	/* key length */
		KTYPE_L2R,		/* key type */
		OFF,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		3,			/* number of key segments */
		QShiftSeg		/* pointer to segment array */
		}
	};

static IIDX JClosedIndices[] = {
	{				/* 1st index */
		JCLOSED_SIZE_JOBN,	/* key length */
		KTYPE_L2R,		/* key type */
		OFF,			/* duplicate flag */
		OFF,			/* null key flag */
		'\0',			/* empty character */
		1,			/* number of key segments */
		JClosedSeg		/* pointer to segment array */
		}
	};

static IFIL aJob = {
	NULL,		/* name of data and index file w/o extension (.DAT or .IDX) */
	-1,		/* find first available file number */
	QRUN_SIZE,	/* data record length */
	EXT_SIZE,	/* block size of disk allocation for data file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) data file mode */
	QRUN_NUM_IDX,	/* number of associated indices */
	EXT_SIZE,	/* block size of disk allocation for index file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) index file mode */
	QJobIndices	/* pointer to IIDEX index structure */
	};

static IFIL aShift = {
	NULL,		/* name of data and index file w/o extension (.DAT or .IDX) */
	-1,		/* find first available file number */
	QSHIFT_SIZE,	/* data record length */
	EXT_SIZE,	/* block size of disk allocation for data file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) data file mode */
	QSHIFT_NUM_IDX,	/* number of associated indices */
	EXT_SIZE,	/* block size of disk allocation for index file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) index file mode */
	QShiftIndices	/* pointer to IIDEX index structure */
	};

static IFIL haJob = {
	NULL,		/* name of data and index file w/o extension (.DAT or .IDX) */
	-1,		/* find first available file number */
	QRUN_SIZE,	/* data record length */
	EXT_SIZE,	/* block size of disk allocation for data file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) data file mode */
	QRUN_NUM_IDX,	/* number of associated indices */
	EXT_SIZE,	/* block size of disk allocation for index file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) index file mode */
	QJobIndices	/* pointer to IIDEX index structure */
	};

static IFIL haShift = {
	NULL,		/* name of data and index file w/o extension (.DAT or .IDX) */
	-1,		/* find first available file number */
	QSHIFT_SIZE,	/* data record length */
	EXT_SIZE,	/* block size of disk allocation for data file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) data file mode */
	QSHIFT_NUM_IDX,	/* number of associated indices */
	EXT_SIZE,	/* block size of disk allocation for index file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) index file mode */
	QShiftIndices	/* pointer to IIDEX index structure */
	};

static IFIL jClosed = {
	NULL,		/* name of data and index file w/o extension (.DAT or .IDX) */
	-1,		/* find first available file number */
	JCLOSED_SIZE,	/* data record length */
	EXT_SIZE,	/* block size of disk allocation for data file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) data file mode */
	JCLOSED_NUM_IDX,/* number of associated indices */
	EXT_SIZE,	/* block size of disk allocation for index file */
	FMODE_VEF,	/* ( VIRTUAL | EXCLUSIVE | FIXED ) index file mode */
	JClosedIndices	/* pointer to IIDEX index structure */
	};

#define JOB_DATA_NUM		aJob.tfilno
#define JOB_ADR_NUM		(JOB_DATA_NUM + 1)
#define JOB_JOBN_NUM		(JOB_DATA_NUM + 2)
#define JOB_OPERN_NUM		(JOB_DATA_NUM + 3)
#define JOB_KEY_NUM		(JOB_DATA_NUM + 4)

#define SHIFT_DATA_NUM		aShift.tfilno
#define SHIFT_ADR_NUM		(SHIFT_DATA_NUM + 1)
#define SHIFT_JOBN_NUM		(SHIFT_DATA_NUM + 2)
#define SHIFT_OPERN_NUM		(SHIFT_DATA_NUM + 3)
#define SHIFT_KEY_NUM		(SHIFT_DATA_NUM + 4)

#define HJOB_DATA_NUM		haJob.tfilno
#define HJOB_ADR_NUM		(HJOB_DATA_NUM + 1)
#define HJOB_JOBN_NUM		(HJOB_DATA_NUM + 2)
#define HJOB_OPERN_NUM		(HJOB_DATA_NUM + 3)
#define HJOB_KEY_NUM		(HJOB_DATA_NUM + 4)

#define HSHIFT_DATA_NUM		haShift.tfilno
#define HSHIFT_ADR_NUM		(HSHIFT_DATA_NUM + 1)
#define HSHIFT_JOBN_NUM		(HSHIFT_DATA_NUM + 2)
#define HSHIFT_OPERN_NUM	(HSHIFT_DATA_NUM + 3)
#define HSHIFT_KEY_NUM		(HSHIFT_DATA_NUM + 4)

#define JCLOSED_DATA_NUM	jClosed.tfilno
#define JCLOSED_KEY_NUM		(JCLOSED_DATA_NUM+1)

static char	aMsgStr[80];
static char	aJobName[MAXPATH], aShiftName[MAXPATH], aClosedName[MAXPATH];

static void	ArchiveCleanUp(void);

static void	openIFIL(IFIL *name);
static void	ctreeError(void);

static void	printArchJobMachine(void);
static void	printArchJobJob(void);
static void	printArchJobOperator(void);
static void	printArchShiftMachine(void);
static void	printArchShiftJob(void);
static void	printArchShiftOperator(void);

static char	*a_adr(void *q, QTYPE type);
static char	*a_jobNumber(void *q, QTYPE type);
static char	*a_opNumber(void *q, QTYPE type);
static char	*a_shiftNumber(void *q, QTYPE type);
static char	*a_totalCount(void *q, QTYPE type);
static char	*a_averageRate(void *q, QTYPE type);
static char	*a_runTime(void *q, QTYPE type);
static char	*a_idleTime(void *q, QTYPE type);
static char	*a_setupTime(void *q, QTYPE type);


static void ArchiveCleanUp(void) {
	if (!host)
		return;
	StatusPush(ArchiveSTATUS, ArchiveSTATUS_Saving);
	if (CLISAM())
		ctreeError();
	StatusPop();
	}


void ArchiveInit(void) {
	if (!host)
		return;
	StatusPush(ArchiveSTATUS, ArchiveSTATUS_Loading);
	aJob.pfilnam = aJobName;
	sprintf(aJob.pfilnam, "%s%s%c", dataPath, progName, QRUN_TAIL);
	aShift.pfilnam = aShiftName;
	sprintf(aShift.pfilnam, "%s%s%c", dataPath, progName, QSHIFT_TAIL);
	jClosed.pfilnam = aClosedName;
	sprintf(jClosed.pfilnam, "%s%s%c", dataPath, progName, QCLOSE_TAIL);
	if (INTISAM(ISAM_BUFS, ISAM_FILS, ISAM_SECT) != NO_ERROR)
		ctreeError();
	openIFIL(&aJob);
	openIFIL(&aShift);
	openIFIL(&jClosed);
	atexit(ArchiveCleanUp);
	StatusPop();
	}


void ArchiveSave(void) {
	if (!host)
		return;
	ArchiveCleanUp();
	ArchiveInit();
	}


void GenerateDo(short item) {
	switch (item) {
		case GenerateMENU_JobMachine:
			printArchJobMachine();
			break;
		case GenerateMENU_JobJob:
			printArchJobJob();
			break;
		case GenerateMENU_JobOperator:
			printArchJobOperator();
			break;
		case GenerateMENU_ShiftMachine:
			printArchShiftMachine();
			break;
		case GenerateMENU_ShiftJob:
			printArchShiftJob();
			break;
		case GenerateMENU_ShiftOperator:
			printArchShiftOperator();
			break;
		}
	}


static void openIFIL(IFIL *name) {
	switch (OPNIFIL(name)) {
		case NO_ERROR:
			break;
		case FNOP_ERR:
			if (CREIFIL(name))
				ctreeError();
			break;
		case FCRP_ERR:
			if (RBLIFIL(name) || OPNIFIL(name))
				ctreeError();
			break;
		default:
			ctreeError();
			break;
		}
	}


static void ctreeError(void) {
	Error(Err_Fatal, ErrorGet(ArchiveERROR, ArchiveERROR_Ctree), isam_err, isam_fil);
	}


Boolean existArchJob(Byte adr, DWord jobN) {
	TEXT	tj[QRUN_SIZE];

	memcpy(tj+QRUN_LOC_ADR, &adr, QRUN_SIZE_ADR);
	memcpy(tj+QRUN_LOC_JOBN, &jobN, QRUN_SIZE_JOBN);
	TFRMKEY(JOB_KEY_NUM, tj);
	return (EQLREC(JOB_KEY_NUM, tj, cur_job_text) == NO_ERROR ? true : false);
	}


void saveArchJob(QueryJobStruct *qj) {
	TEXT	tj[QRUN_SIZE];

	if (!host)
		return;
	if ((qj->adr != QUERY_VIRGIN) && (qj->totalCnt > 0)) {
		memcpy(new_job_text, qj, QRUN_SIZE);
		memcpy(tj+QRUN_LOC_ADR, &(qj->adr), QRUN_SIZE_ADR);
		memcpy(tj+QRUN_LOC_JOBN, &(qj->jobN), QRUN_SIZE_JOBN);
		TFRMKEY(JOB_KEY_NUM, tj);
		switch (EQLREC(JOB_KEY_NUM, tj, cur_job_text)) {
			case NO_ERROR:
				if (RWTREC(JOB_DATA_NUM, new_job_text))
					ctreeError();
				break;
			case INOT_ERR:
				if (ADDREC(JOB_DATA_NUM, new_job_text))
					ctreeError();
				break;
			default:
				ctreeError();
				break;
			}
		}
	}


Boolean existArchShift(Byte adr, DWord jobN, Byte shiftN) {
	TEXT	ts[QSHIFT_SIZE];

	memcpy(ts+QSHIFT_LOC_ADR, &adr, QSHIFT_SIZE_ADR);
	memcpy(ts+QSHIFT_LOC_JOBN, &jobN, QSHIFT_SIZE_JOBN);
	memcpy(ts+QSHIFT_LOC_SHIFTN, &shiftN, QSHIFT_SIZE_SHIFTN);
	TFRMKEY(SHIFT_KEY_NUM, ts);
	return (EQLREC(SHIFT_KEY_NUM, ts, cur_shift_text) == NO_ERROR ? true : false);
	}


void saveArchShift(QueryShiftStruct *qs) {
	TEXT	ts[QSHIFT_SIZE];

	if (!host)
		return;
	if ((qs->adr != QUERY_VIRGIN) && (qs->shfTotal > 0)) {
		memcpy(new_shift_text, qs, QSHIFT_SIZE);
		memcpy(ts+QSHIFT_LOC_ADR, &(qs->adr), QSHIFT_SIZE_ADR);
		memcpy(ts+QSHIFT_LOC_JOBN, &(qs->jobN), QSHIFT_SIZE_JOBN);
		memcpy(ts+QSHIFT_LOC_SHIFTN, &(qs->shiftN), QSHIFT_SIZE_SHIFTN);
		TFRMKEY(SHIFT_KEY_NUM, ts);
		switch (EQLREC(SHIFT_KEY_NUM, ts, cur_shift_text)) {
			case NO_ERROR:
				if (RWTREC(SHIFT_DATA_NUM, new_shift_text))
					ctreeError();
				break;
			case INOT_ERR:
				if (ADDREC(SHIFT_DATA_NUM, new_shift_text))
					ctreeError();
				break;
			default:
				ctreeError();
				break;
			}
		}
	}


Boolean existClosedJob(DWord jobNumber) {
	TEXT	key[JCLOSED_SIZE];

	memcpy(key, &jobNumber, sizeof(jobNumber));
	TFRMKEY(JCLOSED_KEY_NUM, key);
	return (EQLREC(JCLOSED_KEY_NUM, key, cur_jclosed_text) == NO_ERROR ? true : false);
	}


void saveClosedJob(DWord jobNumber) {
	ClosedJobStruct	cj;
	TEXT		key[JCLOSED_SIZE];

	cj.delFlag = '\0';
	cj.jobN = jobNumber;
	memcpy(new_jclosed_text, &cj, JCLOSED_SIZE);
	memcpy(key, &cj.jobN, JCLOSED_SIZE_JOBN);
	TFRMKEY(JCLOSED_KEY_NUM, key);
	switch (EQLREC(JCLOSED_KEY_NUM, key, cur_jclosed_text)) {
		case NO_ERROR:
			break;
		case INOT_ERR:
			if (ADDREC(JCLOSED_DATA_NUM, new_jclosed_text))
				ctreeError();
			break;
		default:
			ctreeError();
			break;
		}
	}


char *a_adr(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		sprintf(aMsgStr, "%*u", MACH_CNTR_LEN, qj->adr);
		}
	else {
		qs = (QueryShiftStruct *)q;
		sprintf(aMsgStr, "%*u", MACH_CNTR_LEN, qs->adr);
		}
	return (aMsgStr);
	}


char *a_jobNumber(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		sprintf(aMsgStr, "%*lu", JOB_NUM_LEN, qj->jobN);
		}
	else {
		qs = (QueryShiftStruct *)q;
		sprintf(aMsgStr, "%*lu", JOB_NUM_LEN, qs->jobN);
		}
	return (aMsgStr);
	}


char *a_opNumber(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		sprintf(aMsgStr, "%4u", qj->operN);
		}
	else {
		qs = (QueryShiftStruct *)q;
		sprintf(aMsgStr, "%4u", qs->operN);
		}
	return (aMsgStr);
	}


char *a_shiftNumber(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		sprintf(aMsgStr, "%u", qj->shiftN);
		}
	else {
		qs = (QueryShiftStruct *)q;
		sprintf(aMsgStr, "%u", qs->shiftN);
		}
	return (aMsgStr);
	}


char *a_totalCount(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		sprintf(aMsgStr, "%8lu", qj->totalCnt);
		}
	else {
		qs = (QueryShiftStruct *)q;
		sprintf(aMsgStr, "%8lu", qs->shfTotal);
		}
	return (aMsgStr);
	}


char *a_averageRate(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;
	float			totTime;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		totTime = IULTime2Hrs(IULTimeAdd(qj->prodTime, qj->idleTime));
		if (totTime > 0.0)
			sprintf(aMsgStr, "%8lu", (DWord)((float)qj->totalCnt/totTime));
		else
			sprintf(aMsgStr, "%8lu", 0);
		}
	else {
		qs = (QueryShiftStruct *)q;
		totTime = IULTime2Hrs(IULTimeAdd(qs->sPrdTime, qs->sIdlTime));
		if (totTime > 0.0)
			sprintf(aMsgStr, "%8lu", (DWord)((float)qs->shfTotal/totTime));
		else
			sprintf(aMsgStr, "%8lu", 0);
		}
	return (aMsgStr);
	}


char *a_runTime(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		IULTime2Str(qj->prodTime, aMsgStr);
		}
	else {
		qs = (QueryShiftStruct *)q;
		IULTime2Str(qs->sPrdTime, aMsgStr);
		}
	*(aMsgStr+7) = '\0';
	return (aMsgStr);
	}


char *a_idleTime(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		IULTime2Str(qj->idleTime, aMsgStr);
		}
	else {
		qs = (QueryShiftStruct *)q;
		IULTime2Str(qs->sIdlTime, aMsgStr);
		}
	*(aMsgStr+7) = '\0';
	return (aMsgStr);
	}


char *a_setupTime(void *q, QTYPE type) {
	QueryJobStruct		*qj;
	QueryShiftStruct	*qs;

	if (type == QRUN) {
		qj = (QueryJobStruct *)q;
		IULTime2Str(qj->setUpTim, aMsgStr);
		}
	else {
		qs = (QueryShiftStruct *)q;
		IULTime2Str(qs->sSupTime, aMsgStr);
		}
	*(aMsgStr+7) = '\0';
	return (aMsgStr);
	}


static void printArchJobMachine(void) {
	QueryJobStruct	qj, *tqj;
	Boolean		ok;
	int		i;
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_JobByMachine);
	for (i=0; i<MAX_MACHINES; i++) {
		tqj = getJobQuery(i);
		if (tqj != NULL)
			saveArchJob(tqj);
		}
	ok = PrinterOpen();
	s = FormGetFirst(ArchiveFORM_Machine);
	if (ok && s) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		ok = PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		if (FRSREC(JOB_ADR_NUM, cur_job_text) == NO_ERROR) {
			do {
				memcpy(&qj, cur_job_text, QRUN_SIZE);
				ok = PrinterWrite(a_adr(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_jobNumber(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_opNumber(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_shiftNumber(&qj, QRUN));
				PrinterWrite("     ");
				PrinterWrite(a_totalCount(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_averageRate(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_runTime(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_idleTime(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_setupTime(&qj, QRUN));
				PrinterWrite("\n");
				} while ((ok) && (NXTREC(JOB_ADR_NUM, cur_job_text) == NO_ERROR));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	StatusPop();
	}


static void printArchJobJob(void) {
	QueryJobStruct	qj, *tqj;
	Boolean		ok;
	int		i;
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_JobByJob);
	for (i=0; i<MAX_MACHINES; i++) {
		tqj = getJobQuery(i);
		if (tqj != NULL)
			saveArchJob(tqj);
		}
	ok = PrinterOpen();
	s = FormGetFirst(ArchiveFORM_Job);
	if (ok && s) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		ok = PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		if (FRSREC(JOB_JOBN_NUM, cur_job_text) == NO_ERROR) {
			do {
				memcpy(&qj, cur_job_text, QRUN_SIZE);
				ok = PrinterWrite(a_adr(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_jobNumber(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_opNumber(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_shiftNumber(&qj, QRUN));
				PrinterWrite("     ");
				PrinterWrite(a_totalCount(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_averageRate(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_runTime(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_idleTime(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_setupTime(&qj, QRUN));
				PrinterWrite("\n");
				} while ((ok) && (NXTREC(JOB_JOBN_NUM, cur_job_text) == NO_ERROR));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	StatusPop();
	}


static void printArchJobOperator(void) {
	QueryJobStruct	qj, *tqj;
	Boolean		ok;
	int		i;
	char		*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_JobByOperator);
	for (i=0; i<MAX_MACHINES; i++) {
		tqj = getJobQuery(i);
		if (tqj != NULL)
			saveArchJob(tqj);
		}
	ok = PrinterOpen();
	s = FormGetFirst(ArchiveFORM_Operator);
	if (ok && s) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		ok = PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		if (FRSREC(JOB_OPERN_NUM, cur_job_text) == NO_ERROR) {
			do {
				memcpy(&qj, cur_job_text, QRUN_SIZE);
				ok = PrinterWrite(a_adr(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_jobNumber(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_opNumber(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_shiftNumber(&qj, QRUN));
				PrinterWrite("     ");
				PrinterWrite(a_totalCount(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_averageRate(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_runTime(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_idleTime(&qj, QRUN));
				PrinterWrite("  ");
				PrinterWrite(a_setupTime(&qj, QRUN));
				PrinterWrite("\n");
				} while ((ok) && (NXTREC(JOB_OPERN_NUM, cur_job_text) == NO_ERROR));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	StatusPop();
	}


static void printArchShiftMachine(void) {
	QueryShiftStruct	qs, *tqs;
	Boolean			ok;
	int			i;
	char			*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ShiftByMachine);
	for (i=0; i<MAX_MACHINES; i++) {
		tqs = getShiftQuery(i);
		if (tqs != NULL)
			saveArchShift(tqs);
		}
	ok = PrinterOpen();
	s = FormGetFirst(ArchiveFORM_ShiftMachine);
	if (ok && s) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		ok = PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		if (FRSREC(SHIFT_ADR_NUM, cur_shift_text) == NO_ERROR) {
			do {
				memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
				ok = PrinterWrite(a_adr(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_jobNumber(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_opNumber(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_shiftNumber(&qs, QSHIFT));
				PrinterWrite("     ");
				PrinterWrite(a_totalCount(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_averageRate(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_runTime(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_idleTime(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_setupTime(&qs, QSHIFT));
				PrinterWrite("\n");
				} while ((ok) && (NXTREC(SHIFT_ADR_NUM, cur_shift_text) == NO_ERROR));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	StatusPop();
	}


static void printArchShiftJob(void) {
	QueryShiftStruct	qs, *tqs;
	Boolean			ok;
	int			i;
	char			*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ShiftByJob);
	for (i=0; i<MAX_MACHINES; i++) {
		tqs = getShiftQuery(i);
		if (tqs != NULL)
			saveArchShift(tqs);
		}
	ok = PrinterOpen();
	s = FormGetFirst(ArchiveFORM_ShiftJob);
	if (ok && s) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		ok = PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		if (FRSREC(SHIFT_JOBN_NUM, cur_shift_text) == NO_ERROR) {
			do {
				memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
				ok = PrinterWrite(a_adr(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_jobNumber(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_opNumber(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_shiftNumber(&qs, QSHIFT));
				PrinterWrite("     ");
				PrinterWrite(a_totalCount(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_averageRate(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_runTime(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_idleTime(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_setupTime(&qs, QSHIFT));
				PrinterWrite("\n");
				} while ((ok) && (NXTREC(SHIFT_JOBN_NUM, cur_shift_text) == NO_ERROR));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	StatusPop();
	}


static void printArchShiftOperator(void) {
	QueryShiftStruct	qs, *tqs;
	Boolean			ok;
	int			i;
	char			*s, date[IU_DATE_LEN+1], time[IU_TIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ShiftByOperator);
	for (i=0; i<MAX_MACHINES; i++) {
		tqs = getShiftQuery(i);
		if (tqs != NULL)
			saveArchShift(tqs);
		}
	ok = PrinterOpen();
	s = FormGetFirst(ArchiveFORM_ShiftOperator);
	if (ok && s) {
		IUIDate2Str(SysDate(), date);
		IUITime2Str(SysTime(), time);
		ok = PrinterWrite(s, date, time);
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		PrinterWrite(FormGetNext());
		if (FRSREC(SHIFT_OPERN_NUM, cur_shift_text) == NO_ERROR) {
			do {
				memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
				ok = PrinterWrite(a_adr(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_jobNumber(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_opNumber(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_shiftNumber(&qs, QSHIFT));
				PrinterWrite("     ");
				PrinterWrite(a_totalCount(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_averageRate(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_runTime(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_idleTime(&qs, QSHIFT));
				PrinterWrite("  ");
				PrinterWrite(a_setupTime(&qs, QSHIFT));
				PrinterWrite("\n");
				} while ((ok) && (NXTREC(SHIFT_OPERN_NUM, cur_shift_text) == NO_ERROR));
			}
		PrinterWrite("\f");
		PrinterClose();
		}
	StatusPop();
	}


void deleteArchives(void) {
	QueryJobStruct		qj;
	short			vsnum, attr;
	Rect			r;
	Boolean			done;
	COUNT			err;
	DWord			jobNumber;
	QueryShiftStruct	qs;
	EventRecord		myEvent;

	if (FRSREC(JOB_JOBN_NUM, cur_job_text) != NO_ERROR)
		Error(Err_Warning, ErrorGet(ArchiveERROR, ArchiveERROR_NotFound));
	else {
		HelpPush(ReportHELP_Delete, MenuItemGet(ReportMENU, ReportMENU_Delete));
		vsnum = VirtualGetNew(ArchiveWINDOW_Delete, &r);
		attr = ColorGet(DialogCOLOR, _COLOR_High);
		done = false;
		while (!done) {
			memcpy(&qj, cur_job_text, QRUN_SIZE);
			VirtualWrite(vsnum, 0, 24, GIANT, a_adr(&qj, QRUN), attr);
			VirtualWrite(vsnum, 1, 18, GIANT, a_jobNumber(&qj, QRUN), attr);
			VirtualWrite(vsnum, 2, 22, GIANT, a_opNumber(&qj, QRUN), attr);
			VirtualWrite(vsnum, 3, 25, GIANT, a_shiftNumber(&qj, QRUN), attr);
			VirtualWrite(vsnum, 4, 18, GIANT, a_totalCount(&qj, QRUN), attr);
			VirtualWrite(vsnum, 5, 18, GIANT, a_averageRate(&qj, QRUN), attr);
			VirtualWrite(vsnum, 6, 19, GIANT, a_runTime(&qj, QRUN), attr);
			VirtualWrite(vsnum, 7, 19, GIANT, a_idleTime(&qj, QRUN), attr);
			VirtualWrite(vsnum, 8, 19, GIANT, a_setupTime(&qj, QRUN), attr);
			while (!EventGetNext(EVT_EVERY, &myEvent))
				;
			switch (myEvent.what) {
				case EVT_KEY:
					switch (myEvent.message) {
						case K_ESC:
							done = true;
							break;
						case K_HOME:
							FRSREC(JOB_JOBN_NUM, cur_job_text);
							break;
						case K_PGUP:
						case K_UP:
							PRVREC(JOB_JOBN_NUM, cur_job_text);
							break;
						case K_SPACE:
							if (NXTREC(JOB_JOBN_NUM, cur_job_text) != NO_ERROR)
								FRSREC(JOB_JOBN_NUM, cur_job_text);
							break;
						case K_PGDN:
						case K_DOWN:
							NXTREC(JOB_JOBN_NUM, cur_job_text);
							break;
						case K_END:
							LSTREC(JOB_JOBN_NUM, cur_job_text);
							break;
						case K_CTRLD:
							StatusPush(ArchiveSTATUS, ArchiveSTATUS_Deleting);
							jobNumber = qj.jobN;
							err = FRSREC(JOB_DATA_NUM, cur_job_text);
							while (err == NO_ERROR) {
								memcpy(&qj, cur_job_text, QRUN_SIZE);
								if (qj.jobN == jobNumber)
									err = DELREC(JOB_DATA_NUM);
								err = NXTREC(JOB_DATA_NUM, cur_job_text);
								}
							err = FRSREC(SHIFT_DATA_NUM, cur_shift_text);
							while (err == NO_ERROR) {
								memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
								if (qs.jobN == jobNumber)
									DELREC(SHIFT_DATA_NUM);
								err = NXTREC(SHIFT_DATA_NUM, cur_shift_text);
								}
							if (FRSREC(JOB_JOBN_NUM, cur_job_text) != NO_ERROR)
								done = true;
							StatusPop();
							break;
						}
					break;
				}
			}
		VirtualDispose(vsnum);
		HelpPop();
		}
	}


void transferArchJob(char *name, DWord jobNumber) {
	COUNT		err;
	QueryJobStruct	qj, *tqj;
	short		i;

	for (i=0; i<MAX_MACHINES; i++) {
		tqj = getJobQuery(i);
		if (tqj != NULL)
			saveArchJob(tqj);
		}
	haJob.pfilnam = (char *) r_malloc(strlen(name)+2);
	sprintf(haJob.pfilnam, "%s%c", name, QRUN_TAIL);
	openIFIL(&haJob);
	err = FRSREC(HJOB_DATA_NUM, cur_job_text);
	while (err == NO_ERROR) {
		memcpy(&qj, cur_job_text, QRUN_SIZE);
		if (qj.jobN == jobNumber)
			DELREC(HJOB_DATA_NUM);
		err = NXTREC(HJOB_DATA_NUM, cur_job_text);
		}
	err = FRSREC(JOB_DATA_NUM, cur_job_text);
	while (err == NO_ERROR) {
		memcpy(&qj, cur_job_text, QRUN_SIZE);
		if (qj.jobN == jobNumber) {
			if (ADDREC(HJOB_DATA_NUM, cur_job_text))
				ctreeError();
			DELREC(JOB_DATA_NUM);
			}
		err = NXTREC(JOB_DATA_NUM, cur_job_text);
		}
	CLIFIL(&haJob);
	r_free(haJob.pfilnam);
	}


void transferArchShift(char *name, DWord jobNumber) {
	COUNT			err;
	QueryShiftStruct	qs, *tqs;
	short			i;

	for (i=0; i<MAX_MACHINES; i++) {
		tqs = getShiftQuery(i);
		if (tqs != NULL)
			saveArchShift(tqs);
		}
	haShift.pfilnam = (char *) r_malloc(strlen(name)+2);
	sprintf(haShift.pfilnam, "%s%c", name, QSHIFT_TAIL);
	openIFIL(&haShift);
	err = FRSREC(HSHIFT_DATA_NUM, cur_shift_text);
	while (err == NO_ERROR) {
		memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
		if (qs.jobN == jobNumber)
			DELREC(HSHIFT_DATA_NUM);
		err = NXTREC(HSHIFT_DATA_NUM, cur_shift_text);
		}
	err = FRSREC(SHIFT_DATA_NUM, cur_shift_text);
	while (err == NO_ERROR) {
		memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
		if (qs.jobN == jobNumber) {
			if (ADDREC(HSHIFT_DATA_NUM, cur_shift_text))
				ctreeError();
			DELREC(SHIFT_DATA_NUM);
			}
		err = NXTREC(SHIFT_DATA_NUM, cur_shift_text);
		}
	CLIFIL(&haShift);
	r_free(haShift.pfilnam);
	}


void exportOpenArchJobs(char *name) {
	COUNT		err;
	QueryJobStruct	qj, *tqj;
	short		i;
	FILE		*fp;
	char		date[IU_DATE_LEN+1], time[IU_LTIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ExportOpenJob);
	for (i=0; i<MAX_MACHINES; i++) {
		tqj = getJobQuery(i);
		if (tqj != NULL)
			saveArchJob(tqj);
		}
	if ((fp = fopen(name, "wb")) == NULL) {
		printf("\a");
		return;
		}
	err = FRSREC(JOB_DATA_NUM, cur_job_text);
	while (err == NO_ERROR) {
		memcpy(&qj, cur_job_text, QRUN_SIZE);
		fprintf(fp, "%u%c", qj.adr, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.jobN, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.shiftN, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.operN, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.output, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.status, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.factor, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.timesIdl, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.totalCnt, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.batchCnt, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.downCnt, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.setUpCnt, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.preset, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.feedTime, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.kickTime, data.prefs.exportField);
		IULTime2Str(qj.setUpTim, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.idleTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.prodTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.totlTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.timeDone, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.pctDone, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.cntDone, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.jPreset, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.cRate, data.prefs.exportField);
		IUITime2Str(qj.jStrTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IUIDate2Str(qj.jStrDate, date);
		fprintf(fp, "%s%c", date, data.prefs.exportField);
		IUITime2Str(qj.jEndTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IUIDate2Str(qj.jEndDate, date);
		fprintf(fp, "%s%c", date, data.prefs.exportRecord);
		err = NXTREC(JOB_DATA_NUM, cur_job_text);
		}
	fclose(fp);
	StatusPop();
	}


void exportOpenArchShifts(char *name) {
	COUNT			err;
	QueryShiftStruct	qs, *tqs;
	short			i;
	FILE			*fp;
	char			date[IU_DATE_LEN+1], time[IU_LTIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ExportOpenShift);
	for (i=0; i<MAX_MACHINES; i++) {
		tqs = getShiftQuery(i);
		if (tqs != NULL)
			saveArchShift(tqs);
		}
	if ((fp = fopen(name, "wb")) == NULL) {
		printf("\a");
		return;
		}
	err = FRSREC(SHIFT_DATA_NUM, cur_shift_text);
	while (err == NO_ERROR) {
		memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
		fprintf(fp, "%u%c", qs.adr, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.jobN, data.prefs.exportField);
		fprintf(fp, "%u%c", qs.shiftN, data.prefs.exportField);
		fprintf(fp, "%u%c", qs.operN, data.prefs.exportField);
		IUITime2Str(qs.sStrTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IUIDate2Str(qs.sStrDate, date);
		fprintf(fp, "%s%c", date, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.shfTotal, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.sSetuCnt, data.prefs.exportField);
		fprintf(fp, "%u%c", qs.sBatch, data.prefs.exportField);
		IULTime2Str(qs.sSupTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qs.sIdlTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qs.sPrdTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qs.sTotTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.sCurRate, data.prefs.exportRecord);
		err = NXTREC(SHIFT_DATA_NUM, cur_shift_text);
		}
	fclose(fp);
	StatusPop();
	}


void exportClosedArchJobs(char *nameFrom, char *nameTo) {
	COUNT		err;
	QueryJobStruct	qj, *tqj;
	short		i;
	FILE		*fp;
	char		date[IU_DATE_LEN+1], time[IU_LTIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ExportClosedJob);
	for (i=0; i<MAX_MACHINES; i++) {
		tqj = getJobQuery(i);
		if (tqj != NULL)
			saveArchJob(tqj);
		}
	if ((fp = fopen(nameTo, "wb")) == NULL) {
		printf("\a");
		return;
		}
	haJob.pfilnam = (char *) r_malloc(strlen(nameFrom)+2);
	sprintf(haJob.pfilnam, "%s%c", nameFrom, QRUN_TAIL);
	openIFIL(&haJob);
	err = FRSREC(HJOB_DATA_NUM, cur_job_text);
	while (err == NO_ERROR) {
		memcpy(&qj, cur_job_text, QRUN_SIZE);
		fprintf(fp, "%u%c", qj.adr, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.jobN, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.shiftN, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.operN, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.output, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.status, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.factor, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.timesIdl, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.totalCnt, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.batchCnt, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.downCnt, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.setUpCnt, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.preset, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.feedTime, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.kickTime, data.prefs.exportField);
		IULTime2Str(qj.setUpTim, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.idleTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.prodTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.totlTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qj.timeDone, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		fprintf(fp, "%u%c", qj.pctDone, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.cntDone, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.jPreset, data.prefs.exportField);
		fprintf(fp, "%lu%c", qj.cRate, data.prefs.exportField);
		IUITime2Str(qj.jStrTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IUIDate2Str(qj.jStrDate, date);
		fprintf(fp, "%s%c", date, data.prefs.exportField);
		IUITime2Str(qj.jEndTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IUIDate2Str(qj.jEndDate, date);
		fprintf(fp, "%s%c", date, data.prefs.exportRecord);
		err = NXTREC(HJOB_DATA_NUM, cur_job_text);
		}
	fclose(fp);
	CLIFIL(&haJob);
	r_free(haJob.pfilnam);
	StatusPop();
	}


void exportClosedArchShifts(char *nameFrom, char *nameTo) {
	COUNT			err;
	QueryShiftStruct	qs, *tqs;
	short			i;
	FILE			*fp;
	char			date[IU_DATE_LEN+1], time[IU_LTIME_LEN+1];

	StatusPush(ArchiveSTATUS, ArchiveSTATUS_ExportClosedShift);
	for (i=0; i<MAX_MACHINES; i++) {
		tqs = getShiftQuery(i);
		if (tqs != NULL)
			saveArchShift(tqs);
		}
	if ((fp = fopen(nameTo, "wb")) == NULL) {
		printf("\a");
		return;
		}
	haShift.pfilnam = (char *) r_malloc(strlen(nameFrom)+2);
	sprintf(haShift.pfilnam, "%s%c", nameFrom, QSHIFT_TAIL);
	openIFIL(&haShift);
	err = FRSREC(HSHIFT_DATA_NUM, cur_shift_text);
	while (err == NO_ERROR) {
		memcpy(&qs, cur_shift_text, QSHIFT_SIZE);
		fprintf(fp, "%u%c", qs.adr, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.jobN, data.prefs.exportField);
		fprintf(fp, "%u%c", qs.shiftN, data.prefs.exportField);
		fprintf(fp, "%u%c", qs.operN, data.prefs.exportField);
		IUITime2Str(qs.sStrTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IUIDate2Str(qs.sStrDate, date);
		fprintf(fp, "%s%c", date, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.shfTotal, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.sSetuCnt, data.prefs.exportField);
		fprintf(fp, "%u%c", qs.sBatch, data.prefs.exportField);
		IULTime2Str(qs.sSupTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qs.sIdlTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qs.sPrdTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		IULTime2Str(qs.sTotTime, time);
		fprintf(fp, "%s%c", time, data.prefs.exportField);
		fprintf(fp, "%lu%c", qs.sCurRate, data.prefs.exportRecord);
		err = NXTREC(HSHIFT_DATA_NUM, cur_shift_text);
		}
	fclose(fp);
	CLIFIL(&haShift);
	r_free(haShift.pfilnam);
	StatusPop();
	}

