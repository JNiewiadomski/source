// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#include <dir.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\coord.h>
//#include <common\asynch.h>
#include <common\colors.h>
#include <common\getargs.h>
#include <common\jiffy.h>
#include <common\keyboard.h>

#include <seashell\dialog.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\iu.h>
#include <seashell\menu.h>
#include <seashell\printer.h>
#include <seashell\resource.h>
#include <seashell\string.h>
#include <seashell\status.h>
#include <seashell\window.h>

#include <common\system.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "pcs100.rh"
#include "pcs100.h"
#include "screen.h"
#include "graph.h"
#include "mach.h"
#include "job.h"
#include "query.h"
#include "archiv.h"
#include "schedule.h"

#define OFF		0
#define ON		1

#define UPDOWNARROW	0x17
#define UPARROW		0x18
#define DOWNARROW	0x19

#define Q_EXT	".QRY"

#define QUERY_OUTTYPE_KICKER	0x0F
#define QUERY_OUTTYPE_FEED	0xF0
#define QUERY_OUTTYPE_NONE	0xFF

#define QUERY_RXLEN		256
#define QUERY_TXLEN		MIN_BUF_SIZE
#define QUERY_MODE		(ASIN | BINARY | WIDETRACKRX)

#define DEFAULT_HI_PRIORITY	75L
#define DEFAULT_LO_PRIORITY	150L

#define QARCH_JOB_CUR		0x30
#define QARCH_JOB_LAST		0x4E
#define QARCH_JOB_PREV(x)	((x) + 2)
#define IS_QARCH_JOB_PREV(x)	((x) < QARCH_JOB_LAST)
#define IS_QARCH_JOB(x)		((x)>=QARCH_JOB_CUR && (x)<=QARCH_JOB_LAST)
#define QARCH_JOB_LEN		81

#define QARCH_SHIFT_CUR		0x50
#define QARCH_SHIFT_LAST	0x6E
#define QARCH_SHIFT_PREV(x)	((x) + 2)
#define IS_QARCH_SHIFT_PREV(x)	((x) < QARCH_SHIFT_LAST)
#define IS_QARCH_SHIFT(x)	((x)>=QARCH_SHIFT_CUR && (x)<=QARCH_SHIFT_LAST)
#define QARCH_SHIFT_LEN		42

#define Q_GET	0x00
#define Q_MAKE	0x01
#define Q_WAIT	0x02

#define Q_PERR_FORM	"(%2d, 0x%02X, %2d):%6s: adr=%2d, class=0x%02X, type=0x%02X, len=%2d"
#define Q_TERR_FORM	"(%2d, 0x%02X, %2d):%6s: adr=%2d, class=0x%02X, type=0x%02X, len=%2d\n"

struct QueryPortStruct {
	short	num;		// number of ports found
	int	com;		// current com port
	Word	on	: 1;	// is counter querying enabled ?
	Word	state	: 2;	// current query state
	Word	test	: 1;	// should we do test program?
	Word	wait	: 1;	// should we wait while testing?
	Jiff	waitTime;	// time allotted for response
	Jiff	maxTime;	// maximum time to receive complete response
	Jiff	stampTime;	// time stamp delay times
	Byte	counter;	// querying counter address
	Byte	len;		// length of response
	Byte	class_type;	// querying class
	Byte	type;		// querying type
	};


static QueryPortStruct	Qport;
static char		qMsgStr[80];
static char		qryName[MAXPATH];
static short		qReqType[MAX_MACHINES];
static Word		qOff[MAX_MACHINES];	// number of times offline
static Jiff		qRetry[MAX_MACHINES];	// time counter must respond before going offline
static Byte		qEStat[MAX_MACHINES];	// error tolerance statistics
static QueryJobStruct	*queryJob[MAX_MACHINES];
static QueryShiftStruct	*queryShift[MAX_MACHINES];
static short		offlineScrn, offlineHeight;
static Boolean		qInit=false;
static Schedule		sched(DEFAULT_HI_PRIORITY, DEFAULT_LO_PRIORITY);

static void	QueryCleanUp(void);
static void	QueryMenuInit(void);


static char	calcCheckSum(char *s, short len);
static Boolean	changeErrorStatus(void);
static Boolean	inError(Byte adr, char *ib, short len, char *sb);
static void	doUpdateScreen(short counter);
static void	enableOfflineTrace(Boolean on);
static Boolean	isDisplayed(short counter);
static char	*getQ(char *ib, char *ob, short num);
static void	initQueryJobRecord(short adr);
static void	initQueryShiftRecord(short adr);
static void	loadQueryData(void);
static void	makeQ_GET(void);
static Boolean	makeQuery(void);
static void	modifyQueryTiming(void);
static Boolean	retrieveQuery(Boolean *boost);
static Boolean	stuffQueryJobRecord(char *ib, QueryJobStruct *qj, char *sb);
static Boolean	stuffQueryShiftRecord(char *ib, QueryShiftStruct *qs, char *sb);
static void	uninstallQuery(void);
static void	updateOfflineTrace(void);


// ************************************************************
// * Temporary function definitions until C_Asynch cleaned up *
// ************************************************************

// The base port address of the UART for each of the serial devices is
// maintained in the ROM BIOS communications area beginning at UART_PTR.

#define UART_PTR	0x0400		// beginning of UART pointer table

// The UART has 10 registers accessible through 7 port addresses.  Here are
// their addresses relative to base.  Note that the Baud rate registers (DLL)
// and (DLM) are active only when the Divisor Latch Access Bit (DLAB) is set.

#define UART_LSR	5		// (R)   line status register
#define UART_SCR	7		// (R/W) scratch register (AT/PS2 only)

// Line Status Register bit masks

#define LSR_THRE	0x20		// transmitter holding register empty
#define LSR_TSRE	0x40		// transmitter shift register empty

// Returns base address of com port.

#define ASYNCH_BASE(port)	(peek(0, UART_PTR + (2 * port)))


// Return true if transmitter is empty

static void AsynchWaitTXEmpty(int port) {
	while ((inportb(ASYNCH_BASE(port)+UART_LSR) & LSR_TSRE) == NULL)
		;
	}


// AsynchRS485Board -- enables/disables the internal rs485 board to receive
// and transmit.

static void AsynchRS485Board(int port, short mode) {
	AsynchWaitTXEmpty(port);
	disable();
	outportb(ASYNCH_BASE(port)+UART_SCR, mode);
	enable();
	}


// ********************
// * Querry Functions *
// ********************

static void QueryCleanUp(void) {
	short	i;

	uninstallQuery();
	QueryDataSave();
	for (i=0; i<MAX_MACHINES; i++) {
		if (queryJob[i] != NULL) {
			r_free(queryJob[i]);
			queryJob[i] = NULL;
			}
		if (queryShift[i] != NULL) {
			r_free(queryShift[i]);
			queryShift[i] = NULL;
			}
		}
	sched.Clear();
	}


void QueryInit(void) {
	sched.SetPriority(data.query.hiPri, data.query.loPri);
	loadQueryData();
	Qport.wait = Qport.test = false;
	Qport.state = Q_GET;
	reinstallQuery(data.query.port);
	if (!qInit) {
		QueryMenuInit();
		atexit(QueryCleanUp);
		qInit = true;
		}
	}


