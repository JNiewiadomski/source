#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\getargs.h>

#include "error.h"
#include "gen.h"

static void HelpOptions(void);

main(int argc, char **argv) {
	char	*progName, *outName;

	progName = getname("ASC2ASM");
	printf(COPYRIGHT_FORMAT, progName, "1.0", "1990");
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
				"\nASCII to Assembly Font Converter\n",
				"Reads in a source file describing a character set and generates the\n",
				"corresponding Intel 8051 assembler source file.\n\n",
				"Syntax: ASC2ASM <source> [<destination>]\n",
				"\nASC2ASM may return the following status codes:\n",
				"\t 0  no errors\n",
				"\t 1  error in parameters\n",
				"\t 2  syntax error in file\n",
				"\t 3  I/O or program error\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(ERR_PARAMS);
	}
