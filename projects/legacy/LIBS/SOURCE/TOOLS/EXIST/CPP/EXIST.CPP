// Exist - determines if a directory or a file exists.
//
// Syntax:	Exist [-h] path
//
// Description:	Determines if the path points to an existing directory or
//		file.
//
// Status:	Exist may return the following status codes:
//		0	Directory/file exists.
//		1	Directory/file does not exist.
//		2	Syntax error.
//
// Options:	-h	Display help screen.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9108.15  JN      Original program.

#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\getargs.h>

enum { EXIT_EXIST, EXIT_NO_EXIST, EXIT_SYNTAX };

static void	HelpOptions(void);

main(int argc, char **argv) {
	short	opt;

	while ((opt = getopt("hH")) != OPT_DONE)
		switch (opt) {
			case 'h':
			case 'H':
			default:
				HelpOptions();
				break;
			}

	// If no parameters specified, copy stdin to character buffer.
	// Otherwise, copy parameters to character buffer.

	argc -= optind;
	argv += optind;
	if (argc == 0)
		exit(EXIT_SYNTAX);
	return (access(*argv, 0)==0 ? EXIT_EXIST : EXIT_NO_EXIST);
	}


// HelpOptions explains what the program does and displays the syntax line.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nDetermines is a directory or file exists.\n\n",
				"Syntax:  EXIST [-h] [path]\n\n",
				"Status:  Exist may return the following status codes:\n",
				"          0  Directory/file exists.\n",
				"          1  Directory/file does not exist.\n",
				"          2  Syntax error.\n\n",
				"Options: -h  Displays this help text.\n"
				};

	getinfo("EXIST");
	printf(COPYRIGHT_FORMAT, progName, "1.0", "1991");
	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}