void QueryInitData(void) {
	data.query.port = COM1;
	data.query.minDelay = 8L;
	data.query.maxDelay = 26L;
	data.query.hiPri = DEFAULT_HI_PRIORITY;
	data.query.loPri = DEFAULT_LO_PRIORITY;
	data.query.retries = 6000L;
	data.query.eStat = 5;
	data.query.bell = true;
	data.query.err = false;
	data.query.arch = true;
	data.query.off = false;
	data.query.enable = true;
	data.query.logPrint = false;
	}


static void loadQueryData(void) {
	short	handle, i, num, counter;
	char	sig[sizeof(PCS100_SIG)];

	StatusPush(QuerySTATUS, QuerySTATUS_Loading);
	for (i=0; i<MAX_MACHINES; i++) {
		queryJob[i] = NULL;
		queryShift[i] = NULL;
		}
	for (i=0; i<mchMax; i++)
		createQuery(mach[i]->counter);
	sprintf(qryName, "%s%s%s", dataPath, progName, Q_EXT);
	if ((handle = open(qryName, O_RDONLY | O_BINARY)) != -1) {
		read(handle, sig, sizeof(PCS100_SIG));
		if (memcmp(PCS100_SIG, sig, sizeof(PCS100_SIG)) == 0) {
			read(handle, &num, sizeof(num));
			num = min(num, MAX_MACHINES);
			for (i=0; i<num; i++) {
				read(handle, &counter, sizeof(counter));
				if (counter >= MAX_MACHINES)
					i = MAX_MACHINES;
				else {
					createQuery(counter);
					read(handle, queryJob[counter], sizeof(QueryJobStruct));
					read(handle, queryShift[counter], sizeof(QueryShiftStruct));
					if (host) {
						queryJob[counter]->status = QUERY_STATUS_OFF;
						queryJob[counter]->cRate = 0L;
						queryShift[counter]->sCurRate = 0L;
						}
					}
				}
			}
		else
			MenuErrorPost(ErrorGet(QueryERROR, QueryERROR_Load), true);
		close(handle);
		}
	StatusPop();
	}


