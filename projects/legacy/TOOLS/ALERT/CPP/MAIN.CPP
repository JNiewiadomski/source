static char *pHelp[] = {
"The Alert command displays an alert box containing the prompt message until\n",
"the ENTER key is pressed.  If the message parameter is not specified, standard\n",
"input is used.\n",
"\n",
"Syntax: Alert [message] [options]\n",
"\n",
"Status: Alert can return the following status codes:\n",
"          0         No errors.\n",
"          1         Syntax error.\n",
"          2         Aborted by user.\n",
"          3         Out of memory.\n",
"\n",
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
// 10-14-94  jn  v2.0    Overhaul of the window mechanism.
// 03-05-93  jn  v1.2.2  Use new Base library; fix blank line bug.
// 09-26-91  jn  v1.2.1  Correctly centers on multi-sized screens.
// 08-26-91  jn  v1.2    Exists with error code on Ctrl-C or Ctrl-Break.
// 05-30-91  jn  v1.1    Update to conform to new COMMON library.
// 01-28-91  jn  v1.0    Original program.

#include <dos.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>
#include <strstrea.h>

#include <base\include\cbreak.h>
#include <base\include\ckeyin.h>
#include <base\include\ctime.h>
#include <base\include\ctool.h>

#include "h\cwindow.h"

class CMyTool : public CTool {
	enum { kExitAbort=kExitSyntax+1, kExitOutOfMemory };
	enum { L_BORDER=L_BAD+1, L_COLOR, L_HEIGHT, L_SILENT, L_WIDTH };

	static KeyWord pTable[];

	int	fBorder, fColor, fHeight, fWidth;
	Boolean	fSilent;
	CBreak	fCtrlBreak;
	char	fBuff[1024];

	int	Alert(istream &in);
	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
	char	*Duplicate(char *s);
	char	**Read(istream &in, int height, int width);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "b",		-L_BORDER	},
	{ "border",	-L_BORDER	},
	{ "c",		-L_COLOR	},
	{ "color",	-L_COLOR	},
	{ "h",		-L_HEIGHT	},
	{ "height",	-L_HEIGHT	},
	{ "s",		 L_SILENT	},
	{ "silent",	 L_SILENT	},
	{ "w",		-L_WIDTH	},
	{ "width",	-L_WIDTH	},
	};


CMyTool	tool("Alert", "2.0", "1991-1994");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));
	fBorder = CWindow::kDouble;
	fColor = RED;
	fHeight = 7;
	fSilent = false;
	fWidth = 52;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_BORDER:
			fBorder = atoi(argument);
			return (true);
		case L_COLOR:
			fColor = atoi(argument);
			return (true);
		case L_HEIGHT:
			fHeight = atoi(argument);
			return (true);
		case L_SILENT:
			fSilent = true;
			return (true);
		case L_WIDTH:
			fWidth = atoi(argument);
			return (true);
		}
	return (false);
	}

int CMyTool::DoWork(short argc, const char **argv) {
	//
	// If no parameters specified, copy stdin to character buffer.
	// Otherwise, copy parameters to character buffer.
	//
	if (argc == 0)
		return Alert(cin);
	else {
		strstream	io(fBuff, sizeof(fBuff), ios::in | ios::out);

		while (argc--) {
			io << *argv++;
			if (argc)
				io << ' ';
			}
		io << ends;
		return Alert(io);
		}
	}


int CMyTool::Alert(istream &in) {
	const char	kEnter[] = "[® Enter ¯]";
	CWindow		window;
	CKeyIn		keyboard;
	Boolean		done=false;
	int		status = kExitOK;
	char		**buff;

//	buff = Read(in, fHeight-2, fWidth-4);
	if (!window.Open(fHeight, fWidth, fBorder, WHITE, fColor)) {
		Help();
		return (kExitSyntax);
		}
//	window.Write(buff);
	window << in;
	window.Goto(fHeight-2, fWidth-sizeof(kEnter)-1);
	window << kEnter;

	if (!fSilent) {
		CTimer fTimer(20);

		sound(440);
		while (!fTimer.Expired())
			;
		nosound();
		}

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


char **CMyTool::Read(istream &in, int height, int width) {
	char	**buff, **current, *s, *t, save_ch;
	int	len, num_lines;

	buff = new (char *[height]);
	if (!buff)
		return (0);
	for (current=buff, num_lines=height; num_lines--; )
		*current++ = 0;
	for (current=buff, num_lines=height; num_lines; ) {
		if (!in.getline(fBuff, sizeof(fBuff)))
			break;
		while ((s = strpbrk(fBuff, "\t")) != 0)
			*s = ' ';
		for (s=strtok(fBuff, "\v"); s; num_lines--, s=strtok(0, "\v")) {
			for (len=strlen(s); len>width && num_lines; num_lines--) {
				save_ch = *(s+width);
				*(s+width) = '\0';
				t = strrchr(s, ' ');
				if (t) {
					*(s+width) = save_ch;
					*t = '\0';
					*current++ = Duplicate(s);
					s = t + 1;
					}
				else {
					*current++ = Duplicate(s);
					s += width;
					*s = save_ch;
					}
				len = strlen(s);
				}
			if (num_lines)
				*current++ = Duplicate(s);
			}
		}
	return (buff);
	}


char *CMyTool::Duplicate(char *s) {
	char	*buff;

	buff = new char[strlen(s)+1];
	if (!buff) {
		cout << "Out of memory.\n";
		exit(kExitOutOfMemory);
		}
	strcpy(buff, s);
	return (buff);
	}