// Update - updates old PCS100 data files to the new v4.0 format.
//
// Syntax:	Update [-h] [path]
//
// Description:	Updates old PCS100 data files to the new v4.0 format.
//
// Status:	Update may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//
// Options:	-h	Display help screen.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9105.15  JN      Original program.


#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\getargs.h>

#define PCS100			"PCS100"
#define PCS100_SIG		"PCS100 by Vorne Industries Incorporated\x1A"
#define PROGRAM_SIGNIATURE	0x45

enum { EXIT_OK, EXIT_SYNTAX };

#define JOB_EXT		".JOB"
#define JOB_NAME_LEN	30

struct JobStruct {
	DWord	num;
	char	name[JOB_NAME_LEN+1];
	DWord	total;
	Word	date;
	};

static short		jobMax=0;
static JobStruct	**job=NULL;


#define MCH_EXT		".MCH"
#define MAX_MACHINES	100
#define MACH_NAME_LEN	30

struct MachineStruct {
	short	counter;
	short	class_type;
	char	name[MACH_NAME_LEN+1];
	Word	date;
	};

static short		mchMax=0;
static MachineStruct	*mach[MAX_MACHINES];

#define OPR_EXT		".OPR"
#define OPER_NAME_LEN	30

struct OperStruct {
	short	num;
	char	name[OPER_NAME_LEN+1];
	Word	date;
	};

static short		operMax=0;
static OperStruct	**oper=NULL;


static void	HelpOptions(void);
static Boolean	JobLoad(char *name);
static void	JobSave(char *name);
static void	JobUpdate(char *format, ...);
static Boolean	MachineLoad(char *name);
static void	MachineSave(char *name);
static void	MachineUpdate(char *format, ...);
static Boolean	OperatorLoad(char *name);
static void	OperatorSave(char *name);
static void	OperatorUpdate(char *format, ...);


main(int argc, char *argv[]) {
	short	opt;
	char	path[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], file[MAXFILE];

	getinfo("UPDATE");
	fprintf(stderr, COPYRIGHT_FORMAT, progName, "1.0", "1991");
	while ((opt = getopt("h")) != OPT_DONE)
		switch (opt) {
			case 'h':
			default:
				HelpOptions();
				break;
			}
	argc -= optind;
	argv += optind;
	if (argc > 0)
		fnsplit(argv[0], drive, dir, file, NULL);
	else
		fnsplit(dataPath, drive, dir, file, NULL);
	fnmerge(path, drive, dir, strlen(file)==0 ? PCS100 : file, NULL);
	MachineUpdate("%s%s", path, MCH_EXT);
	JobUpdate("%s%s", path, JOB_EXT);
	OperatorUpdate("%s%s", path, OPR_EXT);
	return (EXIT_OK);
	}


// HelpOptions explains what the program does and displays the syntax line.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nUpdates old PCS100 data files to the new v4.0 format.\n\n",
				"Syntax:  UPDATE [-h] [path]\n\n",
				"Status:  Update may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Syntax error.\n\n",
				"Options: -h  Displays this help text.\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}


static Boolean JobLoad(char *name) {
	short	handle, i;
	char	ch;

	if ((handle = open(name, O_RDONLY | O_BINARY)) == -1) {
		printf("Unable to open file %s.\n", name);
		return (false);
		}
	while (read(handle, &ch, sizeof(ch))==sizeof(ch) && ch!=A_CTRLZ)
		;
	if (read(handle, &ch, sizeof(ch))!=sizeof(ch) || ch!=PROGRAM_SIGNIATURE) {
		printf("File %s has an invalid signiature.\n", name);
		return (false);
		}
	read(handle, &jobMax, sizeof(jobMax));
	if (jobMax == 0)
		return (true);
	job = (JobStruct **) malloc(sizeof(JobStruct *) * jobMax);
	for (i=0; i<jobMax; i++) {
		job[i] = (JobStruct *) malloc(sizeof(JobStruct));
		read(handle, job[i], sizeof(JobStruct));
		}
	close(handle);
	return (true);
	}


