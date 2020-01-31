// ******************
// * Video Routines *
// ******************

#include <dos.h>

#include <common\common.h>

// BIOS interrupts
#define VIDEO_IO	0x10

// BIOS video routines
#define ALT_SEL		18

#define BIOS_PAGE	0x0040
#define CRT_COLS	0x004A
#define CRT_ROWS	0x0084


void VideoScreenSize(short *rows, short *cols) {
	char far	*CrtCols = (char far *) MK_FP(BIOS_PAGE, CRT_COLS);
	char far	*CrtRows = (char far *) MK_FP(BIOS_PAGE, CRT_ROWS);

	// determine whether video writes must be synchronized
	_AH = ALT_SEL;
	_BL = 0x10;
	geninterrupt(VIDEO_IO);
	*rows = (_BL == 0x10) ? 25 : *CrtRows + 1;
	*cols = *CrtCols;
	}
