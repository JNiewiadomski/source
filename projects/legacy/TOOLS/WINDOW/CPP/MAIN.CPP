static char *pHelp[] = {
"The Alert command displays an alert box containing the prompt message until\n",
"the ENTER key is pressed.  If the message parameter is not specified, standard\n",
"input is used.\n\n",
"Syntax: Alert [message] [options]\n\n",
"Status: Alert can return the following status codes:\n",
"          0         No errors.\n",
"          1         Syntax error.\n",
"          2         Aborted by user.\n\n",
"Options: /?         Displays this help text.\n",
"         /border #  Border style (0 to 4).\n",
"         /color #   Specifies the border color.\n",
"         /height #  Specifies rectangle height.\n",
"         /silent    Does not sound bell.\n",
"         /width #   Specified rectangle width.\n"
};

// Examples
//	The following example displays an alert box and waits for you to
//	press the ENTER key.
//
//	Alert Please insert next disk to be searched.
//
// History
// --------------------------------------------------------------------------
// 9410.14  jn  v1.0  Original program.

#include <dos.h>
#include <iostream.h>
#include <string.h>
#include <strstrea.h>

//#include <base\include\calert.h>
//#include <base\include\cascii.h>
#include <base\include\cbreak.h>
//#include <base\include\ctime.h>
#include <base\include\ckeyin.h>
#include <base\include\ctool.h>

#include "h\cwindow.h"

class CMyTool : public CTool {
	enum { kExitAbort=kExitSyntax+1 };
//	enum { L_SILENT=L_BAD+1 };

//	static KeyWord pTable[];

//	Boolean	fSilent;
	CBreak	fCtrlBreak;
//	CAlert	fAlert;
	char	fBuff[1024];

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
	int	Window(istream &in);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


//KeyWord CMyTool::pTable[] = {
//	{ "silent",	L_SILENT	},
//	};


CMyTool	tool("Window", "1.0", "1994");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
//	fOptions.Init(pTable, NUMELE(pTable));
//	fSilent = false;
	}


Boolean	CMyTool::DoOption(LToken /*option*/, const char * /*argument*/) {
//	switch (option) {
//		case L_SILENT:
//			fSilent = true;
//			return (true);
//		}
	return (false);
	}

int CMyTool::DoWork(short argc, const char **argv) {
	//
	// If no parameters specified, copy stdin to character buffer.
	// Otherwise, copy parameters to character buffer.
	//
	if (argc == 0)
		return Window(cin);
	else {
		strstream	io(fBuff, sizeof(fBuff), ios::in | ios::out);

		while (argc--) {
			io << *argv++;
			if (argc)
				io << ' ';
			}
		io << ends;
		return Window(io);
		}
	}


int CMyTool::Window(istream &in) {
	const int	kHeight=15, kWidth=70;
	const char	kPressEnter[] = "[® Enter ¯]";
	CWindow		window;
	CKeyIn		keyboard;
	Boolean		done=false;
	int		status = kExitOK;

	window.Open(kHeight, kWidth, CWindow::kSingle, WHITE, LIGHTBLUE);
	window << in;
	window.Goto(kHeight-2, kWidth-sizeof(kPressEnter)-1);
	window << kPressEnter;

	keyboard.Flush();
	while (!done) {
		switch (keyboard.Read()) {
			case kReturn:
				done = true;
				break;
			case kCtrlC:
			case kCtrlBrk:
				status = kExitAbort;
				done = true;
				break;
			}
		}
	window.Close();
	return (status);
	}