static void JobSave(char *name) {
	short	handle, i;

	if ((handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) == -1) {
		printf("Unable to save file %s.\n", name);
		return;
		}
	write(handle, PCS100_SIG, sizeof(PCS100_SIG));
	write(handle, &jobMax, sizeof(jobMax));
	for (i=0; i<jobMax; i++) {
		write(handle, job[i], sizeof(JobStruct));
		free(job[i]);
		}
	if (job != NULL)
		free(job);
	close(handle);
	}


static void JobUpdate(char *format, ...) {
	va_list	args;
	char	path[MAXPATH];

	va_start(args, format);
	vsprintf(path, format, args);
	va_end(args);
	if (JobLoad(path))
		JobSave(path);
	}


static Boolean MachineLoad(char *name) {
	short	handle, i;
	char	ch;

	if ((handle = open(name, O_RDONLY | O_BINARY)) == -1) {
		printf("Unable to open file %s.\n", name);
		return (false);
		}
	while (read(handle, &ch, sizeof(ch))==sizeof(ch) && ch!=A_CTRLZ)
		;
	if (read(handle, &ch, sizeof(ch))!=sizeof(ch) || ch!=PROGRAM_SIGNIATURE) {
		printf("File %s has an invalid signiature.\n", name);
		return (false);
		}
	read(handle, &ch, sizeof(ch));
	mchMax = ch;
	for (i=0; i<mchMax; i++) {
		mach[i] = (MachineStruct *) malloc(sizeof(MachineStruct));
		read(handle, mach[i], sizeof(MachineStruct));
		}
	close(handle);
	return (true);
	}


static void MachineSave(char *name) {
	short	handle, i;

	if ((handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) == -1) {
		printf("Unable to save file %s.\n", name);
		return;
		}
	write(handle, PCS100_SIG, sizeof(PCS100_SIG));
	write(handle, &mchMax, sizeof(mchMax));
	for (i=0; i<mchMax; i++) {
		write(handle, mach[i], sizeof(MachineStruct));
		free(mach[i]);
		}
	close(handle);
	}


static void MachineUpdate(char *format, ...) {
	va_list	args;
	char	path[MAXPATH];

	va_start(args, format);
	vsprintf(path, format, args);
	va_end(args);
	if (MachineLoad(path))
		MachineSave(path);
	}


static Boolean OperatorLoad(char *name) {
	short	handle, i;
	char	ch;

	if ((handle = open(name, O_RDONLY | O_BINARY)) == -1) {
		printf("Unable to open file %s.\n", name);
		return (false);
		}
	while (read(handle, &ch, sizeof(ch))==sizeof(ch) && ch!=A_CTRLZ)
		;
	if (read(handle, &ch, sizeof(ch))!=sizeof(ch) || ch!=PROGRAM_SIGNIATURE) {
		printf("File %s has an invalid signiature.\n", name);
		return (false);
		}
	read(handle, &operMax, sizeof(operMax));
	if (operMax == 0)
		return (true);
	oper = (OperStruct **) malloc(sizeof(OperStruct *) * operMax);
	for (i=0; i<operMax; i++) {
		oper[i] = (OperStruct *) malloc(sizeof(OperStruct));
		read(handle, oper[i], sizeof(OperStruct));
		}
	close(handle);
	return (true);
	}


static void OperatorSave(char *name) {
	short	handle, i;

	if ((handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) == -1) {
		printf("Unable to save file %s.\n", name);
		return;
		}
	write(handle, PCS100_SIG, sizeof(PCS100_SIG));
	write(handle, &operMax, sizeof(operMax));
	for (i=0; i<operMax; i++) {
		write(handle, oper[i], sizeof(OperStruct));
		free(oper[i]);
		}
	if (oper != NULL)
		free(oper);
	close(handle);
	}


static void OperatorUpdate(char *format, ...) {
	va_list	args;
	char	path[MAXPATH];

	va_start(args, format);
	vsprintf(path, format, args);
	va_end(args);
	if (OperatorLoad(path))
		OperatorSave(path);
	}
