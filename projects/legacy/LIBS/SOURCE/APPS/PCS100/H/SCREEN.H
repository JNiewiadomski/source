// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

#define SCREEN_NONE		0
#define SCREEN_ALL_MCH		1
#define SCREEN_SNGL_MCH		2
#define SCREEN_ALL_JOB		3
#define SCREEN_SNGL_JOB		4
#define SCREEN_ALL_SMCH		5
#define SCREEN_SNGL_SMCH	6
#define SCREEN_ALL_SJOB		7
#define SCREEN_SNGL_SJOB	8
#define SCREEN_GRAPH		9

enum QTYPE { QRUN, QSHIFT };

struct ScreenStruct {
	short	num;		// virtual screen number
	short	bot;		// bottom of screen
	short	rig;		// right of screen
	short	lo;		// screen low attribute
	short	hi;		// screen high attribute
	short	inv;		// screen inverse attribute
	short	cur;		// current screen
	};

struct ScreenInfo {
	short	pos;		// current position in item list
	short	nextPos;	// next active position in item list
	short	loc;		// location relative to top
	short	max;		// maximum number of items in list
	short	num;		// maximum number of items that fit on screen
	};

extern ScreenStruct	scrn;

void	ScreenAdjustMenu(void);
void	ScreenDo(int ch);
void	ScreenInit(void);
void	ScreenMonitorNothing(void);
void	ScreenPrint(void);
void	ScreenTitleSet(char *title);
