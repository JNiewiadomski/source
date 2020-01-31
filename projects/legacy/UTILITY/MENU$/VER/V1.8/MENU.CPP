// ver     description
// --------------------------------------------------------------------------
// 1.8   - Allow menu commands to span more than one line -- they must start
//         with a white space character. Menu items must not start with white
//         space. Enhance checks for errors.
// 1.7.2 - Write border characters for dividing line as well; implement hot
//         keys: characters within curly braces ({}) will be highlighted and
//         pressing the first character of a hot key item will execute those
//         commands.
// 1.7.1 - Deleted MENUDEL.BAT file and implemented same functionality within
//         VMENU.BAT, so now one less batch file is needed.
// 1.7   - Added the directive .alias.
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

#include <base\include\ckeyin.h>
#include <base\include\ctime.h>
#include <base\include\ctoken.h>

#include "h\calias.h"
#include "h\cvideo.h"

const char	kCopyright[] = "MENU  Version 1.8  Copyright (c) 1988-1994 Vorne Industries Incorporated\n";
const int	kMaxBuff = 8192;

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
	CAlias	fAlias;

	void		DoCommand(char *cmdStr);
	ExitCode	DoCommandParse(char *source, ExitCode code);
	void		Exit(ExitCode code, const char *message=0);
	int		GetColor(char *buff);
	short		GetKey();
	void		Print(const char *text, short row, short col, short len, Boolean inverse);
	void		Read();
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
	const char	*alias;

	for (source=strtok(source, ";"); source; source=strtok(0, ";"))
		switch (tokens.Check(source, L_BAD, true)) {
			case L_EXIT:	code = kExit_Once;	break;
			case L_LOGOUT:	code = kExit_Logout;	break;
			default:
				alias = fAlias.Get(source);
				if (alias)
					cout << alias;
				else
					cout << source;
				cout << '\n';
				break;
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


void CMenu::Print(const char *text, short row, short col, short len, Boolean inverse) {
	short	attr;
	char	ch;

	attr = inverse ? kAttrInvHigh : kAttrHigh;
	fVideo.PutAttr(row, col++, attr, 1);
	len--;
	while (*text) {
		ch = *text++;
		if (ch == '{')
			attr = inverse ? kAttrInvLow : kAttrLow;
		else if (ch == '}')
			attr = inverse ? kAttrInvHigh : kAttrHigh;
		else {
			fVideo.PutCharAttr(row, col++, ch, attr, 1);
			len--;
			}
		}
	if (len)
		fVideo.PutAttr(row, col, attr, len);
	}


void CMenu::Read() {
	enum {	L_BAD, L_ALIAS, L_CLEANUP, L_COLOR, L_INIT, L_SAVER };
	static KeyWord	t_directives[] = {
				{ ".alias",	L_ALIAS		},
				{ ".cleanup",	L_CLEANUP	},
				{ ".color",	L_COLOR		},
				{ ".init",	L_INIT		},
				{ ".saver",	L_SAVER		},
				};
	CToken		directives(t_directives, NUMELE(t_directives));
	static char	buff[kMaxBuff];

	while (true) {
//		cin >> ws;
		if (!cin.getline(buff, sizeof(buff)))
			break;
		if (*buff == '\0' || *buff == ';')
			continue;
		else if (*buff == '.') {
			switch (directives.Check(buff, L_BAD, true)) {
				case L_ALIAS:
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					if (!fAlias.Add(buff)) {
						strcat(buff, " -- syntax error or out of alias symbol table space.\n");
						Exit(kExit_Cancel, buff);
						}
					break;
				case L_CLEANUP:
					free(fMenuCleanup);
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					fMenuCleanup = strdup(buff);
					break;
				case L_COLOR:
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					short low = GetColor(buff);
					cin >> ws;
					cin.getline(buff, sizeof(buff));
					short high = GetColor(buff);
					fVideo.SetAttr(low, high);
					break;
				case L_INIT:
					free(fMenuInit);
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
		else if (*buff == ' ' || *buff == '\t') {
			strcat(buff, " -- expecting menu item.\n");
			Exit(kExit_Cancel, buff);
			}
		else if (fNumMenuItems < NUMELE(fMenuItem)) {
			fMenuItem[fNumMenuItems] = strdup(buff);
			if (*buff == '-') {
				if (fNumMenuItems == 0) {
					strcat(buff, " -- cannot be first menu item.\n");
					Exit(kExit_Cancel, buff);
					}
				if (*fMenuItem[fNumMenuItems-1] == '-') {
					strcat(buff, " -- cannot be consecutive menu item.\n");
					Exit(kExit_Cancel, buff);
					}
				fMenuCommand[fNumMenuItems++] = "";
				}
			else {
				int	len;
				char	ch, *s;

				fLenMenuItems = max(strlen(buff), fLenMenuItems);
				for (*buff='\0'; ; ) {
					if (!cin.get(ch))
						break;
					if (ch != ' ' && ch != '\t') {
						cin.putback(ch);
						break;
						}
					len = strlen(buff);
					s = buff + len;
					if (len) {
						*s++ = ';';
						len++;
						}
					cin >> ws;
					if (!cin.getline(s, sizeof(buff)-len))
						break;
					}
				fMenuCommand[fNumMenuItems++] = strdup(buff);
				}
			}
		else {
			strcat(buff, " -- maximum number of menu items exceeded.\n");
			Exit(kExit_Cancel, buff);
			}
		}
	if (fNumMenuItems < 1)
		Exit(kExit_Cancel, "No items listed in menu data file.\n");
	if (*fMenuItem[fNumMenuItems-1] == '-') {
		strcpy(buff, fMenuItem[fNumMenuItems-1]);
		strcat(buff, " -- cannot be last menu item.\n");
		Exit(kExit_Cancel, buff);
		}
	}


void CMenu::Start() {
	CKeyIn	keyboard;
	short	top, lef, bot, rig, i, curPtr, nextPtr, len, ch;
	Boolean	done;
	char	*s;

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
			if (*fMenuItem[i] == '-') {
				fVideo.PutCharAttr(top+i+1, lef, 'Ç', kAttrLow, 1);
				fVideo.PutCharAttr(top+i+1, lef+1, 'Ä', kAttrLow, len);
				fVideo.PutCharAttr(top+i+1, rig, '¶', kAttrLow, 1);
				}
			else
				Print(fMenuItem[i], top+i+1, lef+1, len, false);
//				fVideo.PutStrAttr(top+i+1, lef+2, fMenuItem[i], kAttrHigh);
			}
		done = false;
		while (!done) {
			if (nextPtr != curPtr) {
				if (curPtr >= 0)
					Print(fMenuItem[curPtr], top+1+curPtr, lef+1, len, false);
//					fVideo.PutAttr(top+1+curPtr, lef+1, kAttrHigh, len);
				curPtr = nextPtr;
				Print(fMenuItem[curPtr], top+1+curPtr, lef+1, len, true);
//				fVideo.PutAttr(top+1+curPtr, lef+1, kAttrInv, len);
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
					//
					// search for hot keys
					//
					i = 0;
					do {
						s = strchr(fMenuItem[i], '{');
						if (s && toupper(s[1]) == ch)
							break;
						} while ((++i < fNumMenuItems));
					if (i < fNumMenuItems) {
						fVideo.ClearScreen();
						DoCommand(fMenuCommand[i]);
						}
					//
					// search for first letter
					//
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