void QueryDataSave(void) {
	short	handle, i, counter;

	if (!host)
		return;
	StatusPush(QuerySTATUS, QuerySTATUS_Saving);
	if ((handle = open(qryName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) != -1) {
		write(handle, PCS100_SIG, sizeof(PCS100_SIG));
		write(handle, &mchMax, sizeof(mchMax));
		for (i=0; i<mchMax; i++) {
			counter = mach[i]->counter;
			write(handle, &counter, sizeof(counter));
			write(handle, queryJob[counter], sizeof(QueryJobStruct));
			write(handle, queryShift[counter], sizeof(QueryShiftStruct));
			}
		close(handle);
		}
	else
		MenuErrorPost(ErrorGet(QueryERROR, QueryERROR_Save), true);
	StatusPop();
	}


Boolean QuerySubscribe(void) {
	short		handle;
	struct stat	stat_buf;
	static long	pubSubTime=0;

	if ((handle = open(qryName, O_RDONLY)) == -1)
		return (false);
	fstat(handle, &stat_buf);
	close(handle);
	if (pubSubTime != stat_buf.st_ctime) {
		QueryCleanUp();
		QueryInit();
		pubSubTime = stat_buf.st_ctime;
		}
	return (true);
	}


void createQuery(short adr) {
	if (queryJob[adr] == NULL) {
		queryJob[adr] = (QueryJobStruct *) r_malloc(sizeof(QueryJobStruct));
		queryShift[adr] = (QueryShiftStruct *) r_malloc(sizeof(QueryShiftStruct));
		initQueryJobRecord(adr);
		initQueryShiftRecord(adr);
		qReqType[adr] = QARCH_JOB_PREV(QARCH_JOB_CUR);
		qOff[adr] = 0;
		qRetry[adr] = JiffyTime() + data.query.retries;
		qEStat[adr] = 0;
		if (!sched.Insert(adr, Pri_High))
			Error(Err_Recover, ErrorGet(QueryERROR, QueryERROR_Schedule), adr);
		}
	}


void deleteQuery(short adr) {
	if (queryJob[adr] != NULL) {
		if (mchMax == 0)
			uninstallQuery();
		else
			while (Qport.counter == adr)
				updateQuery();
		if (!sched.Remove(adr))
			Error(Err_Recover, ErrorGet(QueryERROR, QueryERROR_Delete), adr);
		r_free(queryJob[adr]);
		queryJob[adr] = NULL;
		r_free(queryShift[adr]);
		queryShift[adr] = NULL;
		}
	}


QueryJobStruct *getJobQuery(Byte counter) {
	return (queryJob[counter]);
	}


QueryShiftStruct *getShiftQuery(Byte counter) {
	return (queryShift[counter]);
	}


static void initQueryJobRecord(short adr) {
	QueryJobStruct	*ptr;

	ptr = queryJob[adr];
	ptr->adr = QUERY_VIRGIN;
	ptr->jobN = 0;
	ptr->shiftN = 0;
	ptr->operN = 0;
	ptr->output = QUERY_OUTTYPE_NONE;
	ptr->status = QUERY_STATUS_OFF;
	ptr->factor = 0;
	ptr->timesIdl = 0;
	ptr->totalCnt = 0;
	ptr->batchCnt = 0;
	ptr->downCnt = 0;
	ptr->setUpCnt = 0;
	ptr->preset = 0;
	ptr->feedTime = 0;
	ptr->kickTime = 0;
	ptr->setUpTim = 0;
	ptr->idleTime = 0;
	ptr->prodTime = 0;
	ptr->totlTime = 0;
	ptr->timeDone = 0;
	ptr->pctDone = 0;
	ptr->cntDone = 0;
	ptr->jPreset = 0;
	ptr->cRate = 0;
	ptr->jStrTime = 0;
	ptr->jStrDate = 0;
	ptr->jEndTime = 0;
	ptr->jEndDate = 0;
	}


static void initQueryShiftRecord(short adr) {
	QueryShiftStruct *ptr;

	ptr = queryShift[adr];
	ptr->adr = QUERY_VIRGIN;
	ptr->jobN = 0;
	ptr->shiftN = 0;
	ptr->operN = 0;
	ptr->sStrTime = 0;
	ptr->sStrDate = 0;
	ptr->shfTotal = 0;
	ptr->sSetuCnt = 0;
	ptr->sBatch = 0;
	ptr->sSupTime = 0;
	ptr->sIdlTime = 0;
	ptr->sPrdTime = 0;
	ptr->sTotTime = 0;
	ptr->sCurRate = 0;
	}


void resetQuery(void) {
	if (Qport.on) {
		if (mchMax == 0)
			uninstallQuery();
		}
	else
		reinstallQuery(Qport.com);
	}


void reinstallQuery(int port) {
	char	ch;

	uninstallQuery();
	Qport.num = equip.serial;
	Qport.com = (port+1 > Qport.num) ? COM1 : port;
	data.query.port = Qport.com;
	Qport.on = ((Qport.num > 0) && (mchMax>0));
	if (Qport.on) {
		if (asiopen(Qport.com, QUERY_MODE, QUERY_RXLEN, QUERY_TXLEN, 9600, P_S_STICK, 1, 8, OFF, OFF) == ASSUCCESS)
			AsynchRS485Board(Qport.com, ASIN);
		else
			Qport.on = false;
		}
	if (getQueryState())
		ch = host ? UPARROW : UPDOWNARROW;
	else
		ch = DOWNARROW;
	StatusPrint(SCREEN_WIDTH-2, "%c", ch);
	}


static void uninstallQuery(void) {
	if (Qport.on) {
		makeQ_GET();
		AsynchRS485Board(Qport.com, 0);
		asiquit(Qport.com);
		Qport.on = false;
		}
	}


Boolean getQueryState(void) {
	return (data.query.enable && Qport.on ? true : false);
	}


int getQueryPort(void) {
	return (Qport.com);
	}


static void makeQ_GET(void) {
	if (Qport.state == Q_GET)
		return;
	if (!sched.Insert(Qport.counter, Pri_High))
		Error(Err_Fatal, ErrorGet(QueryERROR, QueryERROR_Reschedule), Qport.counter);
	Qport.state = Q_GET;
	}


void updateQuery(void) {
	static Boolean	updating=false;
	Jiff		time;
	EventRecord	myEvent;
	Boolean		boost;

	if (updating)
		return;
	updating = true;
	time = JiffyTime();
	switch (Qport.state) {
		case Q_GET:
			if (data.query.enable && sched.GetNext(&Qport.counter))
				Qport.state = Q_MAKE;
			break;
		case Q_MAKE:
			if (Qport.test) {
				printf("...");
				if (Qport.wait) {
					printf("?");
					while (!EventAvail(EVT_KEY, &myEvent))
						;
					if (myEvent.message == K_ESC)
						Qport.test = false;
					else
						EventGetNext(EVT_KEY, &myEvent);
					printf("\b");
					}
				}
			if (makeQuery()) {
				Qport.stampTime = time;
				Qport.waitTime = time + data.query.minDelay;
				Qport.maxTime = time + data.query.maxDelay;
				Qport.state = Q_WAIT;
				}
			else
				Qport.state = Q_GET;
			break;
		case Q_WAIT:
			if (Qport.stampTime > time || Qport.waitTime < time) {
				if (Qport.test && Qport.wait && getrxcnt(Qport.com)<1) {
					printf("!");
					while (getrxcnt(Qport.com) < 1 && Qport.test)
						if (EventAvail(EVT_KEY, &myEvent)) {
							if (myEvent.message == K_ESC)
								Qport.test = false;
							else
								EventGetNext(EVT_KEY, &myEvent);
							break;
							}
					printf("\b");
					}
				if (retrieveQuery(&boost)) {
					if (!sched.Insert(Qport.counter, (boost | isDisplayed(Qport.counter)) ? Pri_High : Pri_Low))
						Error(Err_Fatal, ErrorGet(QueryERROR, QueryERROR_Reschedule), Qport.counter);
					Qport.state = Q_GET;
					}
				}
			break;
		}
	updating = false;
	}


/*
 * makeQuery transmits a query to the PCS100 counters serially by asserting
 * the RTS line (required for RS485 B&B converter and calling
 * AsynchRS485Board (required for RS485 card in PC's), transmitting the data
 * packet, waiting for all bits to be transmitted, and undoes the RTS and
 * AsynchRS485Board assertions for transmitions.  The query data packet has
 * the following format:
 *	Char	Length	Descrition
 *	-------------------------------------------------------------
 *	0x02	1	Start of Text
 *	Class	1	Class of counter (for now, 'A' which is 0x41)
 *	Address	1	Counter address
 *	Type	1	Type of data packet requested (Job/Shift & archival)
 *	0x0D	1	Terminated with a carriage return
 */

static Boolean makeQuery(void) {
	short	Qadr, pos;
	char	s[3];

	if (!host)
		return (true);
	if ((pos = MachineFind(Qport.counter)) == MCH_NONE)
		return (false);
	Qport.class_type = mach[pos]->class_type;
	sscanf(itoa(Qport.counter, s, 10), "%x", &Qadr);
	if (data.query.arch)
		Qport.type = qReqType[Qport.counter];
	else
		Qport.type = IS_QARCH_JOB(qReqType[Qport.counter]) ? QARCH_JOB_CUR : QARCH_SHIFT_CUR;
	Qport.len = IS_QARCH_JOB(qReqType[Qport.counter]) ? QARCH_JOB_LEN : QARCH_SHIFT_LEN;
	asrts(Qport.com, ON);
	AsynchRS485Board(Qport.com, ASOUT);
	asputc(Qport.com, 0x02);
	asputc(Qport.com, Qport.class_type);
	asputc(Qport.com, Qadr);
	asputc(Qport.com, Qport.type);
	asputc(Qport.com, 0x0D);
	AsynchWaitTXEmpty(Qport.com);
	asrts(Qport.com, OFF);
	AsynchRS485Board(Qport.com, ASIN);
	asiclear(Qport.com, ASIN);	/* clear out receiver buffer */
	asierst(Qport.com);		/* reset alert and error flags */
	if (Qport.test)
		printf("0x%02X  0x%02X  0x%02X  0x%02X  0x%02X\n", 0x02, Qport.class_type, Qadr, Qport.type, 0x0D);
	return (true);
	}


static Boolean isDisplayed(short counter) {
	switch (scrn.cur) {
		case SCREEN_ALL_MCH:
		case SCREEN_ALL_SMCH:
			return (MachineIsAllDisplayed(counter));
		case SCREEN_SNGL_MCH:
		case SCREEN_SNGL_SMCH:
			return (MachineIsSingleDisplayed(counter));
		case SCREEN_ALL_JOB:
		case SCREEN_ALL_SJOB:
			return (isAllJobsDisplayed(queryJob[counter]->jobN));
		case SCREEN_SNGL_JOB:
		case SCREEN_SNGL_SJOB:
			return (isSingleJobDisplayed(queryJob[counter]->jobN));
		case SCREEN_GRAPH:
			return (GraphIsDisplayed(counter));
		default:
			return (false);
		}
	}


static Boolean retrieveQuery(Boolean *boost)
  {
    char             ib[QUERY_RXLEN], sb[QUERY_RXLEN];
    QueryJobStruct   qj;
    QueryShiftStruct qs;
    Jiff             time;
    short            len;

    *boost = false;
    if (getrxcnt(Qport.com) < 1) {
      if (data.query.off) {
        qOff[Qport.counter]++;
        updateOfflineTrace();
        }
      if (Qport.test) {
        printf("Counter %2d is OFF LINE (type=0x%02X, len=%2d)\n", Qport.counter, Qport.type, Qport.len);
        if (data.query.bell)
          ErrorSound();
        }
      if (JiffyElapsed(qRetry[Qport.counter])) {
        queryJob[Qport.counter]->status = QUERY_STATUS_OFF;
        queryJob[Qport.counter]->cRate = 0L;
        queryShift[Qport.counter]->sCurRate = 0L;
        if (qEStat[Qport.counter])
          qEStat[Qport.counter]--;
        }
      else
        *boost = true;
      }
    else {
      time = JiffyTime();
      if ((getrxcnt(Qport.com) < Qport.len) && (Qport.stampTime < time && Qport.maxTime > time))
        return (false);
      _aserror = 1;
      len = asiwgetb(Qport.com, ib, getrxcnt(Qport.com), sb);
      if (len != Qport.len) {
        inError(Qport.counter, ib, len, sb);
	*boost = changeErrorStatus() ? false : true;
        }
      else {
        if (IS_QARCH_JOB(qReqType[Qport.counter])) {
          if (stuffQueryJobRecord(ib, &qj, sb)) {
            if (qReqType[Qport.counter] == QARCH_JOB_CUR) {
              if (queryJob[Qport.counter]->jobN == qj.jobN)
                qReqType[Qport.counter] = QARCH_SHIFT_CUR;
              else {
                saveArchJob(queryJob[Qport.counter]);
                qReqType[Qport.counter] = QARCH_JOB_PREV(qReqType[Qport.counter]);
                }
              memcpy(queryJob[Qport.counter], &qj, QRUN_SIZE);
              if (!existClosedJob(queryJob[Qport.counter]->jobN) || counterStatus(Qport.counter)==QUERY_STATUS_RUN)
                addJob(queryJob[Qport.counter]->jobN);
              }
            else {
              if (IS_QARCH_JOB_PREV(qReqType[Qport.counter]))
                qReqType[Qport.counter] = QARCH_JOB_PREV(qReqType[Qport.counter]);
              else
                qReqType[Qport.counter] = QARCH_SHIFT_PREV(QARCH_SHIFT_CUR);
              saveArchJob(&qj);
              queryJob[Qport.counter]->status = QUERY_STATUS_UPDATING;
              }
            if (qEStat[Qport.counter])
              qEStat[Qport.counter]--;
            }
          else
	    *boost = changeErrorStatus() ? false : true;
          }
        else {
          if (stuffQueryShiftRecord(ib, &qs, sb)) {
            if (qReqType[Qport.counter] == QARCH_SHIFT_CUR) {
              if ((queryShift[Qport.counter]->jobN == qs.jobN) && (queryShift[Qport.counter]->shiftN == qs.shiftN))
                qReqType[Qport.counter] = QARCH_JOB_CUR;
              else {
                saveArchShift(queryShift[Qport.counter]);
                qReqType[Qport.counter] = QARCH_SHIFT_PREV(qReqType[Qport.counter]);
                }
              memcpy(queryShift[Qport.counter], &qs, QSHIFT_SIZE);
              }
            else {
              if (IS_QARCH_SHIFT_PREV(qReqType[Qport.counter]))
                qReqType[Qport.counter] = QARCH_SHIFT_PREV(qReqType[Qport.counter]);
              else
                qReqType[Qport.counter] = QARCH_JOB_CUR;
              saveArchShift(&qs);
              queryJob[Qport.counter]->status = QUERY_STATUS_UPDATING;
              }
            if (qEStat[Qport.counter])
              qEStat[Qport.counter]--;
            }
          else
	    *boost = changeErrorStatus() ? false : true;
          }
        qRetry[Qport.counter] = time + data.query.retries;
        }
      }
    doUpdateScreen(Qport.counter);
    return (true);
    }


static Boolean changeErrorStatus(void) {
	if (qEStat[Qport.counter] < data.query.eStat)
		qEStat[Qport.counter]++;
	if (qEStat[Qport.counter] == data.query.eStat) {
		queryJob[Qport.counter]->status = QUERY_STATUS_ERROR;
		return (true);
		}
	return (false);
	}


static Boolean inError(Byte adr, char *ib, short len, char *sb)
  {
    char  *msg, msgStr[80], ch;
    Word  class_type, type, error;
    short i;

    class_type = (Word)*ib & 0xFF;
    type = (Word)*(ib+2) & 0xFF;
    error = asibstat(Qport.com) & ~(TXEMPTY | RXEMPTY);
    if (error) {
      if (error & LINERR) {
        error = asilstat(Qport.com, STATIC);
        if (error & OVERRUN)
          msg = "OV_RUN";
        else if (error & FRAMERR)
          msg = "FRAME";
        else if (error & PARERR)
          msg = "PARITY";
        else
          msg = "BREAK";
        }
      else if (error & MODCHG)
        msg = NULL;
      else if (error & RXFULL)
        msg = "rFULL";
      else if (error & RXOVFLOW)
        msg = "rOVER";
      else if (error & TXFULL)
        msg = "tFULL";
      else
        msg = "ALERT";
      }
    else if (len != Qport.len)
      msg = "LEN";
    else if (adr != Qport.counter)
      msg = "ADR";
    else if (class_type != Qport.class_type)
      msg = "CLASS";
    else if (type != Qport.type)
      msg = "TYPE";
    else if (*(ib+Qport.len-1) != 0x0D)
      msg = "CR";
    else if (calcCheckSum(ib+3, Qport.len-5) != *(ib+Qport.len-2))
      msg = "SUM";
    else
      msg = NULL;
    if (msg == NULL) {
      if (Qport.test)
        printf("Received GOOD data from counter %2d (type=0x%02X, len=%2d)\n", Qport.counter, Qport.type, Qport.len);
      return (false);
      }
    if (data.query.err) {
      sprintf(msgStr, Q_PERR_FORM, Qport.counter, Qport.type, Qport.len, msg, adr, class_type, type, len);
      MenuErrorPost(msgStr, true);
      }
    if (Qport.test) {
      printf(Q_TERR_FORM, Qport.counter, Qport.type, Qport.len, msg, adr, class_type, type, len);
      if (!data.query.err && data.query.bell)
        ErrorSound();
      for (i=0; i<len; i++) {
        if (sb[i] & OVERRUN_ERR)
          ch = 'o';
        else if (sb[i] & FRAMING_ERR)
          ch = 'f';
        else if (sb[i] & PARITY_ERR)
          ch = 'p';
        else
          ch = ' ';
        printf("%02X%c ", (Byte)ib[i], ch);
        }
      printf("\n\n");
      }
    if (data.query.logPrint && PrinterOpen()) {
      IUIDate2Str(SysDate(), msgStr);
      PrinterWrite("Error printed on %s", msgStr);
      IUITime2Str(SysTime(), msgStr);
      PrinterWrite(" at %s.\n", msgStr);
      PrinterWrite(Q_TERR_FORM, Qport.counter, Qport.type, Qport.len, msg, adr, class_type, type, len);
      for (i=0; i<len; i++) {
        if (sb[i] & OVERRUN_ERR)
          ch = 'o';
        else if (sb[i] & FRAMING_ERR)
          ch = 'f';
        else if (sb[i] & PARITY_ERR)
          ch = 'p';
        else
          ch = ' ';
        PrinterWrite("%02X%c ", (Byte)ib[i], ch);
        }
      PrinterWrite("\n\n");
      PrinterClose();
      }
    return (true);
    }


/*
 * stuffQueryJobRecord accepts a data packet received serially from the
 * PCS100 counter already copied into a buffer pointed to by *ib, copies
 * relevant portions from the buffer into the QueryJobStruct *qj, tests
 * certain bytes to make sure the data is reasonably correct, and calls
 * inError to do more data integrity checks on the received packet.
 * The data packet received from the PCS100 counter has the following
 * format:
 *	Offset	Length	Descrition
 *	-------------------------------------------------------------
 *	0	1	counter class
 *	1	1	counter address
 *	2	1	counter type
 *	3	4	job number
 *	7	2	operator number
 *	9	1	shift number
 *	10	1	output
 *	11	1	status
 *	12	1	factor
 *	13	2	times idle
 *	15	4	total count
 *	19	3	batch count
 *	22	2	down count
 *	24	4	setup count
 *	28	2	preset
 *	30	2	feed time
 *	32	2	kick time
 *	34	4	setup time
 *	38	4	idle time
 *	42	4	production time
 *	46	4	total time
 *	50	4	time done
 *	54	2	percent done
 *	56	4	count done
 *	60	4	job preset
 *	64	3	current rate
 *	67	3	job start time
 *	70	3	job start date
 *	73	3	job end time
 *	76	3	job end date
 *	79	1	check sum
 *	80	1	carriage return (0x0D)
 */

static Boolean stuffQueryJobRecord(char *ib, QueryJobStruct *qj, char *sb) {
	char	ob[9];

	qj->adr		= atoi(getQ(ib+ 1, ob, 1));
	qj->jobN	= atol(getQ(ib+ 3, ob, 4));
	qj->shiftN	= atoi(getQ(ib+ 9, ob, 1));
	qj->operN	= atoi(getQ(ib+ 7, ob, 2));
	qj->output	= *(ib+10);
	qj->status	= *(ib+11);
	qj->factor	= atoi(getQ(ib+12, ob, 1));
	qj->timesIdl	= atoi(getQ(ib+13, ob, 2));
	qj->totalCnt	= atol(getQ(ib+15, ob, 4));
	qj->batchCnt	= atol(getQ(ib+19, ob, 3));
	qj->downCnt	= atoi(getQ(ib+22, ob, 2));
	qj->setUpCnt	= atol(getQ(ib+24, ob, 4));
	qj->preset	= atoi(getQ(ib+28, ob, 2));
	qj->feedTime	= atoi(getQ(ib+30, ob, 2));
	qj->kickTime	= atoi(getQ(ib+32, ob, 2));
	qj->setUpTim	= IUPacked2LTime(getQ(ib+34, ob, 4));
	qj->idleTime	= IUPacked2LTime(getQ(ib+38, ob, 4));
	qj->prodTime	= IUPacked2LTime(getQ(ib+42, ob, 4));
	qj->totlTime	= IUPacked2LTime(getQ(ib+46, ob, 4));
	qj->timeDone	= IUPacked2LTime(getQ(ib+50, ob, 4));
	qj->pctDone	= atoi(getQ(ib+54, ob, 2));
	qj->cntDone	= atol(getQ(ib+56, ob, 4));
	qj->jPreset	= atol(getQ(ib+60, ob, 4));
	qj->cRate	= atol(getQ(ib+64, ob, 3));
	qj->jStrTime	= IUPacked2ITime(getQ(ib+67, ob, 3));
	qj->jStrDate	= IUPacked2IDate(getQ(ib+70, ob, 3));
	qj->jEndTime	= IUPacked2ITime(getQ(ib+73, ob, 3));
	qj->jEndDate	= IUPacked2IDate(getQ(ib+76, ob, 3));
	return (inError(qj->adr, ib, Qport.len, sb) ? false : true);
	}


/*
 * stuffQueryShiftRecord accepts a data packet received serially from the
 * PCS100 counter already copied into a buffer pointed to by *ib, copies
 * relevant portions from the buffer into the QueryShiftStruct *qs, tests
 * certain bytes to make sure the data is reasonably correct, and calls
 * inError to do more data integrity checks on the received packet.
 * The data packet received from the PCS100 counter has the following
 * format:
 *	Offset	Length	Descrition
 *	-------------------------------------------------------------
 *	0	1	counter class
 *	1	1	counter address
 *	2	1	counter type
 *	3	4	job number
 *	7	2	operator number
 *	9	1	shift number
 *	10	3	shift start time
 *	13	3	shift start date
 *	16	4	shift total
 *	20	3	shift setup count
 *	23	2	shift batch
 *	25	3	shift setup time
 *	28	3	shift idle time
 *	31	3	shift production time
 *	34	3	shift total time
 *	37	3	shift current rate
 *	40	1	check sum
 *	41	1	carriage return (0x0D)
 */

static Boolean stuffQueryShiftRecord(char *ib, QueryShiftStruct *qs, char *sb) {
	char	ob[9];

	qs->adr		= atoi(getQ(ib+ 1, ob, 1));
	qs->jobN	= atol(getQ(ib+ 3, ob, 4));
	qs->shiftN	= atoi(getQ(ib+ 9, ob, 1));
	qs->operN	= atoi(getQ(ib+ 7, ob, 2));
	qs->sStrTime	= IUPacked2ITime(getQ(ib+10, ob, 3));
	qs->sStrDate	= IUPacked2IDate(getQ(ib+13, ob, 3));
	qs->shfTotal	= atol(getQ(ib+16, ob, 4));
	qs->sSetuCnt	= atol(getQ(ib+20, ob, 3));
	qs->sBatch	= atoi(getQ(ib+23, ob, 2));
	qs->sSupTime	= IUPacked2ITime(getQ(ib+25, ob, 3));
	qs->sIdlTime	= IUPacked2ITime(getQ(ib+28, ob, 3));
	qs->sPrdTime	= IUPacked2ITime(getQ(ib+31, ob, 3));
	qs->sTotTime	= IUPacked2ITime(getQ(ib+34, ob, 3));
	qs->sCurRate	= atol(getQ(ib+37, ob, 3));
	return (inError(qs->adr, ib, Qport.len, sb) ? false : true);
	}


static char *getQ(char *ib, char *ob, short num) {
	switch (num) {
		case 1:
			sprintf(ob, "%02x", (Byte)*ib);
			break;
		case 2:
			sprintf(ob, "%02x%02x", (Byte)*ib, (Byte)*(ib+1));
			break;
		case 3:
			sprintf(ob, "%02x%02x%02x", (Byte)*ib, (Byte)*(ib+1), (Byte)*(ib+2));
			break;
		case 4:
			sprintf(ob, "%02x%02x%02x%02x", (Byte)*ib, (Byte)*(ib+1), (Byte)*(ib+2), (Byte)*(ib+3));
			break;
		default:
			*ob = '\0';
			break;
		}
	return (ob);
	}


static char calcCheckSum(char *s, short len) {
	char	cSum;

	cSum = 0;
	while (len > 0) {
		cSum ^= *s++;
		len--;
		}
	return (cSum);
	}


static void doUpdateScreen(short counter) {
	switch (scrn.cur) {
		case SCREEN_ALL_MCH:
			MachineAllScreenUpdate(counter);
			break;
		case SCREEN_SNGL_MCH:
			MachineSingleScreenUpdate(counter);
			break;
		case SCREEN_ALL_JOB:
			updateAllJobsScreen(queryJob[counter]->jobN);
			break;
		case SCREEN_SNGL_JOB:
			updateSingleJobScreen(queryJob[counter]->jobN);
			break;
		case SCREEN_ALL_SMCH:
			MachineShiftAllScreenUpdate(counter);
			break;
		case SCREEN_SNGL_SMCH:
			MachineShiftSingleScreenUpdate(counter);
			break;
		case SCREEN_ALL_SJOB:
			updateShiftAllJobsScreen(queryJob[counter]->jobN);
			break;
		case SCREEN_SNGL_SJOB:
			updateShiftSingleJobScreen(queryJob[counter]->jobN);
			break;
		case SCREEN_GRAPH:
			GraphUpdate(counter);
			break;
		}
	}


char *q_jobNumber(short adr, QTYPE type)
  {
    static char qJobNumber[JOB_NUM_LEN+1];

    if (type == QRUN)
      sprintf(qMsgStr, "%*lu", JOB_NUM_LEN, (*queryJob[adr]).jobN);
    else
      sprintf(qMsgStr, "%*lu", JOB_NUM_LEN, (*queryShift[adr]).jobN);
    *(qMsgStr+JOB_NUM_LEN) = '\0';
    movmem(qMsgStr, qJobNumber, JOB_NUM_LEN+1);
    return (qJobNumber);
    }


char *q_opNumber(short adr, QTYPE type)
  {
    static char qOpNumber[4+1];

    if (type == QRUN)
      sprintf(qMsgStr, "%4u", (*queryJob[adr]).operN);
    else
      sprintf(qMsgStr, "%4u", (*queryShift[adr]).operN);
    *(qMsgStr+4) = '\0';
    movmem(qMsgStr, qOpNumber, 4+1);
    return (qOpNumber);
    }


char *q_shftNmbr(short adr, QTYPE type)
  {
    static char qShiftNumber[1+1];

    if (type == QRUN)
      sprintf(qMsgStr, "%u", (*queryJob[adr]).shiftN);
    else
      sprintf(qMsgStr, "%u", (*queryShift[adr]).shiftN);
    *(qMsgStr+1) = '\0';
    movmem(qMsgStr, qShiftNumber, 1+1);
    return (qShiftNumber);
    }


char *q_timeStart(short adr, QTYPE type)
  {
    static char qTimeStart[8+1];

    if (type == QRUN)
      IUITime2Str((*queryJob[adr]).jStrTime, qMsgStr);
    else
      IUITime2Str((*queryShift[adr]).sStrTime, qMsgStr);
    *(qMsgStr+8) = '\0';
    movmem(qMsgStr, qTimeStart, 8+1);
    return (qTimeStart);
    }


char *q_dateStart(short adr, QTYPE type)
  {
    static char qDateStart[8+1];

    if (type == QRUN)
      IUIDate2Str((*queryJob[adr]).jStrDate, qMsgStr);
    else
      IUIDate2Str((*queryShift[adr]).sStrDate, qMsgStr);
    *(qMsgStr+8) = '\0';
    movmem(qMsgStr, qDateStart, 8+1);
    return (qDateStart);
    }


char *q_totalCount(short adr, QTYPE type)
  {
    static char qTotalCount[8+1];

    if (type == QRUN)
      sprintf(qMsgStr, "%8lu", (*queryJob[adr]).totalCnt);
    else
      sprintf(qMsgStr, "%8lu", (*queryShift[adr]).shfTotal);
    *(qMsgStr+8) = '\0';
    movmem(qMsgStr, qTotalCount, 8+1);
    return(qTotalCount);
    }


char *q_setupCount(short adr, QTYPE type)
  {
    static char qSetupCount[8+1];

    if (type == QRUN)
      sprintf(qMsgStr, "%8lu", (*queryJob[adr]).setUpCnt);
    else
      sprintf(qMsgStr, "%8lu", (*queryShift[adr]).sSetuCnt);
    *(qMsgStr+8) = '\0';
    movmem(qMsgStr, qSetupCount, 8+1);
    return(qSetupCount);
    }


char *q_setupTime(short adr, QTYPE type)
  {
    static char qSetupTime[10+1];

    if (type == QRUN)
      IULTime2Str((*queryJob[adr]).setUpTim, qMsgStr);
    else
      IUITime2Str((*queryShift[adr]).sSupTime, qMsgStr);
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qSetupTime, 10+1);
    return (qSetupTime);
    }


