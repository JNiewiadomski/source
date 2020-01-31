#include <dir.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROG_NAME	"PRUNE"

typedef struct {
	unsigned	day   : 5;
	unsigned	month : 4;
	unsigned	year  : 7;
	} PackedDate;


typedef struct {
	unsigned	seconds : 5;
	unsigned	minutes : 6;
	unsigned	hours   : 5;
	} PackedTime;


static int	CompareDateTime(time_t t1, struct ffblk *ffblk);
static void	Error(void);
static time_t	MakeDateTime(PackedDate *aDate, PackedTime *aTime);
static void	Start(int days, int argc, char *argv[]);


static char	pOrgPath[MAXPATH];
static char	pPath[MAXPATH];
static char	pDrive[MAXDRIVE];
static char	pDir[MAXDIR];
static char	pName[MAXFILE+MAXEXT];
static char	pExt[MAXEXT];
static char	pTempPath[MAXPATH];


main(int argc, char *argv[]) {
	int	days;

	if (argc < 3)
		Error();
	days = atoi(argv[1]);
	Start(days, argc-2, argv+2);
	return (EXIT_SUCCESS);
	}


static int CompareDateTime(time_t t1, struct ffblk *ffblk) {
	time_t	t2;

	t2 = MakeDateTime((PackedDate *)(&ffblk->ff_fdate), (PackedTime *)(&ffblk->ff_ftime));
	return (t2 <= t1);
	}


static void Error(void) {
	printf("Usage: %s days filelist...\n", PROG_NAME);
	printf("\n");
	printf("Purges files that have not been modified for the specified\n");
	printf("number of days.\n");
	exit(EXIT_FAILURE);
	}


static time_t MakeDateTime(PackedDate *aDate, PackedTime *aTime) {
	struct tm	time_struct;

	time_struct.tm_year = aDate->year + 80;
	time_struct.tm_mon = aDate->month - 1;
	time_struct.tm_mday = aDate->day;
	time_struct.tm_hour = aTime->hours;
	time_struct.tm_min = aTime->minutes;
	time_struct.tm_sec = aTime->seconds;
	time_struct.tm_isdst = -1;
	return (mktime(&time_struct));
	}


#define	DAY	(60L * 60L * 24L)

static void Start(int days, int argc, char *argv[]) {
	time_t		t;
	struct ffblk	ffblk;
	int		done;

	t = time(NULL) - (DAY * days);
	for ( ; argc; argc--) {
		strcpy(pOrgPath, *argv++);
		fnsplit(pOrgPath, pDrive, pDir, pName, pExt);
		strcat(pName, pExt);
		if (strlen(pName) == 0)
			strcat(pOrgPath, "*.*");
		done = findfirst(pOrgPath, &ffblk, 0);
		while (!done) {
			fnmerge(pPath, pDrive, pDir, NULL, NULL);
			if (CompareDateTime(t, &ffblk)) {
				strcpy(pTempPath, pPath);
				strcat(pTempPath, ffblk.ff_name);
				if (remove(pTempPath) == -1) {
					printf("%s: ", PROG_NAME);
					switch (errno) {
						case ENOENT:
							printf("No such file or directory (%s).\n", pTempPath);
							break;
						case EACCES:
							printf("Permission denied to remove file '%s'.\n", pTempPath);
							break;
						default:
							printf("Unrecognized error while removing file '%s'.\n", pTempPath);
							break;
						}
					}
				}
			done = findnext(&ffblk);
			}
		}
	}