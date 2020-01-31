// ver     description
// --------------------------------------------------------------------------
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
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <..\base\inc\base.h>
#include <..\base\inc\ckeyin.h>
#include <..\base\inc\ctimer.h>

#include "inc\cvideo.h"

enum {	kErr_Cancel,		// escape key pressed to exit menu
	kErr_Do,		// execute commands and return to menu
	kErr_Once,		// do not return to menu program when done
	kErr_DOS_Ver,		// invalid version of DOS
	kErr_Not_Found,		// menu data file not found
	kErr_Syntax,		// syntax error encountered
	kErr_No_Items,		// no items listed in data file
	kErr_Not_Write		// cannot open file for writing
	};

#define MAX_ITEMS	23
#define MAX_BUFF	512

static size_t	numMenuItems=0, lenMenuItems=0;
static char	*menuItem[MAX_ITEMS], *menuCommand[MAX_ITEMS];
static long	saverDelay=66000L;		// default to 11 minutes
static CVideo	video;


static void	DoCommand(char *cmdStr);
static short	InKey();
static void	MenuDo();
static char	*MenuGetLine(int fHand);
static void	MenuRead(const char *name);


// Menu begins executing here.  We verify that proper version of DOS is
// running, read in the menu data file, display the menu and wait for key
// commands.

int main(int argc, char *argv[]) {
	const char	kCopyright[]="MENU  Version 1.5  Copyright (c) 1988-1993 Vorne Industries Incorporated";

	if (_osmajor < 2)			// we require minimum DOS 2.00
		exit(kErr_DOS_Ver);
	MenuRead(argc > 1 ? argv[1] : "main.mnu");
	MenuDo();
	video.ClearScreen();
	video.PutStrAttr(0, 0, kCopyright, kAttrLow);
	video.Goto(1, 0);
	return (kErr_Cancel);
	}


static void DoCommand(char *cmdStr) {
	short	fHand, ch;
	char	*name, *t, command[MAX_BUFF];

	video.ClearScreen();
	if ((name = getenv("MENU")) == 0)
		name = "$.BAT";
	if ((fHand = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_TEXT, S_IREAD | S_IWRITE)) == -1)
		exit(kErr_Not_Write);
	ch = ((_osmajor*100+_osminor) < 330) ? kErr_Once : kErr_Do;
	while (*cmdStr) {
		t = command;
		while (*cmdStr && *cmdStr != ';')
			*t++ = *cmdStr++;
		*t++ = '\n';
		*t = '\0';
		if (strcmp(command, "exit\n")) {
			if (*cmdStr)
				cmdStr++;
			write(fHand, command, strlen(command));
			}
		else {
			*cmdStr = '\0';
			ch = kErr_Once;
			}
		}
	close(fHand);
	exit(ch);
	}


static short InKey() {
	CTimer	next_time(saverDelay);
	CKeyIn	keyboard;

	while (!keyboard.Avail()) {
		if (saverDelay && next_time.Elapsed()) {
			video.ClearScreen();
			while (!keyboard.Avail())
				;
			keyboard.Read();
			return (kNull);
			}
		}
	return (keyboard.Read());
	}


