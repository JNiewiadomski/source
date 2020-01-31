// íkeywords: tee.cpp 1.1 mon 03-22-93 10:14:22í
// ílgbí
// 1.0     JACKN     mon 03-22-93 10:06:03
//         Original program; 9103.06
// 1.1     JACKN     mon 03-22-93 10:14:22
//         Use new Base library; 9301.11
// ílgeí
// ínokeywordsí

// Tee - echoes stdin to stdout and optional files.

#include <fstream.h>
#include <iostream.h>
#include <stdio.h>
#include <string.h>

#include <ctool.h>

static char *pHelp[] = {
"Copies standard input onto standard output.  If any files are named, they\n",
"too become destinations for output.  The files are created if necessary.\n",
"If they already exist, their prior contents are lost unless the append option\n",
"is used.\n\n",
"Syntax:  Tee [/?] [/append] [file...]\n\n",
"Status:  Tee may return the following status codes:\n",
"          0       No errors.\n",
"          1       Syntax error.\n",
"          2       I/O error.\n\n",
"Options: /?       Display this help text.\n",
"         /append  Append to existing files.\n"
};

class CMyTool : public CTool {
	enum { kExitIO=kExitSyntax+1 };
	enum { L_APPEND=L_BAD+1 };

	static KeyWord pTable[];

	int	fOpenMode;			// open for write/append

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "append",	L_APPEND	},
	};


CMyTool	tool("Tee", "1.1", "1991-1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fOpenMode = ios::out;
	}


Boolean	CMyTool::DoOption(LToken option, const char * /*argument*/) {
	switch (option) {
		case L_APPEND:
			fOpenMode |= ios::app;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	int		numFiles, i, status=kExitOK;
	ofstream	*fout[FOPEN_MAX];
	char		ch;

	if (argc > FOPEN_MAX) {
		cerr << "Maximum number of files is " << FOPEN_MAX << ".\n";
		return (kExitSyntax);
		}
	for (numFiles=i=0; i<argc; i++) {
		fout[i] = new ofstream(argv[i], fOpenMode);
		if (!fout[i]) {
			cerr << "Unable to open file '" << argv[i] << "'.\n";
			status = kExitIO;
			}
		else
			numFiles++;
		}
	while (cin.get(ch)) {
		cout << ch;
		for (i=0; i<numFiles; i++)
			*fout[i] << ch;
		}
	for (i=0; i<numFiles; i++)
		delete fout[i];
	return (status);
	}
