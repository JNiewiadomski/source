// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#define JOB_NUM_LEN	8
#define JOB_NAME_LEN	30
#define JOB_TOT_LEN	8

typedef struct {
	DWord	num;
	char	name[JOB_NAME_LEN+1];
	DWord	total;
	Word	date;
	} JobStruct;

extern short		jobMax;
extern JobStruct	**job;

void	JobAdjustMenu(void);
void	JobCleanUp(void);
void	JobDoMenu(short item);
void	JobInit(void);
void	JobSubscribe(void);


void    addJob(long jobNumber);
void    closeJob(void);
void	exportClosedJobs(void);
void	exportOpenJobs(void);
void    installJobs(void);
Boolean isAllJobsDisplayed(long jobNumber);
Boolean isSingleJobDisplayed(long jobNumber);
void    listJobs(void);

void	doAllJobsScreen(int ch, QTYPE type);
void	monitorAllJobs(void);
void	printAllJobsScreen(void);
void	updateAllJobsScreen(long jobNumber);

void	doSingleJobScreen(int ch, QTYPE type);
void	monitorSingleJob(void);
void	printSingleJobScreen(void);
void	updateSingleJobScreen(long jobNumber);

void	monitorShiftAllJobs(void);
void	printShiftAllJobsScreen(void);
void	updateShiftAllJobsScreen(long jobNumber);

void	monitorShiftSingleJob(void);
void	printShiftSingleJobScreen(void);
void	updateShiftSingleJobScreen(long jobNumber);
