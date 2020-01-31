// ver     description
// --------------------------------------------------------------------------
// 1.6   - Added the directives .init and .cleanup; default saver to 0;
//         recognize textual color specifications listed with the .color
//         directive.
// 1.5.2 - use new CTimer class
// 1.5.1 - allow shift keys to wake up screen saver; remember previous
//         selector after wake up; merged functions into class CMenu; new
//         exit status of kExitLogout; lose concept of 'MENU' variable
// 1.5   - use base library; flush input queue before displaying menu
//         choices; base batch file name on DOS environment variable 'MENU'
//         if it is defined
// 1.4   - use "exit" status if running versions prior to DOS 3.30 (no calls
//         were implemented back then); implement menu, vmenu and nmenu;
//         changed exit codes for efficiency
// 1.3   - use classes; ability to disable saver; take out key assignments
// 1.2.1 - handle different screen sizes
// 1.2   - allow functions to be assigned to key presses
// 1.1   - new syntax; exit option
// 1.0.8 - specifiable name of data file; colors now are parameters
// 1.0.7 - fix improper operation on IBM PC running PC-DOS v2.10
// 1.0.6 - uses a batch file to invoke menu selector; runs on any hardware
// 1.0.5 - looks for environment variable MENU to change colors
// 1.0.4 - creates batch file to execute commands; thus, 0 bytes used
// 1.0.3 - allow attributes to be specified at command line
// 1.0.2 - optimize routines to use less memory
// 1.0.1 - add ability to have multiple commands per choice
// 1.0   - original program that lists and executes user selections

#include <ctype.h>
#include <dir.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <ckeyin.h>
#include <ctime.h>
#include <ctoken.h>

#include "h\cvideo.h"

const char	kCopyright[] = "MENU  Version 1.6  Copyright (c) 1988-1993 Vorne Industries Incorporated\n";
const int	kMaxBuff = 512;

class CMenu {
public:
	enum ExitCode {
		kExit_Cancel,	// escape key pressed to exit menu
		kExit_Do,	// execute commands and return to menu
		kExit_Once,	// do not return to menu program when done
		kExit_Logout	// perform logout procedure and do not return
		};
private:
	enum { kMaxItems=23 };

	CTimer	fTimer;
	CVideo	fVideo;
	size_t	fNumMenuItems;
	size_t	fLenMenuItems;
	char	*fMenuItem[kMaxItems];
	char	*fMenuCommand[kMaxItems];
	char	*fMenuInit;
	char	*fMenuCleanup;

	void		DoCommand(char *cmdStr);
	ExitCode	DoCommandParse(char *source, ExitCode code);
	void		Exit(ExitCode code, const char *message=0);
	int		GetColor(char *buff);
	short		GetKey();
	void		Read();
public:
	CMenu();
	~CMenu() { delete fMenuInit; delete fMenuCleanup; }

	void	Start();
	};


// Menu begins executing here.  We verify that proper version of DOS is
// running, read in the menu data file, display the menu and wait for key
// commands.

void main() {
	static CMenu	menu;

	menu.Start();
	}


// ***********************
// * CMenu Class Methods *
// ***********************

CMenu::CMenu() : fTimer(0) {
	fNumMenuItems = 0;
	fLenMenuItems = 0;
	fMenuInit = 0;
	fMenuCleanup = 0;
	}


void CMenu::DoCommand(char *cmdStr) {
	ExitCode code = ((_osmajor * 100 + _osminor) < 330) ? kExit_Once : kExit_Do;
	if (fMenuInit)
		code = DoCommandParse(fMenuInit, code);
	code = DoCommandParse(cmdStr, code);
	if (fMenuCleanup)
		code = DoCommandParse(fMenuCleanup, code);
	if (code == kExit_Do) {
		int		disk;
		static char	dir[MAXDIR];

		disk = getdisk();
		getcurdir(disk+1, dir);
		cout << (char)('A'+disk) << ":\n";
		cout << "CD \\" << dir << '\n';
		}
	Exit(code);
	}


