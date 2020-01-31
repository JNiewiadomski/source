// Stamp - allows user to modify file creation dates and times.
//
// Syntax:	Stamp [-h] filename... MM-DD-YYYY HH:MM:SS
//
// Description:	Allows the user to modify the file creation dates and times.
//
// Status:	Stamp may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//		2	Invaid date.
//		3	Invaid time.
//
// Options:	-h	Display help screen.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9105.30  JN      Original program.


#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\getargs.h>

#define BASE_YEAR	1980

enum { EXIT_OK, EXIT_SYNTAX, EXIT_DATE, EXIT_TIME };

static void		HelpOptions(void);
static struct ftime	*MakeDateTime(char *sdate, char *stime);

main(int argc, char *argv[]) {
	int		opt, handle;
	int		done;
	struct ffblk	fblock;
	struct ftime	*dateTime;

	getinfo("STAMP");
	printf(COPYRIGHT_FORMAT, progName, "1.1", "1991");
	while ((opt = getopt("hH")) != OPT_DONE)
		switch (opt) {
			case 'h':
			case 'H':
			default:
				HelpOptions();
				break;
			}
	argc -= optind;
	argv += optind;
	if (argc < 3)
		HelpOptions();
	dateTime = MakeDateTime(argv[argc-2], argv[argc-1]);
	argc -= 2;
	for ( ; argc > 0; argc--) {
		done = findfirst(argv[argc-1], &fblock, 0);
		while (!done) {
			handle = open(fblock.ff_name, O_RDWR);
			if (setftime(handle, dateTime) == -1)
				printf("Unable to stamp %s.\n", fblock.ff_name);
			close(handle);
			done = findnext(&fblock);
			}
		}
	return (EXIT_OK);
	}


// HelpOptions explains what the program does and displays the syntax line.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nAllows user to modify file creation dates and times.\n\n",
				"Syntax:  STAMP [-h] filename... MM-DD-YY HH:MM:SS\n\n",
				"Status:  Stamp may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Syntax error.\n",
				"          2  Invalid date.\n",
				"          3  Invalid time.\n\n",
				"Options: -h  Displays this help text.\n\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}


static struct ftime *MakeDateTime(char *sdate, char *stime) {
	Word			year, month, day, hour, minute, sec;
	int			items;
	static struct ftime	dateTime;

	items = sscanf(sdate, "%02d-%02d-%04d", &month, &day, &year);
	if (items != 3 || year < BASE_YEAR || year > BASE_YEAR+127) {
		printf("Invalid date format (%s)\n", sdate);
		exit(EXIT_DATE);
		}
	items = sscanf(stime, "%02d:%02d:%02d", &hour, &minute, &sec);
	if (items != 3 || hour > 23 || minute > 59 || sec > 59) {
		printf("Invalid time format (%s)\n", stime);
		exit(EXIT_TIME);
		}
	dateTime.ft_year = year - BASE_YEAR;
	dateTime.ft_month = month;
	dateTime.ft_day = day;
	dateTime.ft_hour = hour;
	dateTime.ft_min = minute;
	dateTime.ft_tsec = sec / 2;
	return (&dateTime);
	}
