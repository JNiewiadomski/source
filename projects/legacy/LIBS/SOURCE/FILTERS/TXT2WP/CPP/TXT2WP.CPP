// íkeywords: txt2wp.cpp 1.0 mon 03-22-93 10:17:50í
// ílgbí
// 1.0     JACKN     mon 03-22-93 10:17:50
//         Original program; 9302.15
// ílgeí
// ínokeywordsí

#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <ctool.h>

static char *pHelp[] = {
"Txt2WP is a filter that prepares an ASCII text file for input into a word\n",
"processor by converting all single line terminators into spaces and\n",
"converting line terminator pairs into single line terminators.\n\n",
"Syntax:  Txt2WP [/?] [/line]\n\n",
"Input:   Standard input.\n\n",
"Output:  Standard Output.\n\n",
"Status:  Txt2WP may return the following status codes:\n",
"          0     No errors.\n",
"          1     Syntax error.\n",
"          2     I/O error.\n\n",
"Options: /?     Displays this help text.\n",
"         /line  Maximum length of a line (default 255).\n"
};

class CMyTool : public CTool {
	enum	{ kExitIO=kExitSyntax+1 };
	enum	{ L_LINE_LEN=L_BAD+1 };

	static KeyWord pTable[];

	short	fLineLen;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "line",	-L_LINE_LEN	},
	};


CMyTool	tool("Txt2WP", "1.0", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fLineLen = 255;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_LINE_LEN:
			fLineLen = atoi(argument);
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char ** /*argv*/) {
	if (argc != 0) {
		Help();
		return (kExitSyntax);
		}

	int	status=kExitOK;
	char	*buff, *bp, ch;
	Boolean	first=true, ln=false;

	if ((buff = new char[fLineLen]) == 0) {
		cerr << "Error: Out of memory.\n";
		return (kExitIO);
		}
	while (cin.get(buff, fLineLen)) {
		if (cin.get(ch) && ch != '\n') {
			cerr << "Error: Line length exceeded " << fLineLen << " characters.\n";
			cin.putback(ch);
			status = kExitIO;
			}
		if (strlen(buff) > 0) {
			bp = strtok(buff, " ");
			while (bp) {
				if (!first)
					cout << ' ';
				cout << bp;
				bp = strtok(0, " ");
				first = false;
				}
			}
		else if (!first) {
			cout << '\n';
			first = ln = true;
			}
		else {
			if (ln)
				ln = false;
			else {
				cout << '\n';
				ln = true;
				}
			}
		}
	return (status);
	}