CMenu::ExitCode CMenu::DoCommandParse(char *source, ExitCode code) {
	enum { L_BAD, L_EXIT, L_LOGOUT };
	static KeyWord	table[] = {
				{ "exit",	L_EXIT		},
				{ "logout",	L_LOGOUT	},
				};
	CToken		tokens(table, NUMELE(table));

	for (source=strtok(source, ";"); source; source=strtok(0, ";"))
		switch (tokens.Check(source, L_BAD, true)) {
			case L_EXIT:	code = kExit_Once;	break;
			case L_LOGOUT:	code = kExit_Logout;	break;
			default:	cout << source << '\n';	break;
			}
	return (code);
	}


void CMenu::Exit(ExitCode code, const char *message) {
	if (code == kExit_Cancel)
		cerr << kCopyright;
	if (message)
		cerr << message;
	exit(code);
	}


int CMenu::GetColor(char *buff) {
	enum {	L_BLACK,	L_BLUE,		L_GREEN,	L_CYAN,
		L_RED,		L_MAGENTA,	L_BROWN,	L_LGRAY,
		L_DGRAY,	L_LBLUE,	L_LGREEN,	L_LCYAN,
		L_LRED,		L_LMAGENTA,	L_YELLOW,	L_WHITE,
		L_BAD
		};
	static KeyWord	t_colors[] = {
				{ "black",	L_BLACK		},
				{ "blue",	L_BLUE		},
				{ "brown",	L_BROWN		},
				{ "cyan",	L_CYAN		},
				{ "dgray",	L_DGRAY		},
				{ "green",	L_GREEN		},
				{ "lblue",	L_LBLUE		},
				{ "lcyan",	L_LCYAN		},
				{ "lgray",	L_LGRAY		},
				{ "lgreen",	L_LGREEN	},
				{ "lmagenta",	L_LMAGENTA	},
				{ "lred",	L_LRED		},
				{ "magenta",	L_MAGENTA	},
				{ "red",	L_RED		},
				{ "white",	L_WHITE		},
				{ "yellow",	L_YELLOW	},
				};
	CToken		colors(t_colors, NUMELE(t_colors));
	int		val;

	buff = strtok(buff, " \t");
	if (isdigit(*buff))
		return (atoi(buff));
	val = colors.Check(buff, L_BAD, true);
	if (val == L_BAD) {
		strcat(buff, " -- unrecognized color.\n");
		Exit(kExit_Cancel, buff);
		}
	return (val);
	}


short CMenu::GetKey() {
	CKeyIn	keyboard;

	fTimer.Reset();
	while (!keyboard.Avail()) {
		if (fTimer.Get() && fTimer.Expired()) {
			fVideo.ClearScreen();
			Word status = keyboard.Status();
			while (!keyboard.Avail() && status==keyboard.Status())
				;
			keyboard.Flush();
			return (kNull);
			}
		}
	return (keyboard.Read());
	}