char *q_idleTime(short adr, QTYPE type)
  {
    static char qIdleTime[10+1];

    if (type == QRUN)
      IULTime2Str((*queryJob[adr]).idleTime, qMsgStr);
    else
      IUITime2Str((*queryShift[adr]).sIdlTime, qMsgStr);
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qIdleTime, 10+1);
    return (qIdleTime);
    }


char *q_runTime(short adr, QTYPE type)
  {
    static char qRunTime[10+1];

    if (type == QRUN)
      IULTime2Str((*queryJob[adr]).prodTime, qMsgStr);
    else
      IUITime2Str((*queryShift[adr]).sPrdTime, qMsgStr);
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qRunTime, 10+1);
    return (qRunTime);
    }


char *q_totalTime(short adr, QTYPE type)
  {
    static char qTotalTime[10+1];

    if (type == QRUN)
      IULTime2Str((*queryJob[adr]).totlTime, qMsgStr);
    else
      IUITime2Str((*queryShift[adr]).sTotTime, qMsgStr);
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qTotalTime, 10+1);
    return (qTotalTime);
    }


char *q_currentRate(short adr)
  {
    static char qCurrentRate[6+1];

    sprintf(qMsgStr, "%6lu", (*queryJob[adr]).cRate);
    *(qMsgStr+6) = '\0';
    movmem(qMsgStr, qCurrentRate, 6+1);
    return (qCurrentRate);
    }


