// Rez - resource compiler
//
// Syntax:	Rez [-h] [-i path] [-o path] source
//
// Description:	Compiles the resource data of a file according to a textual
//		description.  Rez has the ability to include resource
//		description files and to define symbolic constants.
//
// Status:	Rez may return the following status codes:
//		0	No errors.
//		1	Error in parameters.
//		2	Syntax error in file.
//		3	I/O or program error.
//
// Options:	-h	Displays help screen.
//		-i	Specifies search paths for file inclusion.
//		-o	Specifies path where output will be saved.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9104.22  JN      Original program.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\getargs.h>

#include "error.h"
#include "lex.h"
#include "parse.h"
#include "symbol.h"


static void	HelpOptions(void);


// Main driver of the Symbolic Resource Compiler.  The following options are
// recognized on the command line:
//
//	-i	Search the following pathnames for #include files.  This
//		option can be specified more than once.  Paths are searched
//		in the order they appear on the command line.
//
// 		A PATH environment variable is constructed from the include
//		option.  When we wish to include a file, searchpath() will be
//		called, which automatically uses the PATH environment
//		variable to search for the file if it is not found in the
//		current subdirectory.
//
//	-o	Generate output using the path specified.  The name of the
//		output file can be specified using the .FILE directive in
//		the source file itself.

main(int argc, char *argv[]) {
	int	incLen=5, opt;
	char	incBuf[MAXDOS+1], outBuf[MAXDOS+1];

	strcpy(incBuf, "PATH=");
	strcpy(outBuf, "");
	getinfo("REZ");
	fprintf(stderr, COPYRIGHT_FORMAT, progName, "1.0", "1991");
	while ((opt = getopt("i:o:")) != OPT_DONE)
		switch (opt) {
			case 'i':
				if ((incLen += strlen(optarg) + 1) > MAXDOS)
					HelpOptions();
				strcat(incBuf, ";");
				strcat(incBuf, optarg);
				break;
			case 'o':
				strcpy(outBuf, optarg);
				strcat(outBuf, "\\");
				break;
			default:
				HelpOptions();
				break;
			}
	argc -= optind;
	argv += optind;
	if (argc != 1)
		HelpOptions();
	putenv(incBuf);			// now let searchpath() do its thing
	SymbolInit();
	Parse(argv[0], outBuf);
	return (ERR_NONE);
	}


// Displays a brief description of the programs purpose and the command line
// used to invoke it.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nCompiles the resource data of a file according to a textual description.\n",
				"Rez has the ability to include resource description files and to define\n",
				"symbolic constants.\n\n",
				"Syntax:  Rez [-h] [-i path] [-o path] source\n\n",
				"Status:  Rez may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Error in parameters.\n",
				"          2  Syntax error in file.\n",
				"          3  I/O or program error.\n\n",
				"Options: -h  Displays this help text.\n",
				"         -i  Specifies search paths for file inclusion.\n",
				"         -o  Specifies path where output will be saved.\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(ERR_PARAMS);
	}
