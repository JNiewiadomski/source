#include "inc\cvideo.h"

// BIOS memory locations
#define BIOS_PAGE	0x0040
#define CRT_COLS	0x004A
#define CRT_ROWS	0x0084

// attribute modifiers
#define BRIGHT		8

// primary video attributes
#define BLU		1
#define GRN		2
#define RED		4

// composite video attributes
#define BLK		0
#define CYN		(BLU | GRN)		// 3
#define MAG		(BLU | RED)		// 5
#define BRN		(GRN | RED)		// 6
#define WHT		(BLU | GRN | RED)	// 7
#define GRAY		(BLK | BRIGHT)
#define LBLU		(BLU | BRIGHT)
#define LGRN		(GRN | BRIGHT)
#define LCYAN		(CYN | BRIGHT)
#define LRED		(RED | BRIGHT)
#define LMAG		(MAG | BRIGHT)
#define YEL		(BRN | BRIGHT)
#define BWHT		(WHT | BRIGHT)
#define REVERSE		112

// double-line boxes
#define BOX_TOP_LEF	201
#define BOX_TOP_MID	205
#define BOX_TOP_RIG	187
#define BOX_MID_LEF	186
#define BOX_MID_RIG	186
#define BOX_BOT_LEF	200
#define BOX_BOT_MID	205
#define BOX_BOT_RIG	188

// ****************
// * CVideo Class *
// ****************

CVideo::CVideo() {
	char far	*CrtCols = (char far *) MK_FP(BIOS_PAGE, CRT_COLS);
	char far	*CrtRows = (char far *) MK_FP(BIOS_PAGE, CRT_ROWS);

	// find out the size of the screen
	_AH = kAlt_Function;
	_BL = 0x10;
	geninterrupt(kVideo_IO);
	fRows = (_BL == 0x10) ? 25 : *CrtRows + 1;
	fCols = *CrtCols;

	// get the video mode and page
	_AH = kGet_State;
	geninterrupt(kVideo_IO);
	fMode = _AL;
	fPage = _BH;

	// set up default video attributes
	fAttr[kAttrLow] = WHT;
	fAttr[kAttrHigh] = BWHT;
	fAttr[kAttrInv] = 0x78;
	}


void CVideo::Box(short top, short lef, short bot, short rig, short attr) {
	short	i, x;

	x = rig - lef - 1;
	PutCharAttr(top, lef, BOX_TOP_LEF, attr, 1);
	PutCharAttr(top, lef+1, BOX_TOP_MID, attr, x);
	PutCharAttr(top, rig, BOX_TOP_RIG, attr, 1);
	for (i = top+1; i < bot; ++i) {
		PutCharAttr(i, lef, BOX_MID_LEF, attr, 1);
		PutCharAttr(i, rig, BOX_MID_RIG, attr, 1);
		}
	PutCharAttr(bot, lef, BOX_BOT_LEF, attr, 1);
	PutCharAttr(bot, lef+1, BOX_BOT_MID, attr, x);
	PutCharAttr(bot, rig, BOX_BOT_RIG, attr, 1);
	}


void CVideo::PutAttr(short row, short col, short attr, short num) {
	short	i;

	i = 0;
	while (i++ < num) {
		Goto(row, col++);
		_AH = kRead_Char_Attr;
		_BH = fPage;
		geninterrupt(kVideo_IO);
		_AH = kWrite_Char_Attr;
		_BH = fPage;
		_BL = fAttr[attr];
		_CX = 1;
		geninterrupt(kVideo_IO);
		}
	}


void CVideo::PutCharAttr(short row, short col, short ch, short attr, short num) {
	Goto(row, col);
	_AH = kWrite_Char_Attr;
	_AL = ch;
	_BH = fPage;
	_BL = fAttr[attr];
	_CX = num;
	geninterrupt(kVideo_IO);
	}


void CVideo::PutStrAttr(short row, short col, const char *s, short attr) {
	while (*s) {
		Goto(row, col++);
		_AL = *s++;			// must not alter regs later
		_AH = kWrite_Char_Attr;
		_BH = fPage;
		_BL = fAttr[attr];
		_CX = 1;
		geninterrupt(kVideo_IO);
		}
	}


void CVideo::SetAttr(short low, short high) {
	fAttr[kAttrLow] = low;
	fAttr[kAttrHigh] = high;
	if (fMode != 7)
		fAttr[kAttrInv] = ((low << 4) & REVERSE) | high;
	}
