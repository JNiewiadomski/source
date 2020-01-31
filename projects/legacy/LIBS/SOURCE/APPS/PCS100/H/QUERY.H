// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#define QUERY_STATUS_RUN       0x01     /* 0000 0001 */
#define QUERY_STATUS_IDLE      0x02     /* 0000 0010 */
#define QUERY_STATUS_SETUP     0x04     /* 0000 0100 */
#define QUERY_STATUS_PRINT     0x18     /* 0001 1000 */
#define QUERY_STATUS_UPDATING  0x20     /* 0010 0000 */
#define QUERY_STATUS_OFF       0x40     /* 0100 0000 */
#define QUERY_STATUS_ERROR     0x80     /* 1000 0000 */

#define QUERY_VIRGIN 0xFF

typedef struct {
	Byte	adr;		/* counter address */
	DWord	jobN;		/* job number */
	Byte	shiftN;		/* shift number */
	Word	operN;		/* operator number */
	Byte	output;		/* output type */
	Byte	status;		/* counter status */
	Byte	factor;		/* counter multiplier */
	Word	timesIdl;	/* number of times idle */
	DWord	totalCnt;	/* total count */
	DWord	batchCnt;	/* total batches */
	Word	downCnt;	/* batch countdown */
	DWord	setUpCnt;	/* setup total count */
	Word	preset;		/* batch preset */
	Word	feedTime;	/* feed output time */
	Word	kickTime;	/* kicker output time */
	DWord	setUpTim;	/* set-up time */
	DWord	idleTime;	/* idle time */
	DWord	prodTime;	/* production time */
	DWord	totlTime;	/* total time */
	DWord	timeDone;	/* time until done */
	Word	pctDone;	/* percent done */
	DWord	cntDone;	/* count until done */
	DWord	jPreset;	/* job quantity */
	DWord	cRate;		/* current rate */
	Word	jStrTime;	/* job start time */
	Word	jStrDate;	/* job start date */
	Word	jEndTime;	/* job end time */
	Word	jEndDate;	/* job end date */
	} QueryJobStruct;

typedef struct {
	Byte	adr;		/* counter address */
	DWord	jobN;		/* job number */
	Byte	shiftN;		/* shift number */
	Word	operN;		/* operator number */
	Word	sStrTime;	/* shift start time */
	Word	sStrDate;	/* shift start date */
	DWord	shfTotal;	/* shift total count */
	DWord	sSetuCnt;	/* shift setup count */
	Word	sBatch;		/* shift total batches */
	Word	sSupTime;	/* shift setup time */
	Word	sIdlTime;	/* shift idle time */
	Word	sPrdTime;	/* shift production time */
	Word	sTotTime;	/* shift total time */
	DWord	sCurRate;	/* shift current rate */
	} QueryShiftStruct;

#define QRUN_SIZE	sizeof(QueryJobStruct)
#define QSHIFT_SIZE	sizeof(QueryShiftStruct)


void	QueryDataSave(void);
void	QueryDoMenu(short item);
void	QueryInit(void);
void	QueryInitData(void);
Boolean	QuerySubscribe(void);

DWord			calcAverageRate(short adr, QTYPE type);
DWord			calcPcsTilDone(short adr);
Byte			counterStatus(short adr);
void			createQuery(short adr);
DWord			currentRate(short adr);
void			deleteQuery(short adr);
Byte			factor(short adr);
QueryJobStruct		*getJobQuery(Byte counter);
QueryShiftStruct	*getShiftQuery(Byte counter);
int			getQueryPort(void);
Boolean			getQueryState(void);
Boolean			isOffline(short adr);
short			percCompleted(short adr);
char			*q_averageRate(short adr, QTYPE type);
char			*q_batchCnt(short adr, QTYPE type);
char			*q_currentRate(short adr);
char			*q_dateStart(short adr, QTYPE type);
char			*q_downCnt(short adr);
char			*q_idle(short adr);
char			*q_idleTime(short adr, QTYPE type);
char			*q_jobNumber(short adr, QTYPE type);
char			*q_noIdleTime(short adr);
char			*q_opNumber(short adr, QTYPE type);
char			*q_outDelay(short adr);
char			*q_outDuration(short adr);
char			*q_outType(short adr);
char			*q_pcsRqrd(short adr);
char			*q_pcsTilDone(short adr);
char			*q_percCompleted(short adr);
char			*q_pieces(short adr);
char			*q_preset(short adr);
char			*q_runTime(short adr, QTYPE type);
char			*q_setupCount(short adr, QTYPE type);
char			*q_setupTime(short adr, QTYPE type);
char			*q_shftNmbr(short adr, QTYPE type);
char			*q_status(short adr);
char			*q_timeStart(short adr, QTYPE type);
char			*q_totalCount(short adr, QTYPE type);
char			*q_totalTime(short adr, QTYPE type);
char			*q_withIdleTime(short adr);
void			reinstallQuery(int port);
void			resetQuery(void);
void			updateQuery(void);