void CMenu::Read() {
	enum {	L_BAD, L_CLEANUP, L_COLOR, L_INIT, L_SAVER };
	static KeyWord	t_directives[] = {
				{ ".cleanup",	L_CLEANUP	},
				{ ".color",	L_COLOR		},
				{ ".init",	L_INIT		},
				{ ".saver",	L_SAVER		},
				};
	CToken		directives(t_directives, NUMELE(t_directives));
	static char	buff[kMaxBuff];
	short		low, high;

	while (true) {
		cin >> ws;
		if (!cin.getline(buff, sizeof(buff)))
			break;
		if (buff[0] == ';')
			continue;
		else if (buff[0] == '.') {
			switch (directives.Check(buff, L_BAD, true)) {
				case L_CLEANUP:
					delete fMenuCleanup;
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					fMenuCleanup = strdup(buff);
					break;
				case L_COLOR:
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					low = GetColor(buff);
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					high = GetColor(buff);
					fVideo.SetAttr(low, high);
					break;
				case L_INIT:
					delete fMenuInit;
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					fMenuInit = strdup(buff);
					break;
				case L_SAVER:
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					fTimer.Set(0, atoi(buff), 0);
					break;
				default:
					strcat(buff, " -- unrecognized directive.\n");
					Exit(kExit_Cancel, buff);
					break;
				}
			}
		else if (fNumMenuItems < NUMELE(fMenuItem)) {
			fMenuItem[fNumMenuItems] = strdup(buff);
			if (buff[0] == '-')
				fMenuCommand[fNumMenuItems++] = "";
			else {
				fLenMenuItems = max(strlen(buff), fLenMenuItems);
				cin >> ws;
				cin.getline(buff, sizeof(buff));
				fMenuCommand[fNumMenuItems++] = strdup(buff);
				}
			}
		}
	if (fNumMenuItems < 1)
		Exit(kExit_Cancel, "No items listed in menu data file.\n");
	}


void CMenu::Start() {
	CKeyIn	keyboard;
	short	top, lef, bot, rig, i, curPtr, nextPtr, len, ch;
	Boolean	done;

	Read();
	keyboard.Flush();
	top = ((fVideo.GetRows()-2) / 2) - (fNumMenuItems / 2);
	bot = top + fNumMenuItems + 1;
	lef = ((fVideo.GetCols()-2) / 2) - 1 - (fLenMenuItems / 2);
	rig = lef + fLenMenuItems + 3;
	len = fLenMenuItems+2;
	curPtr = -1;
	nextPtr = 0;
	while (true) {
		fVideo.ClearScreen();
		fVideo.Box(top, lef, bot, rig, kAttrLow);
		for (i=0; i<fNumMenuItems; i++) {
			if (*fMenuItem[i] == '-')
				fVideo.PutCharAttr(top+i+1, lef+1, 'Ä', kAttrLow, len);
			else
				fVideo.PutStrAttr(top+i+1, lef+2, fMenuItem[i], kAttrHigh);
			}
		done = false;
		while (!done) {
			if (nextPtr != curPtr) {
				if (curPtr >= 0)
					fVideo.PutAttr(top+1+curPtr, lef+1, kAttrHigh, len);
				curPtr = nextPtr;
				fVideo.PutAttr(top+1+curPtr, lef+1, kAttrInv, len);
				}
			fVideo.HideCursor();
			switch (ch = GetKey()) {
				case kEscape:
					fVideo.ClearScreen();
					Exit(kExit_Cancel);
					break;
				case kHome:
				case kPageUp:
					nextPtr = 0;
					break;
				case kEnd:
				case kPageDown:
					nextPtr = fNumMenuItems-1;
					break;
				case kUp:
					if (curPtr > 0)
						nextPtr--;
					if (*fMenuItem[nextPtr] == '-')
						nextPtr--;
					break;
				case kDown:
					if (curPtr < fNumMenuItems-1)
						nextPtr++;
					if (*fMenuItem[nextPtr] == '-')
						nextPtr++;
					break;
				case kSpace:
					if (curPtr == fNumMenuItems-1)
						nextPtr = 0;
					else {
						nextPtr++;
						if (*fMenuItem[nextPtr] == '-')
							nextPtr++;
						}
					break;
				case kReturn:
					fVideo.ClearScreen();
					DoCommand(fMenuCommand[curPtr]);
					break;
				case kNull:
					curPtr = -1;
					done = true;
					break;
				default:
					ch = toupper(ch);
					nextPtr = curPtr;
					i = 1;
					do {
						if (nextPtr < fNumMenuItems-1)
							nextPtr++;
						else
							nextPtr = 0;
						} while ((++i < fNumMenuItems) && (ch != toupper(*fMenuItem[nextPtr])));
					if (ch != *fMenuItem[nextPtr])
						nextPtr = curPtr;
					break;
				}
			}
		}
	}