DWord currentRate(short adr) {
	return ((*queryJob[adr]).cRate);
	}


DWord calcAverageRate(short adr, QTYPE type)
  {
    float totTime;

    if (type == QRUN) {
      totTime = IULTime2Hrs(IULTimeAdd((*queryJob[adr]).prodTime, (*queryJob[adr]).idleTime));
      if (totTime > 0.0)
        return ((DWord)((float)(*queryJob[adr]).totalCnt / totTime));
      else
        return (0L);
      }
    else {
      totTime = IULTime2Hrs(IULTimeAdd(queryShift[adr]->sPrdTime, queryShift[adr]->sIdlTime));
      if (totTime > 0.0)
        return ((DWord)((float)queryShift[adr]->shfTotal / totTime));
      else
        return (0L);
      }
    }


char *q_averageRate(short adr, QTYPE type)
  {
    static char qAverageRate[6+1];

    sprintf(qMsgStr, "%6lu", calcAverageRate(adr, type));
    *(qMsgStr+6) = '\0';
    movmem(qMsgStr, qAverageRate, 6+1);
    return (qAverageRate);
    }


char *q_outType(short adr) {
	switch ((*queryJob[adr]).output) {
		case QUERY_OUTTYPE_KICKER:
			return (StringGet(QueryOutputSTRING, QueryOutputSTRING_Kicker));
		case QUERY_OUTTYPE_FEED:
			return (StringGet(QueryOutputSTRING, QueryOutputSTRING_Feed));
		default:
			return (StringGet(QueryOutputSTRING, QueryOutputSTRING_None));
		}
	}


