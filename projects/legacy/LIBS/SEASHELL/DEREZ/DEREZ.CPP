// DeRez - resource decompiler
//
// Syntax:	DeRez [-h] source [destination]
//
// Description:	Reads in a resource file and generates the equivalent textual
//		description.
//
// Status:	DeRez may return the following status codes:
//		0	No errors.
//		1	Error in parameters.
//		2	Syntax error in file.
//		3	I/O or program error.
//
// Options:	-h	Displays help screen.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9104.22  JN      Original program.

#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\getargs.h>

#include "error.h"
#include "gen.h"


static void	HelpOptions(void);


main(int argc, char **argv) {
	char	*progName, *outName;

	progName = getname("DEREZ");
	printf(COPYRIGHT_FORMAT, progName, "1.0", "1991");
	switch (argc) {
		case 2:
			outName = NULL;
			break;
		case 3:
			outName = argv[2];
			break;
		default:
			HelpOptions();
			break;
		}
	Gen(argv[1], outName);
	return (ERR_OK);
	}


// Displays a brief description of the programs purpose and the command line
// used to invoke it.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nReads in a resource file and generates the equivalent textual description.\n\n",
				"Syntax:  DeRez [-h] source [destination]\n\n",
				"Status:  DeRez may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Error in parameters.\n",
				"          2  Syntax error in file.\n",
				"          3  I/O or program error.\n\n"
				"Options: -h  Displays this help text.\n",
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(ERR_PARAMS);
	}
