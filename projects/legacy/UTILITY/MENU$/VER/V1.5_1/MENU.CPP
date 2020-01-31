// ver     description
// --------------------------------------------------------------------------
// 1.5.1 - allow shift keys to wake up screen saver; remember previous
//         selector after wake up; merged functions into class CMenu; new
//         exit status of kExitLogout
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
#include <ctimer.h>

#include "h\cvideo.h"

const char	kCopyright[] = "MENU  Version 1.5.1  Copyright (c) 1988-1993 Vorne Industries Incorporated\n";
const int	kMaxBuff = 512;

class CMenu {
	enum { kMaxItems=23 };
	enum ExitCode {
		kExit_Cancel,	// escape key pressed to exit menu
		kExit_Do,	// execute commands and return to menu
		kExit_Once,	// do not return to menu program when done
		kExit_Logout	// perform logout procedure and do not return
		};

	long	fSaverDelay;
	CVideo	fVideo;
	size_t	fNumMenuItems;
	size_t	fLenMenuItems;
	char	*fMenuItem[kMaxItems];
	char	*fMenuCommand[kMaxItems];

	void	DoCommand(char *cmdStr);
	void	Exit(ExitCode code, const char *message=0);
	short	GetKey();
	void	Read();
public:
	CMenu();

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

CMenu::CMenu() {
	fSaverDelay = 66000L;		// default to 11 minutes
	fNumMenuItems = 0;
	fLenMenuItems = 0;
	}


void CMenu::DoCommand(char *cmdStr) {
	char		*t;
	static char	command[kMaxBuff];
	ExitCode	code;

	code = ((_osmajor * 100 + _osminor) < 330) ? kExit_Once : kExit_Do;
	while (*cmdStr) {
		t = command;
		while (*cmdStr && *cmdStr != ';')
			*t++ = *cmdStr++;
		*t++ = '\n';
		*t = '\0';
		if (strcmpi(command, "exit\n") == 0) {
			*cmdStr = '\0';
			code = kExit_Once;
			}
		else if (strcmpi(command, "logout\n") == 0) {
			*cmdStr = '\0';
			code = kExit_Logout;
			}
		else {
			if (*cmdStr)
				cmdStr++;
			cout << command;
			}
		}
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


void CMenu::Exit(ExitCode code, const char *message) {
	if (code == kExit_Cancel)
		cerr << kCopyright;
	if (message)
		cerr << message;
	exit(code);
	}


short CMenu::GetKey() {
	CTimer	next_time(fSaverDelay);
	CKeyIn	keyboard;

	while (!keyboard.Avail()) {
		if (fSaverDelay && next_time.Elapsed()) {
			fVideo.ClearScreen();
			Word status = keyboard.Status();
			while (!keyboard.Avail() && status==keyboard.Status())
				;
			if (keyboard.Avail())
				keyboard.Read();
			return (kNull);
			}
		}
	return (keyboard.Read());
	}


void CMenu::Read() {
	static char	buff[kMaxBuff];
	short		low, high;

	while (true) {
		cin >> ws;
		if (!cin.getline(buff, sizeof(buff)))
			break;
		if (buff[0] == ';')
			continue;
		else if (buff[0] == '.') {
			if (strcmpi(buff, ".color") == 0) {
				cin.getline(buff, sizeof(buff));
				low = atoi(buff);
				cin.getline(buff, sizeof(buff));
				high = atoi(buff);
				fVideo.SetAttr(low, high);
				}
			else if (strcmpi(buff, ".saver") == 0) {
				cin.getline(buff, sizeof(buff));
				fSaverDelay = (long)atoi(buff) * 6000L;
				}
			else
				Exit(kExit_Cancel, "Unrecognized directive (.color, .saver).\n");
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
