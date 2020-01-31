// íkeywords: substenv.cpp 1.1 wed 03-17-93 10:53:42í
// ílgbí
// 1.0     JACKN     wed 03-17-93 10:49:24
//         Original program; 9302.25
// 1.1     JACKN     wed 03-17-93 10:53:42
//         Allow /delimit and /max options to accept arguments.
// ílgeí
// ínokeywordsí

// SubstEnv - substitutes % delimited keywords with DOS environment values.

static char *pHelp[] = {
"A filter that substitutes batch file style environment variables with their\n",
"current values.\n\n",
"Syntax:  SubstEnv [/?]\n\n",
"Status:  SubstEnv may return the following status codes:\n",
"          0        No errors.\n",
"          1        Syntax error.\n",
"          2        Out of memory.\n\n",
"Options: /?        Displays this help text.\n",
"         /delimit  Specifies variable delimiters (default is '%').\n",
"         /max      Specifies maximum line length (default is 255 bytes).\n"
};

#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <ctool.h>

class CMyTool : public CTool {
	enum	{ kExitMemory=kExitSyntax+1 };
	enum	{ L_MAX=L_BAD+1, L_DELIMITER };

	static KeyWord pTable[];

	short	fMaxLine;
	char	fDelimiter;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "delimit",	-L_DELIMITER	},
	{ "max",	-L_MAX		},
	};


CMyTool	tool("SubstEnv", "1.0.1", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fMaxLine = 256;
	fDelimiter = '%';
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_DELIMITER:
			fDelimiter = *argument;
			return (true);
		case L_MAX:
			fMaxLine = atoi(argument) + 1;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char ** /*argv*/) {
	if (argc != 0) {
		Help();
		return (kExitSyntax);
		}

	char	*line, *lp, nl, *delimit, *env;
	short	line_num=1;

	line = new char[fMaxLine];
	if (!line)
		return (kExitMemory);
	while (cin.get(line, fMaxLine)) {
		lp = line;
		while ((delimit = strchr(lp, fDelimiter)) != 0) {
			*delimit = 0;
			cout << lp;
			lp = delimit + 1;
			if ((delimit = strchr(lp, fDelimiter)) == 0) {
				cerr << "Unterminated variable on line " << line_num << "\n";
				cout << fDelimiter;
				break;
				}
			*delimit = 0;
			strupr(lp);
			if ((env = getenv(lp)) == 0) {
				cerr << "Variable " << lp << " is undefined.\n";
				cout << fDelimiter << lp << fDelimiter;
				}
			else
				cout << env;
			lp = delimit + 1;
			}
		cout << lp;
		if (cin.get(nl)) {
			if (nl == '\n') {
				line_num++;
				cout << '\n';
				}
			else {
				cerr << "Line " << line_num << " is longer than " << (fMaxLine-1) << " characters.\n";
				cin.putback(nl);
				}
			}
		}
	return (kExitOK);
	}