static void MenuDo() {
	CKeyIn	keyboard;
	short	top, lef, bot, rig, i, curPtr, nextPtr, len, ch;
	Boolean	done;

	while (keyboard.Avail())
		keyboard.Read();
	top = ((video.GetRows()-2) / 2) - (numMenuItems / 2);
	bot = top + numMenuItems + 1;
	lef = ((video.GetCols()-2) / 2) - 1 - (lenMenuItems / 2);
	rig = lef + lenMenuItems + 3;
	while (true) {
		video.ClearScreen();
		video.Box(top, lef, bot, rig, kAttrLow);
		len = lenMenuItems+2;
		for (i=0; i<numMenuItems; i++) {
			if (*menuItem[i] == '-')
				video.PutCharAttr(top+i+1, lef+1, 'Ä', kAttrLow, len);
			else
				video.PutStrAttr(top+i+1, lef+2, menuItem[i], kAttrHigh);
			}
		curPtr = -1;
		nextPtr = 0;
		done = false;
		while (!done) {
			if (nextPtr != curPtr) {
				if (curPtr >= 0)
					video.PutAttr(top+1+curPtr, lef+1, kAttrHigh, len);
				curPtr = nextPtr;
				video.PutAttr(top+1+curPtr, lef+1, kAttrInv, len);
				}
			video.HideCursor();
			switch (ch = InKey()) {
				case kEscape:
					return;
				case kHome:
				case kPageUp:
					nextPtr = 0;
					break;
				case kEnd:
				case kPageDown:
					nextPtr = numMenuItems-1;
					break;
				case kUp:
					if (curPtr > 0)
						nextPtr--;
					if (*menuItem[nextPtr] == '-')
						nextPtr--;
					break;
				case kDown:
					if (curPtr < numMenuItems-1)
						nextPtr++;
					if (*menuItem[nextPtr] == '-')
						nextPtr++;
					break;
				case kSpace:
					if (curPtr == numMenuItems-1)
						nextPtr = 0;
					else {
						nextPtr++;
						if (*menuItem[nextPtr] == '-')
							nextPtr++;
						}
					break;
				case kReturn:
					DoCommand(menuCommand[curPtr]);
					break;
				case kNull:
					done = true;
					break;
				default:
					ch = toupper(ch);
					nextPtr = curPtr;
					i = 1;
					do {
						if (nextPtr < numMenuItems-1)
							nextPtr++;
						else
							nextPtr = 0;
						} while ((++i < numMenuItems) && (ch != toupper(*menuItem[nextPtr])));
					if (ch != *menuItem[nextPtr])
						nextPtr = curPtr;
					break;
				}
			}
		}
	}


// MenuGetLine returns the next line of the menu data file.  0 is returned
// if EOF is reached.  Otherwise a pointer is returned to the buffer
// containing the line read in.

static char *MenuGetLine(int fHand) {
	const char	kComment=';';		// beginning of comment char
	char		ch, *s;
	static char	buff[MAX_BUFF+1];
	int		len;

	while (true) {
		if (eof(fHand) || read(fHand, &ch, sizeof(ch)) != sizeof(ch))
			return (0);
		if (ch == kComment)
			while (read(fHand, &ch, sizeof(ch)) == sizeof(ch) && ch != '\n')
				;
		else {
			while ((ch == ' ' || ch == '\t') && !eof(fHand))
				read(fHand, &ch, sizeof(ch));
			if (ch != '\n')
				break;
			}
		}
	for (s=buff, len=sizeof(buff)-1; ch!='\n' && len && !eof(fHand); len--) {
		*s++ = ch;
		read(fHand, &ch, sizeof(ch));
		}
	*s = '\0';
	while (ch != '\n' && !eof(fHand))
		read(fHand, &ch, sizeof(ch));
	return(buff);
	}


static void MenuRead(const char *name) {
	int	fHand;
	char	*s;
	short	low, high;

	if ((fHand = open(name, O_RDONLY | O_TEXT)) == -1)
		exit(kErr_Not_Found);
	while ((s = MenuGetLine(fHand)) != 0) {
		if (*s == '.') {
			if (!strcmp(s, ".color")) {
				low = atoi(MenuGetLine(fHand));
				high = atoi(MenuGetLine(fHand));
				video.SetAttr(low, high);
				}
			else if (!strcmp(s, ".saver"))
				saverDelay = (long)atoi(MenuGetLine(fHand)) * 6000L;
			else
				exit(kErr_Syntax);
			}
		else if (numMenuItems < MAX_ITEMS) {
			menuItem[numMenuItems] = strdup(s);
			if (*s == '-')
				menuCommand[numMenuItems++] = "";
			else {
				lenMenuItems = max(strlen(s), lenMenuItems);
				menuCommand[numMenuItems++] = strdup(MenuGetLine(fHand));
				}
			}
		}
	close(fHand);
	if (numMenuItems < 1)
		exit(kErr_No_Items);
	}
