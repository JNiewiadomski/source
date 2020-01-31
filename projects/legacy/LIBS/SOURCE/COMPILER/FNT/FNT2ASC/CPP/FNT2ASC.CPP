#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\getargs.h>

#include "error.h"
#include "gen.h"

static void HelpOptions(void);

main(int argc, char **argv) {
	char	*progName, *outName;
	short	opt;

	progName = getname("FNT2ASC");
	printf(COPYRIGHT_FORMAT, progName, "1.1", "1990, 1991");
	while ((opt = getopt("df:h:")) != OPT_DONE)
		switch (opt) {
			case 'd':
				double_width=true;
				break;
			case 'f':
				footer = atoi(optarg);
				break;
			case 'h':
				header = atoi(optarg);
				break;
			case OPT_ERR:
				HelpOptions();
				break;
			}
	switch (argc-optind) {
		case 1:
			outName = NULL;
			break;
		case 2:
			outName = argv[optind+1];
			break;
		default:
			HelpOptions();
			break;
		}
	Gen(argv[optind], outName);
	return (ERR_OK);
	}


// Displays a brief description of the programs purpose and the command line
// used to invoke it.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nBinary to ASCII Font Converter\n",
				"Reads in a binary font family and converts it to ASCII format.\n\n",
				"Syntax: FNT2ASC [-d] [-f] [-h] source [destination]\n",
				"\t -d    double the width of all characters\n",
				"\t -f+#  insert # rows to character footer\n",
				"\t -f-#  delete # rows from character footer\n",
				"\t -h+#  insert # rows to character header\n",
				"\t -h-#  delete # rows from character header\n",
				"\nFNT2ASC may return the following status codes:\n",
				"\t 0  no errors\n",
				"\t 1  error in parameters\n",
				"\t 2  syntax error in file\n",
				"\t 3  I/O or program error\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(ERR_PARAMS);
	}
