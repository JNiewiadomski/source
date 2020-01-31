// OIScan - scans the communication between the ITT O/I Panel and the PLC.
//
// Syntax:	OIScan [-2] [-h] [file]
//
// Description:	monitors all communication packets sent between the ITT
//		O/I Panel and the PLC.
//
// Output:	Writes to stdout the data packets being transmitted serially.
//
// Status:	OIScan may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//		2	I/O error.
//
// Options:	-2	Selects COM2 for serial communication.
//		-h	Display help screen.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0ë1  9103.06  JN      Original program.

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\getargs.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "datalink.h"
#include "oiscan.h"

enum { EXIT_OK, EXIT_SYNTAX, EXIT_IO };

FILE	*fptr;

static void	HelpOptions(void);


main() {
	char	*progName;
	int	opt, port=COM1;

	progName = getname("OISCAN");
	fprintf(stderr, COPYRIGHT_FORMAT, progName, "1.0ë2", "1991");
	while ((opt = getopt("2")) != OPT_DONE)
		switch (opt) {
			case '2':
				port = COM2;
				break;
			default:
				HelpOptions();
				break;
			}
	_argc -= optind;
	_argv += optind;
	if (_argc > 0)
		fptr = fopen(_argv[0], "wt");
	else
		fptr = NULL;
	if (!DataLinkInit(port, 9600, 0x11))
		return (EXIT_IO);
	while (!kbhit() || getch() != A_ESC)
		DataLinkReceive();
	DataLinkCleanUp();
	}


// HelpOptions explains what the program does and displays the syntax line.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nScans the communication between the ITT O/I Panel and the PLC.  Output\n",
				"is directed to stdout.  If a file name is specified, then output is\n",
				"also written to the file.\n\n",
				"Syntax:  OIScan [-2] [-h] [file]\n\n",
				"Status:  OIScan may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Syntax error.\n",
				"          2  I/O error.\n\n",
				"Options: -2  Selects COM2 for serial communication.\n",
				"         -h  Display help screen.\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}