char *q_status(short adr) {
	short	st;

	st = queryJob[adr]->status;
	if (st & QUERY_STATUS_SETUP)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Setup));
	else if (st & QUERY_STATUS_PRINT)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Printing));
	else if (st & QUERY_STATUS_IDLE)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Idle));
	else if (st & QUERY_STATUS_RUN)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Running));
	else if (st & QUERY_STATUS_UPDATING)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Updating));
	else if (st & QUERY_STATUS_ERROR)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_BadData));
	else if (st & QUERY_STATUS_OFF)
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Offline));
	else
		return (StringGet(QueryStatusSTRING, QueryStatusSTRING_Unknown));
	}


Byte counterStatus(short adr)
  {
    short st;

    st = queryJob[adr]->status;
    if (st & QUERY_STATUS_SETUP)
      return (QUERY_STATUS_SETUP);
    else if (st & QUERY_STATUS_PRINT)
      return (QUERY_STATUS_PRINT);
    else if (st & QUERY_STATUS_IDLE)
      return (QUERY_STATUS_IDLE);
    else if (st & QUERY_STATUS_RUN)
      return (QUERY_STATUS_RUN);
    else if (st & QUERY_STATUS_UPDATING)
      return (QUERY_STATUS_UPDATING);
    else if (st & QUERY_STATUS_ERROR)
      return (QUERY_STATUS_ERROR);
    else if (st & QUERY_STATUS_OFF)
      return (QUERY_STATUS_OFF);
    else
      return (0xFF);
    }


