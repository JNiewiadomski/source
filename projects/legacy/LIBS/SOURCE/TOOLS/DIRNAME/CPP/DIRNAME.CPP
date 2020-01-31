#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\getargs.h>

enum {	ERR_OK,
	ERR_PARAMS,
	ERR_SYNTAX,
	ERR_PROG
	};

static void HelpOptions(void);

main(int argc, char **argv) {
	char	*progName, *outName, dir[MAXPATH], *s;
	FILE	*fp;

	progName = getname("DIRNAME");
	printf(COPYRIGHT_FORMAT, progName, "1.0", "1990");
	switch (argc) {
		case 1:
			outName = "SETDIR.BAT";
			break;
		case 2:
			outName = argv[1];
			break;
		default:
			HelpOptions();
			break;
		}
	getcwd(dir, MAXPATH);
	s = dir + strlen(dir) - 1;		// point to end of string
	if (*s == '\\')
		s = "ROOT";
	else
		while (*(s-1) != '\\')
			s--;
	if ((fp = fopen(outName, "wt")) == NULL)
		return (ERR_PROG);
	fprintf(fp, "SET DIR=%s", s);
	fclose(fp);
	return (ERR_OK);
	}


// Displays a brief description of the programs purpose and the command line
// used to invoke it.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nCurrent Directory Name Extractor\n",
				"Creates a batch file that sets an environment variable to the name\n",
				"of the crrent subdirectory.  The default batch file name is SETDIR.BAT.\n\n",
				"Syntax: DIRNAME [<filename>]\n",
				"\t 0  no errors\n",
				"\t 1  error in parameters\n",
				"\t 2  syntax error in file\n",
				"\t 3  I/O or program error\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(ERR_PARAMS);
	}
