#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\getargs.h>

#include "error.h"
#include "gen.h"

static void HelpOptions(void);

main(int argc, char **argv) {
	char	*progName, *outName;

	progName = getname("EGA2ASC");
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
				"\nEGA to ASCII Font Converter\n",
				"Reads in a binary EGA font whose width is always 8 pixels and converts\n",
				"it to ASCII format.\n\n",
				"Syntax: EGA2ASC <source> [<destination>]\n",
				"\nEGA2ASC may return the following status codes:\n",
				"\t 0  no errors\n",
				"\t 1  error in parameters\n",
				"\t 2  syntax error in file\n",
				"\t 3  I/O or program error\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(ERR_PARAMS);
	}