Boolean isOffline(short adr)
  {
    return (queryJob[adr]->status == QUERY_STATUS_OFF ? true : false);
    }


char *q_pieces(short adr)
  {
    static char qPieces[2+1];

    sprintf(qMsgStr, "%2u", (*queryJob[adr]).factor);
    *(qMsgStr+2) = '\0';
    movmem(qMsgStr, qPieces, 2+1);
    return (qPieces);
    }


Byte factor(short adr) {
	return (queryJob[adr]->factor);
	}


char *q_idle(short adr)
  {
    static char qIdle[4+1];

    sprintf(qMsgStr, "%4u", (*queryJob[adr]).timesIdl);
    *(qMsgStr+4) = '\0';
    movmem(qMsgStr, qIdle, 4+1);
    return (qIdle);
    }


char *q_batchCnt(short adr, QTYPE type)
  {
    static char qBatchCnt[8+1];

    if (type == QRUN)
      sprintf(qMsgStr, "%8lu", queryJob[adr]->batchCnt);
    else
      sprintf(qMsgStr, "%8u", queryShift[adr]->sBatch);
    *(qMsgStr+8) = '\0';
    movmem(qMsgStr, qBatchCnt, 8+1);
    return (qBatchCnt);
    }


char *q_downCnt(short adr)
  {
    static char qDownCnt[4+1];

    sprintf(qMsgStr, "%4u", (*queryJob[adr]).downCnt);
    *(qMsgStr+4) = '\0';
    movmem(qMsgStr, qDownCnt, 4+1);
    return (qDownCnt);
    }


char *q_preset(short adr)
  {
    static char qPreset[4+1];

    sprintf(qMsgStr, "%4u", (*queryJob[adr]).preset);
    *(qMsgStr+4) = '\0';
    movmem(qMsgStr, qPreset, 4+1);
    return (qPreset);
    }


char *q_outDelay(short adr) {
	static char	qOutDelay[5+1];

	switch ((*queryJob[adr]).output) {
		case QUERY_OUTTYPE_KICKER:
			sprintf(qMsgStr, "%5.3f", (float)(*queryJob[adr]).kickTime/1000);
			break;
		default:
			sprintf(qMsgStr, "0.000");
			break;
		}
	qMsgStr[1] = IUGetDecimal();
	*(qMsgStr+5) = '\0';
	movmem(qMsgStr, qOutDelay, 5+1);
	return (qOutDelay);
	}


char *q_outDuration(short adr)
  {
    static char qOutDuration[5+1];

    switch ((*queryJob[adr]).output) {
      case QUERY_OUTTYPE_KICKER : sprintf(qMsgStr, "0.025");
                                  break;
      case QUERY_OUTTYPE_FEED   : sprintf(qMsgStr, "%5.3f", (float)(*queryJob[adr]).feedTime/1000);
                                  break;
      default                   : sprintf(qMsgStr, "0.000");
                                  break;
      }
    qMsgStr[1] = IUGetDecimal();
    *(qMsgStr+5) = '\0';
    movmem(qMsgStr, qOutDuration, 5+1);
    return (qOutDuration);
    }


char *q_pcsRqrd(short adr)
  {
    static char qPcsRqrd[8+1];

    sprintf(qMsgStr, "%8lu", (*queryJob[adr]).jPreset);
    *(qMsgStr+8) = '\0';
    movmem(qMsgStr, qPcsRqrd, 8+1);
    return (qPcsRqrd);
    }


DWord calcPcsTilDone(short adr)
  {
    DWord pcsRqrd;

    pcsRqrd = (*queryJob[adr]).jPreset;
    if (pcsRqrd > 0)
      return (pcsRqrd - (*queryJob[adr]).totalCnt);
    else
      return (pcsRqrd);
    }


char *q_pcsTilDone(short adr)
  {
    static char qPcsTilDone[10+1];

    sprintf(qMsgStr, "%10ld", (long)calcPcsTilDone(adr));
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qPcsTilDone, 10+1);
    return (qPcsTilDone);
    }


char *q_percCompleted(short adr)
  {
    DWord pcsRqrd;
    float perc;
    static char qPercCompleted[5+1];

    pcsRqrd = (*queryJob[adr]).jPreset;
    if (pcsRqrd > 0) {
      perc = (float)(*queryJob[adr]).totalCnt / (float)pcsRqrd * 100.0;
      if (perc > 999.9)
        perc = 999.9;
      sprintf(qMsgStr, "%5.1f", perc);
      }
    else
      sprintf(qMsgStr, "  0.0");
    qMsgStr[3] = IUGetDecimal();
    *(qMsgStr+5) = '\0';
    movmem(qMsgStr, qPercCompleted, 5+1);
    return (qPercCompleted);
    }


short percCompleted(short adr) {
	DWord	pcsRqrd;
	float	perc;

	pcsRqrd = (*queryJob[adr]).jPreset;
	if (pcsRqrd > 0) {
		perc = (float)(*queryJob[adr]).totalCnt / (float)pcsRqrd * 100.0;
		if (perc > 999.9)
			perc = 999.9;
		}
	else
		perc = 0.0;
	return (perc);
	}


char *q_noIdleTime(short adr)
  {
    DWord       cRate;
    float       nTime;
    static char qNoIdle[10+1];

    cRate = queryJob[adr]->cRate;
    if (cRate > 0)
      nTime = (float)calcPcsTilDone(adr) / (float)cRate;
    else
      nTime = 0.0;
    if (nTime <= 0.0)
      IULTime2Str(0, qMsgStr);
    else if (nTime > 9999.9999)
      IULTime2Str(IUHrs2LTime(0.0), qMsgStr);
    else
      IULTime2Str(IUHrs2LTime(nTime), qMsgStr);
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qNoIdle, 10+1);
    return (qNoIdle);
    }


