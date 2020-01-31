// Right$ - Returns a specified number of rightmost characters in a string.

static char *pHelp[] = {
"Returns a specified number of rightmost characters in a string.\n\n",
"Syntax: Right$ num string [/set x] [/?]\n\n",
"Status: Right$ can return the following status codes:\n",
"          0         No errors.\n",
"          1         Syntax error.\n\n",
"Options: /?         Displays this help text.\n",
"         /output x  Redirects output to the file x.\n",
"         /set x     Preceed output with 'SET x='\n"
};

// Examples
//	The following returns the 8 rightmost characters.
//
//	Right$ 8 "0123456789"
//
// History:	jn	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.1    9309.01  jn      Add ability to redirect output to a file.
// 1.0    9306.24  jn      Original program.

#include <fstream.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <ctool.h>

class CMyTool : public CTool {
//	enum { kExitAbort=kExitSyntax+1 };
	enum { L_OUTPUT=L_BAD+1, L_SET };

	static KeyWord pTable[];

	ostream	*fOut;
	char	*fSet;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	~CMyTool();
	};


KeyWord CMyTool::pTable[] = {
	{ "o",		-L_OUTPUT	},
	{ "output",	-L_OUTPUT	},
	{ "s",		-L_SET		},
	{ "set",	-L_SET		},
	};


CMyTool	tool("Right$", "1.0", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));
	fOut = &cout;
	fSet = 0;
	}


CMyTool::~CMyTool() {
	if (fOut != &cout)
		delete fOut;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_OUTPUT:
			fOut = new ofstream(argument);
			return (fOut != 0);
		case L_SET:
			fSet = new char[strlen(argument)+1];
			if (fSet) {
				strcpy(fSet, argument);
				strupr(fSet);
				}
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	int		num, len;
	const char	*string;

	if (argc != 2) {
		Help();
		return (kExitSyntax);
		}
	num = atoi(argv[0]);
	if (num > 0) {
		string = argv[1];
		len = strlen(string);
		if (num < len)
			string += len - num;
		if (fSet)
			*fOut << "SET " << fSet << '=';
		*fOut << string;
		}
	return (kExitOK);
	}
