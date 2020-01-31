// íkeywords: params.cpp 1.1 mon 03-22-93 10:02:57í
// ílgbí
// 1.0     JACKN     mon 03-22-93 10:01:32
//         Original program.
// 1.1     JACKN     mon 03-22-93 10:02:57
//         Check for NULL pointer returned from strtok; 9301.11
// ílgeí
// ínokeywordsí

// Writes parameters, including program name, to standard output.  The
// parameters are written one per line.  This command is useful for checking
// the results of variable substitution, DOS environment variable
// interpretation, and filename generation.
//
// Syntax:  PARAMS [parameter...]
//
// Status:  Params returns the following status code:
//	0	No errors.

#include <dir.h>
#include <dos.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#define	VERSION	"1.0.1"
#define YEAR	"1992, 1993"

static const char	pCopyright[] = "  Version " VERSION "  Copyright (c) " YEAR " Vorne Industries Incorporated\n\n";

main(int argc, char *argv[]) {
	char	progName[MAXFILE], *s, *env;
	int	count;

	if (_osmajor < 3)
		strcpy(progName, "PARAMS");
	else
		fnsplit(*argv, 0, 0, progName, 0);
	cout << progName << pCopyright;
	s = getenv(progName);
	if (s) {
		env = new char[strlen(s)+1];
		if (env) {
			strcpy(env, s);
			s = strtok(env, " ");
			for (count=0; s; count++) {
				cout << progName << "[" << count << "] " << s << "\n";
				s = strtok(0, " ");
				}
			delete[] env;
			}
		}
	for (count=0; count<argc; count++)
		cout << "[" << count << "] " << argv[count] << "\n";
	return (0);
	}
