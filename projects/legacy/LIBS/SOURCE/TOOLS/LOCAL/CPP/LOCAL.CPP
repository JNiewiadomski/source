// íkeywords: local.cpp 1.1 mon 03-22-93 09:37:12í
// ílgbí
// 1.0     JACKN     mon 03-22-93 09:33:36
//         Original program; 9302.03
// 1.1     JACKN     mon 03-22-93 09:37:12
//         Return available local drive without changing; 9302.09
// ílgeí
// ínokeywordsí

#include <ctool.h>

#include "h\system.h"

static char *pHelp[] = {
"Makes the local drive the current drive.  If drive C: exists, it becomes the\n",
"current drive.  Otherwise drive A: becomes the current drive.\n\n",
"Syntax:  Local [/?] [/test]\n\n",
"Status:  Local may return the following status codes:\n",
"          0    Local drive A:.\n",
"          2    Local drive C:.\n\n",
"Options: /?    Displays this help text.\n",
"         /test Return status without changing drives.\n"
};

class CMyTool : public CTool {
//	enum { kExitIO=kExitSyntax+1 };
	enum { L_TEST=L_BAD+1 };

	static KeyWord pTable[];

	Boolean	fTest;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "test",	L_TEST	},
	};


CMyTool	tool("Local", "1.1", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fTest = false;
	}


Boolean	CMyTool::DoOption(LToken option, const char * /*argument*/) {
	switch (option) {
		case L_TEST:
			fTest = true;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char ** /*argv*/) {
	if (argc != 0)
		Help();
	EquipInit();
	int drive = (equip.drives < 3) ? 0 : 2;
	if (!fTest) {
		setdisk(drive);
		chdir("\\");
		}
	return (drive);
	}
