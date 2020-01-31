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
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.2.2  9303.05  jn      Use new Base library; fix blank line bug.
// 1.2.1  9109.26  jn      Correctly centers on multi-sized screens.
// 1.2    9108.26  jn      Exists with error code on Ctrl-C or Ctrl-Break.
// 1.1    9105.30  jn      Update to conform to new COMMON library.
// 1.0    9101.28  jn      Original program.

#include <dos.h>
#include <iostream.h>
#include <string.h>

#include <calert.h>
#include <cascii.h>
#include <cbreak.h>
#include <ctimer.h>
#include <ctool.h>
#include <ckeyin.h>

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

	fCtrlBreak.Init();
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
		fTimer.Wait(0);
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
	fCtrlBreak.CleanUp();
	return (exit_status);
	}