char *q_withIdleTime(short adr)
  {
    long        pcsTilDone;
    float       avgPcs, wTime;
    static char qWithIdle[10+1];

    avgPcs = (float) calcAverageRate(adr, QRUN);
    if (avgPcs > 0.0) {
      pcsTilDone = (long) calcPcsTilDone(adr);
      if (pcsTilDone > 0)
        wTime = (float) pcsTilDone / avgPcs;
      else
        wTime = 0.0;
      if (wTime <= 0.0)
        IULTime2Str(0, qMsgStr);
      else if (wTime > 9999.9999)
        IULTime2Str(IUHrs2LTime(0.0), qMsgStr);
      else
        IULTime2Str(IUHrs2LTime(wTime), qMsgStr);
      }
    else
      IULTime2Str(0, qMsgStr);
    *(qMsgStr+10) = '\0';
    movmem(qMsgStr, qWithIdle, 10+1);
    return (qWithIdle);
    }


// ****************************************
// * Number of times machines are offline *
// ****************************************

static void enableOfflineTrace(Boolean on) {
	Rect	r;
	short	i;

	if (on) {
		offlineScrn = VirtualGetNew(OfflineTraceWINDOW, &r);
		for (i=0; i<MAX_MACHINES; i++)
			qOff[i] = 0;
		updateOfflineTrace();
		}
	else
		VirtualDispose(offlineScrn);
	data.query.off = on;
	}


static void updateOfflineTrace(void)
  {
    short height, num, i, j;

    height = offlineHeight-2;
    num = (mchMax > height) ? height : mchMax;
    for (i=j=0; j < num; i++)
      if (queryJob[i] != NULL) {
        VirtualPrint(offlineScrn, j+2, 2, MACH_CNTR_LEN, BWHT, "%*d", MACH_CNTR_LEN, i);
        VirtualPrint(offlineScrn, j+2, 8, 5, BWHT, "%5d", qOff[i]);
        j++;
        }
    for (i=num; i<height; i++)
      VirtualWrite(offlineScrn, i+2, 2, 12, " ", MAG);
    }


// **************
// * Query menu *
// **************

static void QueryAdjustMenu(void) {
	MenuItemCheck(QueryMENU, QueryMENU_Comm, (Boolean)(data.query.enable && Qport.on));
	if ((equip.serial > 0) && (mchMax > 0))
		MenuItemEnable(QueryMENU, QueryMENU_Comm);
	else
		MenuItemDisable(QueryMENU, QueryMENU_Comm);
	MenuItemCheck(QueryMENU, QueryMENU_TrackErrors, (Boolean)data.query.err);
	MenuItemCheck(QueryMENU, QueryMENU_ArchivalData, (Boolean)data.query.arch);
	MenuItemCheck(QueryMENU, QueryMENU_OfflineTrace, (Boolean)data.query.off);
	MenuItemCheck(QueryMENU, QueryMENU_LogToPrinter, (Boolean)data.query.logPrint);
	MenuItemCheck(QueryMENU, QueryMENU_ManualQueries, (Boolean)Qport.wait);
	MenuItemCheck(QueryMENU, QueryMENU_SoundBell, (Boolean)data.query.bell);
	}


void QueryDoMenu(short item) {
	Rect	r;
	short	vsnum;

	switch (item) {
		case QueryMENU_Comm:
			makeQ_GET();
			data.query.enable = !data.query.enable;
			StatusPrint(SCREEN_WIDTH-2, "%c", getQueryState() ? UPARROW : DOWNARROW);
			break;
		case QueryMENU_Timing:
			modifyQueryTiming();
			break;
		case QueryMENU_TrackErrors:
			data.query.err = !data.query.err;
			break;
		case QueryMENU_ArchivalData:
			data.query.arch = !data.query.arch;
			break;
		case QueryMENU_OfflineTrace:
			enableOfflineTrace(data.query.off ? false : true);
			break;
		case QueryMENU_LogToPrinter:
			data.query.logPrint = !data.query.logPrint;
			break;
		case QueryMENU_DisplayQueries:
			r.Set(0, 0, SCREEN_HEIGHT-1, SCREEN_WIDTH-1);
			vsnum = VirtualNew(SCREEN_HEIGHT, SCREEN_WIDTH, WNONE, r, NULL, BRN);
			VirtualFill(vsnum, GIANT_RECT, ' ', BRN);
			ScreenSynch();
			printf(StringGet(QuerySTRING, QuerySTRING_DataLink));
			Qport.test = true;
			EventWaitKey();
			Qport.test = false;
			VirtualDispose(vsnum);
			break;
		case QueryMENU_ManualQueries:
			Qport.wait = !(Qport.wait);
			break;
		case QueryMENU_SoundBell:
			data.query.bell = !data.query.bell;
			break;
		}
	QueryAdjustMenu();
	}


static void QueryMenuInit(void) {
	if (data.query.off)
		enableOfflineTrace((Boolean)data.query.off);
	QueryAdjustMenu();
	}


static void modifyQueryTiming(void) {
	short		vsnum;
	Rect		r;
	DialogStruct	d1, d2, d3, d4, d5, d6;
	char		minDelay[5], maxDelay[5], hiPri[5], loPri[5];
	char		retries[5], errStat[3];

	HelpPush(QueryHELP_Timing, MenuItemGet(QueryMENU, QueryMENU_Timing));
	StatusPush(GlobalSTATUS, GlobalSTATUS_Dialog);
	vsnum = VirtualGetNew(QueryWINDOW_Timing, &r);
	d1.len = 4;
	d1.top = 0;
	d1.lef = 28;
	d1.prev = &d6;
	d1.next = &d2;
	sprintf(minDelay, "%ld", data.query.minDelay*10L);
	d1.ans = minDelay;
	d2.len = 4;
	d2.top = 1;
	d2.lef = 28;
	d2.prev = &d1;
	d2.next = &d3;
	sprintf(maxDelay, "%ld", data.query.maxDelay*10L);
	d2.ans = maxDelay;
	d3.len = 4;
	d3.top = 2;
	d3.lef = 28;
	d3.prev = &d2;
	d3.next = &d4;
	sprintf(hiPri, "%ld", data.query.hiPri*10L);
	d3.ans = hiPri;
	d4.len = 4;
	d4.top = 3;
	d4.lef = 28;
	d4.prev = &d3;
	d4.next = &d5;
	sprintf(loPri, "%ld", data.query.loPri*10L);
	d4.ans = loPri;
	d5.len = 4;
	d5.top = 5;
	d5.lef = 28;
	d5.prev = &d4;
	d5.next = &d6;
	sprintf(retries, "%ld", data.query.retries/100L);
	d5.ans = retries;
	d6.len = 2;
	d6.top = 6;
	d6.lef = 28;
	d6.prev = &d5;
	d6.next = NULL;
	sprintf(errStat, "%d", (Word)data.query.eStat);
	d6.ans = errStat;
	DialogShowPrompts(vsnum, &d1);
	if (DialogEditor(vsnum, &d1) != K_ESC) {
		data.query.minDelay = max(1, atol(minDelay)/10L);
		data.query.maxDelay = max(data.query.minDelay, atol(maxDelay)/10L);
		data.query.hiPri = max(1, atol(hiPri)/10L);
		data.query.loPri = max(1, atol(loPri)/10L);
		sched.SetPriority(data.query.hiPri, data.query.loPri);
		data.query.retries = max(100, atol(retries)*100L);
		data.query.eStat = max(1, atoi(errStat));
		}
	VirtualDispose(vsnum);
	StatusPop();
	HelpPop();
	}

