// The Video module provides routines to perform low-level video manipulation
// for fast screen refresh.

#include <conio.h>
#include <dos.h>

#include <common\common.h>

#include <seashell\video.h>


// BIOS interrupts
#define VIDEO_IO	0x0010

// BIOS video routine numbers
#define SET_MODE	0x0000
#define CUR_TYPE	0x0001
#define CUR_POS		0x0002
#define GET_CUR		0x0003
#define GET_STATE	0x000F
#define ALT_SEL		0x0012

// Cursor Masks
#define CUR_OFF		0x0020
#define CUR_ON		0x00DF

// Video locations
#define MONODSP		0xB000		// monochrome display memory
#define COLRDSP		0xB800		// color display memory

// Video modes
#define MDA_M80		7

// VideoCopy equates
#define VSTAT		0x03DA		// video (CRT) status register
#define HRTRCE		1		// horizontal retrace bit mask
#define VRTRCE		8		// vertical retrace bit mask
#define WRDCNT1		240		// no. of words to copy during vert. retrace
#define WRDCNT2		94		// no. of words to copy during horz. retrace

#define BIOS_PAGE	0x0040
#define CRT_MODE	0x0049
#define CRT_COLS	0x004A
#define CRT_START	0x004E
#define CRT_ADDR	0x0063
#define CRT_ROWS	0x0084
#define CRT_EGA		0x0087


VideoRecord	video;

static Byte	vpage=0;		// active video page
static Boolean	vinfo=False;		// tracks whether VideoGetInfo called


// Pass back the vertical and horizontal position of cursor

void VideoGetCursor(short *row, short *col) {
	if (!vinfo)
		VideoGetInfo();
	_AH = GET_CUR;
	_BH = vpage;
	geninterrupt(VIDEO_IO);
	*row = _DH;
	*col = _DL;
	}


// Get the cursor start and end raster scan lines

void VideoGetCursorType(short *start, short *end) {
	if (!vinfo)
		VideoGetInfo();
	_AH = GET_CUR;
	_BH = vpage;
	geninterrupt(VIDEO_IO);
	*start = _CH;
	*end = _CL;
	}


// Saves the current video state in the global video

void VideoGetInfo(void) {
	char far	*CrtMode    = (char far *) MK_FP(BIOS_PAGE, CRT_MODE);
	char far	*CrtCols    = (char far *) MK_FP(BIOS_PAGE, CRT_COLS);
	int  far	*CrtStart   = (int  far *) MK_FP(BIOS_PAGE, CRT_START);
	int  far	*CrtAddr    = (int  far *) MK_FP(BIOS_PAGE, CRT_ADDR);
	char far	*CrtRows    = (char far *) MK_FP(BIOS_PAGE, CRT_ROWS);
	char far	*CrtEgaInfo = (char far *) MK_FP(BIOS_PAGE, CRT_EGA);

	// determine whether video writes must be synchronized
	_AH = ALT_SEL;
	_BL = 0x10;
	geninterrupt(VIDEO_IO);
	if (_BL == 0x10) {
		directvideo = (*CrtAddr == 0x03D4) ? 0 : 1;
		video.rows = 25;
		}
	else {
		directvideo = ((*CrtEgaInfo & 0x08) && (*CrtAddr == 0x03D4)) ? 0 : 1;
		video.rows = *CrtRows + 1;
		}

	// determine whether video is color or monochrome
	if (*CrtAddr == 0x03D4) {
		video.colorFlag = True;
		video.dispSeg = COLRDSP;
		}
	else {
		video.colorFlag = False;
		video.dispSeg = MONODSP;
		}

	// copy remaining parameters from BIOS
	video.mode = *CrtMode;
	video.cols = *CrtCols;
	video.size = video.rows * video.cols;
	video.dispOff = *CrtStart;

	// get active screen
	_AH = GET_STATE;
	geninterrupt(VIDEO_IO);
	vpage = _BH;
	vinfo = True;
	}


// Put the cursor at the specified position

void VideoSetCursor(short row, short col) {
	if (!vinfo)
		VideoGetInfo();
	_AH = CUR_POS;
	_BH = vpage;
	_DH = row;
	_DL = col;
	geninterrupt(VIDEO_IO);
	}


// Set the cursor start and end raster scan lines

void VideoSetCursorType(short start, short end) {
	_AH = CUR_TYPE;
	_CH = start;
	_CL = end;
	geninterrupt(VIDEO_IO);
	}


// Set the video mode

void VideoSetMode(Byte mode) {
	_AH = SET_MODE;
	_AL = mode;
	geninterrupt(VIDEO_IO);
	VideoGetInfo();
	}


// If on is TRUE, the cursor will appear, otherwise the cursor will disappear

void VideoShowCursor(Boolean on) {
	if (!vinfo)
		VideoGetInfo();
	_AH = GET_CUR;
	_BH = vpage;
	geninterrupt(VIDEO_IO);
	_AH = CUR_TYPE;
	if (on)
		_CH &= CUR_ON;
	else
		_CH |= CUR_OFF;
	geninterrupt(VIDEO_IO);
	}
