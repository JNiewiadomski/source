// PDI - parallel digital interface compiler.

static char *pHelp[] = {
"The Parallel Digital Interface compiler processes the commands in the\n",
"specified source file.\n\n",
"Syntax: PDI filename [/?] [/batch]\n\n",
"Status: Alert can return the following status codes:\n",
"          0          No errors.\n",
"          1          Errors encountered.\n\n",
"Options: /?          Displays this help text.\n",
"         /address    Set the base address (default=0x0300).\n",
"         /batch      Suppresses creation of batch file.\n",
"         /interrupt  Set the interrupt level (default=5).\n",
};

// Examples
//	The following example compiles the specified file and executes it.
//
//	PDI file
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0à4  9311.17  jn      Modify and recompile for BASE library.
// 1.0à3  9110.24  jn
// 1.0    9005.04  jn      Original program.

#include <dir.h>
#include <stdio.h>
#include <stdlib.h>

#include <cbase.h>
#include <ctool.h>

#include "h\common.h"
#include "h\error.h"
#include "h\lex.h"
#include "h\parse.h"
#include "h\symbol.h"
#include "h\semantic.h"

class CMyTool : public CTool {
//	enum { kExitAbort=kExitSyntax+1 };
	enum { L_ADDRESS=L_BAD+1, L_BATCH, L_INTERRUPT };

	static KeyWord pTable[];

	Boolean	fBatch;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "a",		-L_ADDRESS	},
	{ "address",	-L_ADDRESS	},
	{ "b",		L_BATCH		},
	{ "batch",	L_BATCH		},
	{ "i",		-L_INTERRUPT	},
	{ "interrupt",	-L_INTERRUPT	},
	};


CMyTool	tool("PDI", "1.0à4", "1990-1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fBatch = true;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	char	*endptr;

	switch (option) {
		case L_ADDRESS:
			dev.baseAddress = (int) strtol(argument, &endptr, 10);
			return (true);
		case L_BATCH:
			fBatch = false;
			return (true);
		case L_INTERRUPT:
			dev.interruptLevel = atoi(argument);
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	if (argc != 1) {
		Help();
		return (kExitSyntax);
		}
	return (Parse(*argv, fBatch) ? kExitOK : kExitSyntax);
	}
