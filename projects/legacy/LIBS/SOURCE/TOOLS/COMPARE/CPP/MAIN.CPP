// Compare - sets DOS ERRORLEVEL to indicate if files are the same.

static char *pHelp[] = {
"The Compare command compares two files and returns a DOS ERRORLEVEL that\n",
"indicates whether or not they are identical.\n\n",
"Syntax: Compare file1 file2 [/?] [/silent]\n\n",
"Status: Compare can return the following status codes:\n",
"          0       No errors.\n",
"          1       Syntax error.\n",
"          2       I/O error.\n\n",
"          3       Files are different.\n\n",
"Options: /?       Displays this help text.\n",
"         /silent  Runs silently.  By default, Compare beeps if the files are\n",
"                  different.\n"
};

// Examples
//	The following checks if two files are identical.
//
//	Compare file1 file2
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9310.01  jn      Original program.

#include <dos.h>
#include <fstream.h>
//#include <iostream.h>
//#include <string.h>

//#include <calert.h>
//#include <cascii.h>
//#include <cbreak.h>
#include <ctime.h>
#include <ctool.h>
//#include <ckeyin.h>

class CMyTool : public CTool {
	enum { kExitIO=kExitSyntax+1, kExitDifferent };
	enum { L_SILENT=L_BAD+1 };

	static KeyWord pTable[];

	Boolean	fSilent;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "s",		L_SILENT	},
	{ "silent",	L_SILENT	},
	};


CMyTool	tool("Compare", "1.0", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fSilent = false;
	}


Boolean	CMyTool::DoOption(LToken option, const char * /*argument*/) {
	switch (option) {
		case L_SILENT:
			fSilent = true;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	if (argc != 2) {
		Help();
		return (kExitSyntax);
		}

	ifstream	f1(argv[0], ios::in, ios::binary), f2(argv[1], ios::in, ios::binary);

	if (!f1) {
		cout << "Cannot open file " << argv[0] << " for input.\n";
		return (kExitIO);
		}
	if (!f2) {
		cout << "Cannot open file " << argv[1] << " for input.\n";
		return (kExitIO);
		}
	//
	// compare the two files
	//
	cout << argv[0] << ":\n";
	cout << argv[1] << ":\n";
	for (;;) {
		Byte	ch1, ch2;

		f1.get(ch1);
		f2.get(ch2);
		if (f1.eof() && f2.eof()) {
			cout << "Files are the same.\n";
			return (kExitOK);
			}
		if (ch1 != ch2)
			break;
		}
	//
	// If silent parameter not specified, sound the speaker.
	//
	CTimer	fTimer(20);

	if (!fSilent)
		sound(440);
	cout << "Files are different.\n";
	if (!fSilent) {
		while (!fTimer.Expired())
			;
		nosound();
		}

	return (kExitDifferent);
	}
