// íkeywords: alert.cpp 1.4 wed 03-17-93 09:25:23í
// ílgbí
// 1.0     JACKN     wed 03-17-93 09:12:24
//         Original program; 9101.28
// 1.1     JACKN     wed 03-17-93 09:14:49
//         Update to conform to new COMMON library; 9105.30
// 1.2     JACKN     wed 03-17-93 09:20:49
//         Exists with error code on Ctrl-C or Ctrl-Break; 9108.26
// 1.3     JACKN     wed 03-17-93 09:22:50
//         Correctly centers on multi-sized screens; 9109.26
// 1.4     JACKN     wed 03-17-93 09:25:23
//         Use new Base library; fix blank line bug; 9303.05
// ílgeí
// ínokeywordsí

// Alert - display an alert box.

static char *pHelp[] = {
"The Alert command displays an alert box containing the prompt message until\n",
"the ENTER key is pressed.  If the message parameter is not specified, standard\n",
"input is used.\n\n",
"Syntax: Alert [message] [/?] [/silent]\n\n",
"Status: Alert can return the following status codes:\n",
"          0       No errors.\n",
"          1       Syntax error.\n",
"          2       Aborted by user.\n\n",
"Options: /?       Displays this help text.\n",
"         /silent  Runs silently.  By default, Alert beeps when the dialog box\n",
"                  is displayed.\n"
};

// Examples
//	The following example displays an alert box and waits for you to
//	press the ENTER key.
//
//	Alert Please insert next disk to be searched.

#include <dos.h>
#include <iostream.h>
#include <string.h>

#include <base\include\calert.h>
#include <base\include\cascii.h>
#include <base\include\cbreak.h>
#include <base\include\ctime.h>
#include <base\include\ctool.h>
#include <base\include\ckeyin.h>

const char	kPressEnter[] = "Press ENTER to continue.";

class CMyTool : public CTool {
	enum { kExitAbort=kExitSyntax+1 };
	enum { L_SILENT=L_BAD+1 };

	static KeyWord pTable[];

	Boolean	fSilent;
	CBreak	fCtrlBreak;
	CAlert	fAlert;
	char	fBuff[CAlert::kBufSize];

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "silent",	L_SILENT	},
	};


CMyTool	tool("Alert", "1.2.2", "1991-1993");


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
	char	*s;
	short	i;
	CKeyIn	keyboard;
	Boolean	done;
	int	exit_status=kExitOK;

	//
	// If no parameters specified, copy stdin to character buffer.
	// Otherwise, copy parameters to character buffer.
	//
	if (argc == 0)
		cin.get(fBuff, sizeof(fBuff), kA_SUB);
	else
		for (*fBuff='\0', s=fBuff, i=0; i < argc; i++) {
			strcat(s, argv[i]);
			strcat(s, " ");
			}
	//
	// Open up a dialog box on the screen.
	//
	fAlert.Open(RED, WHITE);
	fAlert.Message(fBuff, kPressEnter);
	//
	// If silent parameter not specified, sound the speaker.
	//
	if (!fSilent) {
		CTimer fTimer(20);

		sound(440);
		while (!fTimer.Expired())
			;
		nosound();
		}
	//
	// Flush the keyboard buffer and wait for the ENTER key to be pressed.
	//
	keyboard.Flush();
	done = false;
	while (!done) {
		switch (keyboard.Read()) {
			case kReturn:
				done = true;
				break;
			case kCtrlC:
			case kCtrlBrk:
				exit_status = kExitAbort;
				done = true;
				break;
			}
		}
	//
	// Cleanup after ourselves.
	//
	fAlert.Close();
	return (exit_status);
	}
