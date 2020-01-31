// íkeywords: flush.cpp 1.0 wed 03-17-93 11:40:07í
// ílgbí
// 1.0     JACKN     wed 03-17-93 11:40:07
//         Original program; 9303.08
// ílgeí
// ínokeywordsí

// Flush - flushes the keyboard buffer.

static char *pHelp[] = {
"The Flush command flushes the keyboard input buffer.\n\n",
"Syntax: Flush [/?]\n\n",
"Status: Flush can return the following status codes:\n",
"          0       No errors.\n\n",
"Options: /?       Displays this help text.\n"
};

// Examples
//	The following example flushes the keyboard input buffer.
//
//	Flush

#include <ctool.h>
#include <ckeyin.h>

class CMyTool : public CTool {
	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


CMyTool	tool("Flush", "1.0", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	}


Boolean	CMyTool::DoOption(LToken /*option*/, const char * /*argument*/) {
	return (false);
	}


int CMyTool::DoWork(short /*argc*/, const char ** /*argv*/) {
	CKeyIn	keyboard;

	keyboard.Flush();
	return (kExitOK);
	}
