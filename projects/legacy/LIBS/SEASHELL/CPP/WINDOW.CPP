// Window Library

#include <conio.h>
#include <mem.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>
#include <common\old\general.h>


#include <common\old\keyboard.h>


#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\error.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\video.h>
#include <seashell\window.h>

// ================================= SCREEN =================================

// video modes
#define CGA_M40		0
#define CGA_C40		1
#define CGA_M80		2
#define CGA_C80		3
#define CGA_CMRES	4
#define CGA_MMRES	5
#define CGA_MHRES	6
#define MDA_M80		7
#define PCJR_CLRES	8
#define PCJR_CMRES	9
#define PCJR_CHRES	10
#define EGA_CMRES	13
#define EGA_CHRES	14
#define EGA_MHRES	15
#define EGA_EHRES	16

#define MAXROW		24
#define MAXCOL		79

struct CELL {
	char	ch;       // character
	char	attr;     // attribute
	};

Rect	GIANT_RECT;

static union {
	CELL	cell;
	short	word;
	} dspbuf[MAXROW+1][MAXCOL+1], oldbuf[MAXROW+1][MAXCOL+1];

static short	oldMode, oldStart, oldEnd, oldRow, oldCol;

static Boolean	changed=False;		// has screen been changed?
static Rect	rChange;		// changed region in dspbuf

static Boolean	invertScreen=False, invertActivated=False;
static Boolean	savePhys=False;


static void	minmaxinit(void);
static void	ScreenCleanUp(void);
static void	ScreenFill(Rect &srectp, char ch, short attr);
static void	ScreenInit(void);
static void	ScreenInvertUpdate(void);
static Boolean	ScreenSlide(Rect &srectp, DIRECTION dir, short dist);
static void	ScreenWrite(short row, short col, short ncols, char *str, short attr);
static void	ScreenWriteCells(short row, short col, CELL *captr, short ncols);

static void	WindowReset(Rect &srectp);


// minmaxinit -- initialize minptr and maxptr

static void minmaxinit(void) {
	rChange.Set(MAXROW, MAXCOL, 0, 0);
	}


// ScreenCleanUp -- terminate display

static void ScreenCleanUp(void) {
	if (savePhys) {
		if (oldMode != video.mode)
			VideoSetMode(oldMode);
		VideoSetCursor(oldRow, oldCol);
		VideoSetCursorType(oldStart, oldEnd);
		puttext(1, 1, MAXCOL+1, MAXROW+1, oldbuf);
		savePhys = False;
		}
	}


// ScreenFill -- fill a rectangle
//   srectp  -  rectangle
//   ch      -  fill character
//   attr    -  fill attribute

static void ScreenFill(Rect &srectp, char ch, short attr) {
	Rect	srect;
	short	word;
	short	r;

	if (!srect.Intersect(srectp, MAXROW, MAXCOL))
		return;
	changed = True;
	word = (attr << 8) | (ch & 0377);
	if (srect.Width() == MAXCOL+1)
		repmem((char *)&dspbuf[srect.Top()][0], (char *)&word, sizeof(CELL), srect.Height()*(MAXCOL+1));
	else if (srect.Width() == 1)
		for (r=srect.Top(); r<=srect.Bot(); r++)
			dspbuf[r][srect.Lef()].word = word;
	else
		for (r=srect.Top(); r<=srect.Bot(); r++)
			repmem((char *)&dspbuf[r][srect.Lef()], (char *)&word, sizeof(CELL), srect.Width());
	rChange.Union(rChange, srect);
	}


// ScreenInit -- initialize display

static void ScreenInit(void) {
	GIANT_RECT.Set(0, 0, GIANT, GIANT);
	VideoGetInfo();
	oldMode = video.mode;
	VideoGetCursor(&oldRow, &oldCol);
	gettext(1, 1, MAXCOL+1, MAXROW+1, oldbuf);
	if (oldMode!=CGA_M80 && oldMode!=CGA_C80 && oldMode!=MDA_M80)
		VideoSetMode(CGA_C80);
	VideoGetCursorType(&oldStart, &oldEnd);
	if (video.mode == MDA_M80) {
		if ((oldStart==6 && oldEnd==7) || (oldEnd > 13)) {   // fix for bug in BIOS
			oldStart = 11;
			oldEnd = 12;
			}
		VideoSetCursorType(11, 12);
		}
	else
		VideoSetCursorType(6, 7);
	VideoShowCursor(False);
	savePhys = True;
	}


void ScreenInvert(void) {
	invertScreen = invertScreen ? False : True;
	rChange.Set(0, 0, MAXROW, MAXCOL);
	changed = True;
	}


void ScreenInvertActivate(Boolean on) {
	invertActivated = on;
	invertScreen = False;
	rChange.Set(0, 0, MAXROW, MAXCOL);
	changed = True;
	if (!on)
		WindowReset(GIANT_RECT);
	}


static void ScreenInvertUpdate(void) {
	int	row, len;
	char	*attr, color;

	if (video.colorFlag)
		for (row=rChange.Top(); row<=rChange.Bot(); row++) {
			attr = (char *) &dspbuf[row][rChange.Lef()];
			attr++;
			for (len=rChange.Rig()-rChange.Lef()+1; len>0; len--, attr+=2) {
				color = *attr & 0x07 ? *attr & 0x07 : (*attr & 0x70) >> 4;
				*attr = invertScreen ? (color << 4) & REVERSE : color;
				}
			}
	else {
		color = invertScreen ? 0x78 : WHT;
		for (row=rChange.Top(); row<=rChange.Bot(); row++) {
			attr = (char *) &dspbuf[row][rChange.Lef()];
			attr++;
			for (len=rChange.Rig()-rChange.Lef()+1; len>0; len--, attr+=2)
				*attr = color;
			}
		}
	}


// ScreenSlide -- slide a rectangle
// RETURN: was slide performed?
//   srectp  -  rectangle
//   dir     -  direction
//   dist    -  distance

static Boolean ScreenSlide(Rect &srectp, DIRECTION dir, short dist) {
	Rect	srect, rActual;
	short	r;

	if (srect.Intersect(srectp, MAXROW, MAXCOL) || dist<=0)
		return (True);
	changed = True;
	switch (dir) {
		case D_UP:
			if (srect.Width() == MAXCOL+1)
				movmem((char *)&dspbuf[srect.Top()][0], (char *)&dspbuf[srect.Top()-dist][0], sizeof(CELL)*srect.Height()*srect.Width());
			else
				for (r=srect.Top(); r<=srect.Bot(); r++)
					movmem((char *)&dspbuf[r][srect.Lef()], (char *)&dspbuf[r-dist][srect.Lef()], sizeof(CELL)*srect.Width());
			rActual.Set(srect.Top()-dist, srect.Lef(), srect.Bot()-dist, srect.Rig());
			break;
		case D_DOWN:
			if (srect.Width() == MAXCOL+1)
				movmem((char *)&dspbuf[srect.Top()][0], (char *)&dspbuf[srect.Top()+dist][0], sizeof(CELL)*srect.Height()*srect.Width());
			else
				for (r=srect.Bot(); r>=srect.Top(); r--)
					movmem((char *)&dspbuf[r][srect.Lef()], (char *)&dspbuf[r+dist][srect.Lef()], sizeof(CELL)*srect.Width());
			rActual.Set(srect.Top()+dist, srect.Lef(), srect.Bot()+dist, srect.Rig());
			break;
		case D_LEFT:
			if (srect.Rig() == MAXCOL && srect.Lef() == dist)
				movmem((char *)&dspbuf[srect.Top()][srect.Lef()], (char *)&dspbuf[srect.Top()][srect.Lef()-dist], sizeof(CELL)*(srect.Height()*(MAXCOL+1)-dist));
			else
				for (r=srect.Top(); r<=srect.Bot(); r++)
					movmem((char *)&dspbuf[r][srect.Lef()], (char *)&dspbuf[r][srect.Lef()-dist], sizeof(CELL)*srect.Width());
			rActual.Set(srect.Top(), srect.Lef()-dist, srect.Bot(), srect.Rig()-dist);
			break;
		case D_RIGHT:
			if (srect.Rig()+dist == MAXCOL && srect.Lef() == 0)
				movmem((char *)&dspbuf[srect.Top()][srect.Lef()], (char *)&dspbuf[srect.Top()][srect.Lef()+dist], sizeof(CELL)*(srect.Height()*(MAXCOL+1)-dist));
			else
				for (r=srect.Top(); r<=srect.Bot(); r++)
					movmem((char *)&dspbuf[r][srect.Lef()], (char *)&dspbuf[r][srect.Lef()+dist], sizeof(CELL)*srect.Width());
			rActual.Set(srect.Top(), srect.Lef()+dist, srect.Bot(), srect.Rig()+dist);
			break;
		}
	rChange.Union(rChange, rActual);
	return (True);
	}


// ScreenSynch -- brings screen up to date

void ScreenSynch(void) {
	if (changed) {
		if (invertActivated)
			ScreenInvertUpdate();
		puttext(1, rChange.Top()+1, MAXCOL+1, rChange.Bot()+1, dspbuf[rChange.Top()]);
		changed = False;
		}
	minmaxinit();
	}


// ScreenWrite -- write string
//   row    -  starting row
//   col    -  starting column
//   ncols  -  number of columns to write
//   str    -  string
//   attr   -  attribute

static void ScreenWrite(short row, short col, short ncols, char *str, short attr) {
	short	i, lef=-1, rig;
	Rect	rActual;

	if (row<0 || row>MAXROW || col>MAXCOL)
		return;
	if (col+ncols > MAXCOL+1)
		ncols = MAXCOL+1-col;
	for (i=0; i<ncols && str[i] != '\0'; i++)
		if (col+i >= 0) {
			rig = col + i;
			if (lef == -1)
				lef = rig;
			dspbuf[row][rig].word = (attr << 8) | (str[i] & 0377);
			}
	if (lef != -1) {
		changed = True;
		rActual.Set(row, lef, row, rig);
		rChange.Union(rChange, rActual);
		}
	}


// ScreenWriteCells -- write vector of CELLs
//   row    -  starting row
//   col    -  starting column
//   captr  -  CELLs
//   ncols  -  number of columns to write

static void ScreenWriteCells(short row, short col, CELL *captr, short ncols) {
	Rect	rActual;

	if (row<0 || row>MAXROW || col<0 || col>MAXCOL)
		return;
	changed = True;
	if (col+ncols > MAXCOL+1)
		ncols = MAXCOL+1-col;
	memmove(&dspbuf[row][col], captr, sizeof(CELL) * ncols);
	rActual.Set(row, col, row, col+ncols-1);
	rChange.Union(rChange, rActual);
	}


// ================================= WINDOW =================================

// drawing characters -- items having two numbers use the first number as
// the horizontal specifier

// single-line boxes
#define VBAR1	179
#define VLINE	179	// alias
#define HBAR1	196
#define HLINE	196	// alias
#define ULC11	218
#define URC11	191
#define LLC11	192
#define LRC11	217
#define TL11	195
#define TR11	180
#define TT11	194
#define TB11	193
#define X11	197

// double-line boxes
#define VBAR2	186
#define HBAR2	205
#define ULC22	201
#define URC22	187
#define LLC22	200
#define LRC22	188
#define TL22	204
#define TR22	185
#define TT22	203
#define TB22	202
#define X22	206

// single-line horizontal & double-line verticle boxes
#define ULC12	214
#define URC12	183
#define LLC12	211
#define LRC12	189
#define TL12	199
#define TR12	182
#define TT12	210
#define TB12	208
#define X12	215

// double-line horizontal & single-line verticle boxes
#define ULC21	213
#define URC21	184
#define LLC21	212
#define LRC21	190
#define TL21	198
#define TR21	181
#define TT21	209
#define TB21	207
#define X21	216

// full and partial blocks
#define BLOCK	219
#define VBAR	219	// alias
#define VBARL	221
#define VBARR	222
#define HBART	223
#define HBARB	220

enum FTYPE { F0000, F1111, F2222, F2111, FMAX };
enum FELTS { UL, UM, UR, ML, MR, LL, LM, LR, EMAX };

static char frame[FMAX][EMAX] = {
	{ ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' ',   ' '   },	// F0000
	{ ULC11, HBAR1, URC11, VBAR1, VBAR1, LLC11, HBAR1, LRC11 },	// F1111
	{ ULC22, HBAR2, URC22, VBAR2, VBAR2, LLC22, HBAR2, LRC22 },	// F2222
	{ ULC21, HBAR2, URC21, VBAR1, VBAR1, LLC11, HBAR1, LRC11 }	// F2111
	};

#define MAXWIND	25

// WROW2, WCOL2 - lower right corner of window, relative to window
#define WROW2(w)	((w)->srect.Height() - 1)
#define WCOL2(w)	((w)->srect.Width() - 1)

typedef void (*drawFunc)(short wnum, Rect &wrect);

struct WINDOW {
	WTYPE	type;		// window frame type
	Boolean	visible;	// True if window should be drawn
	Rect	srect;		// window structure rectangle
	short	attr;		// window frame attribute
	char	*title;		// window title
	short	row;		// current row of cursor in window
	short	col;		// current column of cursor in window
	Boolean	on;		// is cursor visible?
	drawFunc draw;		// redrawing function
	};

static WINDOW	*wind[MAXWIND];		// window info table
static short	wOrder[MAXWIND];	// order of windows, bottom to top
static short	wNumWind;		// number of windows in order array
static short	wMaxRow, wMaxCol;	// maximum physical screen
static char	**wMask=NULL;		// cell ownership mask

// is_top -- determine if window is on top
#define is_top(wnum)	(wNumWind>0 && wOrder[wNumWind - 1] == wnum)

#define SCR_WIND	0		// physical screen "window"


static void	WindowAdd(short wnum);
static void	WindowAllocMask(void);
static void	WindowCleanUp(void);
static void	WindowDimensions(WINDOW *w, short *heightp, short *widthp, short *offsetp);
static void	WindowDispose(short wnum);
static void	WindowDraw(short wnum, Rect &wrectp);
static void	WindowDraw0(short wnum, Rect &wrectp);
static void	WindowDrawAll(Rect &srectp);
static void	WindowDrawFrame(short wnum, Rect &wrectp);
static void	WindowFill(short wnum, Rect &wrectp, char ch, short attr);
static void	WindowFreeMask(void);
static void	WindowGetPhys(short wnum, Rect &srectp, Boolean inclfrm);
static char	*WindowGetTitle(short wnum);
static short	WindowGetTop(void);
static void	WindowHide(short wnum);
static void	WindowHLine(short wnum, short wrow, short wcol1, short wcol2, char ch, short attr);
static void	WindowInit(void);
static Boolean	WindowIsHidden(short wnum);
static short	WindowNew(drawFunc draw);
static void	WindowRemove(short wnum);
static void	WindowSetCursor(short wnum, short wrow, short wcol);
static void	WindowSetFrame(short wnum, WTYPE type, short attr, char *title);
static void	WindowSetMask(short wnum, Rect &srectp);
static void	WindowSetPhysical(short wnum, Rect &srectp);
static void	WindowSetTitle(short wnum, char *title);
static void	WindowShowCursor(short wnum, Boolean on);
static Boolean	WindowSlide(short wnum, Rect &wrectp, DIRECTION dir, short dist);
static void	WindowTop(short wnum);
static void	WindowVLine(short wnum, short wrow1, short wrow2, short wcol, char ch, short attr);
static void	WindowWrite(short wnum, short wrow, short wcol, short ncols, char *str, short attr);
static void	WindowWriteCells(short wnum, short wrow, short wcol, CELL *captr, short ncols);


// HiliteStrLen -- length of a string using the dual attribute encoding
// scheme ('{' for begin hilite and '}' for end hilite).

short HiliteStrLen(char *s) {
	short	i;

	i = 0;
	while (*s) {
		if (*s!='{' && *s!='}')
			i++;
		s++;
		}
	return (i);
	}


// WindowAdd -- add window to top of order array, if necessary

static void WindowAdd(short wnum) {
	short	i;

	for (i=0; i<wNumWind; i++)
		if (wOrder[i] == wnum)
			return;
	wOrder[wNumWind++] = wnum;
	}


// WindowAllocMask -- allocate mask

static void WindowAllocMask(void) {
	short	row;

	wMask = (char **) r_malloc((wMaxRow+1) * sizeof(char *));
	for (row=0; row<=wMaxRow; row++) {
		wMask[row] = (char *) r_malloc(wMaxCol+1);
		}
	}


// WindowCleanUp -- terminate window module

static void WindowCleanUp(void) {
	WindowFreeMask();
	ScreenCleanUp();
	}


// WindowDimensions -- get dimensions of window
//   w        -  window
//   heightp  -  returned height
//   widthp   -  returned width
//   offsetp  -  returned row/col offset

static void WindowDimensions(WINDOW *w, short *heightp, short *widthp, short *offsetp) {
	*widthp = w->srect.Width();
	*heightp = w->srect.Height();
	if (w->type == WNONE)
		*offsetp = 0;
	else {
		*widthp -= 2;
		*heightp -= 2;
		*offsetp = 1;
		}
	}


// WindowDispose -- eliminate window

static void WindowDispose(short wnum) {
	WINDOW	*w=wind[wnum], *w2;
	short	wnum2;

	if (wnum <= 0)
		return;
	WindowRemove(wnum);
	WindowReset(w->srect);
	wnum2 = wOrder[wNumWind - 1];
	w2 = wind[wnum2];
	WindowDrawFrame(wnum2, GIANT_RECT);	// might be new top window
	WindowSetCursor(wnum2, w2->row, w2->col);
	WindowShowCursor(wnum2, w2->on);
	if (w->title != NULL)
		r_free(w->title);
	r_free((char *)w);
	wind[wnum] = NULL;
	}


// WindowDraw -- draw one window's frame and interior
//   wnum    -  window
//   wrectp  -  affected window-relative rect

static void WindowDraw(short wnum, Rect &wrectp) {
	WINDOW	*w=wind[wnum];
	Rect	wrect;

	WindowDrawFrame(wnum, wrectp);
	wrect = wrectp;
	if (w->type != WNONE) {
		wrect.Offset(-1, -1);
		if (!wrect.Intersect(wrect, WROW2(w)-2, WCOL2(w)-2))
			return;
		}
	if (w->draw != NULL)
		(*w->draw)(wnum, wrect);
	}


// WindowDraw0 -- redrawing function for window zero (SCR_WIND)

static void WindowDraw0(short wnum, Rect &wrectp) {
	WindowFill(wnum, wrectp, ' ', WHT);
	}


// WindowDrawAll -- draw all windows in physical screen rectangle
//   srectp  -  affected physical screen rect

static void WindowDrawAll(Rect &srectp) {
	WINDOW	*w;
	short	r, c, i;
	Boolean	visible[MAXWIND];
	Rect	wrect;

	for (i=0; i<MAXWIND; i++)
		visible[i] = False;
	for (r=srectp.Top(); r<=srectp.Bot(); r++)
		for (c=srectp.Lef(); c<=srectp.Rig(); c++)
			visible[wMask[r][c]] = True;
	for (i=0; i<wNumWind; i++) {
		if (!visible[wOrder[i]])
			continue;
		w = wind[wOrder[i]];
		wrect.Set(srectp.Top() - w->srect.Top(), srectp.Lef() - w->srect.Lef(), srectp.Bot() - w->srect.Top(), srectp.Rig() - w->srect.Lef());
		if (!wrect.Intersect(wrect, WROW2(w), WCOL2(w)))
			continue;
		WindowDraw(wOrder[i], wrect);
		}
	}


// WindowDrawFrame -- draw complete or partial frame around window
//   wnum    -  window
//   wrectp  -  rectangle to draw within

static void WindowDrawFrame(short wnum, Rect &wrectp) {
	WINDOW	*w=wind[wnum];
	short	wrow2, wcol2, style, c;
	Rect	wrect1, wrect2;			// inside
	WTYPE	savetype;
	char	*t;

	wrow2 = WROW2(w);
	wcol2 = WCOL2(w);
	if (wrow2 == 0 || wcol2 == 0)
		return;
	if (!wrect1.Intersect(wrectp, wrow2, wcol2))
		return;
	wrect2.Set(max(wrect1.Top(), 1), max(wrect1.Lef(), 1), min(wrect1.Bot(), wrow2-1), min(wrect1.Rig(), wcol2-1));
	switch (w->type) {
		case WNONE:
			return;
		case W0000:
			style = F0000;
			break;
		case W1111:
			style = F1111;
			break;
		case W2222:
			style = F2222;
			break;
		case W2111:
			style = F2111;
			break;
		}
	savetype = w->type;
	w->type = WNONE;		// allow access to frame itself
	if (wrect1.Top() == 0) {
		if (wrect1.Lef() == 0)
			WindowWrite(wnum, 0, 0, 1, &frame[style][UL], w->attr);
		c = 1;
		if (c <= wrect1.Rig())
			WindowHLine(wnum, 0, c, wrect1.Rig(), frame[style][UM], w->attr);
		if (w->title == NULL)
			t = "";
		else
			t = w->title;
		c = w->srect.Width()/2 - strlen(t)/2;
		for ( ; *t && c<=wrect2.Rig(); c++, t++)
			if (c >= wrect2.Lef())
				WindowWrite(wnum, 0, c, 1, t, w->attr);
		if (wrect1.Rig() == wcol2)
			WindowWrite(wnum, 0, wrect1.Rig(), 1, &frame[style][UR], w->attr);
		}
	if (wrect1.Lef() == 0)
		WindowVLine(wnum, wrect2.Top(), wrect2.Bot(), 0, frame[style][ML], w->attr);
	if (wrect1.Rig() == wcol2)
		WindowVLine(wnum, wrect2.Top(), wrect2.Bot(), wrect1.Rig(), frame[style][MR], w->attr);
	if (wrect1.Bot() == wrow2) {
		if (wrect1.Lef() == 0)
			WindowWrite(wnum, wrect1.Bot(), 0, 1, &frame[style][LL], w->attr);
		WindowHLine(wnum, wrect1.Bot(), wrect2.Lef(), wrect2.Rig(), frame[style][LM], w->attr);
		if (wrect1.Rig() == wcol2)
			WindowWrite(wnum, wrect1.Bot(), wrect1.Rig(), 1, &frame[style][LR], w->attr);
		}
	w->type = savetype;
	}


// WindowFill -- fill rectangle
//   wnum    =>  window
//   wrectp  =>  rectangle
//   ch      =>  fill character
//   attr    =>  fill attribute

static void WindowFill(short wnum, Rect &wrectp, char ch, short attr) {
	WINDOW	*w=wind[wnum];
	Rect	wrect, srect;
	char	s[100];
	short	i, ncols, height, width, offset;

	WindowDimensions(w, &height, &width, &offset);
	if (!wrect.Intersect(wrectp, height-1, width-1))
		return;
	if (is_top(wnum)) {
		srect.Set(w->srect.Top()+wrect.Top()+offset, w->srect.Lef()+wrect.Lef()+offset, w->srect.Top()+wrect.Bot()+offset, w->srect.Lef()+wrect.Rig()+offset);
		ScreenFill(srect, ch, attr);
		}
	else {
		ncols = wrect.Width();
		if (ncols >= sizeof(s))
			ncols = sizeof(s) - 1;
		repstr(s, ch, ncols);
		for (i=wrect.Top(); i<=wrect.Bot(); i++)
			WindowWrite(wnum, i, wrect.Lef(), ncols, s, attr);
		}
	}


// WindowFreeMask -- free mask

static void WindowFreeMask(void) {
	short	row;

	if (wMask) {
		for (row=0; row<=wMaxRow; row++)
			if (wMask[row])
				r_free(wMask[row]);
		r_free((char *) wMask);
		wMask = NULL;
		}
	}


// WindowGetPhys -- get window's physical size
//   wnum     -  window
//   srectp   -  size (returned)
//   inclfrm  -  should size include frame?

static void WindowGetPhys(short wnum, Rect &srectp, Boolean inclfrm) {
	WINDOW	*w=wind[wnum];

	srectp = w->srect;
	if (w->type != WNONE && !inclfrm)
		srectp.Inset(1, 1);
	}


static char *WindowGetTitle(short wnum) {
	return (wind[wnum]->title);
	}


// WindowGetTop -- get top window
// RETURN: Window number.

static short WindowGetTop(void) {
	return (wNumWind>0 ? wOrder[wNumWind-1] : -1);
	}


// WindowHide -- hide window

static void WindowHide(short wnum) {
	WINDOW	*w=wind[wnum];

	WindowRemove(wnum);
	WindowReset(w->srect);
	w = wind[wnum = WindowGetTop()];
	WindowDrawFrame(wnum, GIANT_RECT);	// might be new top window
	WindowSetCursor(wnum, w->row, w->col);
	WindowShowCursor(wnum, w->on);
	}


// WindowHLine -- draw howizontal line
//   wnum   -  window
//   wrow   -  row
//   wcol1  -  starting column
//   wcol2  -  ending column
//   ch     -  character
//   attr   -  attribute

static void WindowHLine(short wnum, short wrow, short wcol1, short wcol2, char ch, short attr) {
	Rect	wrect;

	wrect.Set(wrow, wcol1, wrow, wcol2);
	WindowFill(wnum, wrect, ch, attr);
	}


// WindowInit -- initialize window module

static void WindowInit(void) {
	static WINDOW	scr_wind;
	short		i;

	ScreenInit();
	wMaxRow = SCREEN_HEIGHT - 1;
	wMaxCol = SCREEN_WIDTH - 1;
	WindowAllocMask();
	for (i=0; i<MAXWIND; i++)
		wind[i] = NULL;
	scr_wind.type = WNONE;
	scr_wind.srect.Set(0, 0, wMaxRow, wMaxCol);
	scr_wind.attr = WHT;
	scr_wind.title = NULL;
	scr_wind.row = scr_wind.col = 0;
	scr_wind.on = False;
	scr_wind.draw = WindowDraw0;
	wind[SCR_WIND] = &scr_wind;
	WindowTop(SCR_WIND);
	}


// WindowIsHidden - determine if window is hidden
// RETURN: Is it?

static Boolean WindowIsHidden(short wnum) {
	short	i;

	for (i=0; i<wNumWind; i++)
		if (wOrder[i] == wnum)
			return (False);
	return (True);
	}


// WindowNew -- create new window
// RETURN: Window number.

static short WindowNew(drawFunc draw) {
	WINDOW	*w;
	short	wnum;

	if (wNumWind == MAXWIND)
		Error(Err_Fatal, ErrorGet(_WindowERROR, _WindowERROR_OutOfWin));
	for (wnum=0; wnum<MAXWIND; wnum++)
		if (wind[wnum] == NULL)
			break;
	if (wnum == MAXWIND)
		Error(Err_Fatal, ErrorGet(_WindowERROR, _WindowERROR_OutOfWin));
	wind[wnum] = w = (WINDOW *) r_malloc(sizeof(WINDOW));
	w->srect.Set(0, 0, 0, 0);
	w->row = w->col = 0;
	w->on = False;
	w->type = W1111;
	w->attr = WHT;
	w->title = NULL;
	w->draw = draw;
	return (wnum);
	}


// WindowRemove -- remove window from order array

static void WindowRemove(short wnum) {
	short	i;

	for (i=0; i<wNumWind; i++)
		if (wOrder[i] == wnum) {
			for (i++; i<wNumWind; i++)
				wOrder[i-1] = wOrder[i];
			wNumWind--;
			return;
			}
	}


// WindowReset -- recalculate mask rectangle and redraw screen
//   srectp  -  affected rectangle

static void WindowReset(Rect &srectp) {
	Rect	srect, srect2;
	WINDOW	*w;
	short	i;

	if (srectp.Top() > srectp.Bot() || srectp.Lef() > srectp.Rig())
		return;
	if (!srect.Intersect(srectp, wMaxRow, wMaxCol))
		return;
	for (i=0; i<wNumWind; i++) {
		w = wind[wOrder[i]];
		srect2.Set(max(srect.Top(), w->srect.Top()), max(srect.Lef(), w->srect.Lef()), min(srect.Bot(), w->srect.Bot()), min(srect.Rig(), w->srect.Rig()));
		WindowSetMask(wOrder[i], srect2);
		}
	WindowDrawAll(srect);
	}


// WindowSetCursor -- set cursor position

static void WindowSetCursor(short wnum, short wrow, short wcol) {
	WINDOW	*w=wind[wnum];
	short	height, width, offset;

	WindowDimensions(w, &height, &width, &offset);
	if (wrow<0 || wrow>=height || wcol<0 || wcol>=width)
		return;
	w->row = wrow;
	w->col = wcol;
	if (is_top(wnum))
		VideoSetCursor(w->srect.Top()+wrow+offset, w->srect.Lef()+wcol+offset);
	}


// WindowSetFrame -- set window's frame appearance
//   wnum   =>  window
//   type   =>  frame type (WNONE, W0000, W1111, W2222, W2111)
//   attr   =>  frame attribute
//   title  =>  frame title

static void WindowSetFrame(short wnum, WTYPE type, short attr, char *title) {
	WINDOW	*w=wind[wnum];

	w->type = type;
	w->attr = attr;
	if (title != NULL) {
		if (w->title != NULL)
			r_free(w->title);
		if ((w->title = (char *) r_malloc(strlen(title) + 1)) != NULL)
			strcpy(w->title, title);
		}
	WindowDrawFrame(wnum, GIANT_RECT);
	}


// WindowSetMask -- fill mask rectangle with window number
//   wnum    -  window
//   srectp  -  rectangle to fill

static void WindowSetMask(short wnum, Rect &srectp) {
	Rect	srect;
	short	r;

	if (!srect.Intersect(srectp, wMaxRow, wMaxCol))
		return;
	for (r=srect.Top(); r<=srect.Bot(); r++)
		if (srect.Rig() >= srect.Lef())
			setmem(&wMask[r][srect.Lef()], srect.Width(), wnum);
	}


// WindowSetPhysical -- set window's physical size
//   wnum    -  window
//   srectp  -  size (including frame)

static void WindowSetPhysical(short wnum, Rect &srectp) {
	WINDOW	*w=wind[wnum];
	Rect	srect, srectfixup;

	if (srectp.Top() < -1 || srectp.Top() > wMaxRow || srectp.Lef() < -1 || srectp.Lef() > wMaxCol || srectp.Height() < 1 || srectp.Width() < 1)
		return;
	srect.Intersect(srectp, wMaxRow, wMaxCol);
	if (WindowIsHidden(wnum))
		srectfixup = srect;
	else
		srectfixup.Union(srect, w->srect);
	w->srect = srect;
	WindowAdd(wnum);
	WindowReset(srectfixup);
	WindowShowCursor(wnum, w->on);
	}


static void WindowSetTitle(short wnum, char *title) {
	WINDOW	*w=wind[wnum];

	if (title) {
		if (w->title)
			r_free(w->title);
		if ((w->title = (char *) r_malloc(strlen(title)+1)) != NULL)
			strcpy(w->title, title);
		}
	else if (w->title)
		r_free(w->title);
	WindowDrawFrame(wnum, GIANT_RECT);
	}


// WindowShowCursor -- turn cursor on or off

static void WindowShowCursor(short wnum, Boolean on) {
	WINDOW	*w=wind[wnum];

	w->on = on;
	if (is_top(wnum))
		VideoShowCursor(on);
	}


// WindowSlide -- slide rectangle
// RETURN: was slide performed?
//   wnum    -  window
//   wrectp  -  rectangle
//   dir     -  direction
//   dist    -  distance

static Boolean WindowSlide(short wnum, Rect &wrectp, DIRECTION dir, short dist) {
	WINDOW	*w=wind[wnum];
	Rect	wrect, srect;
	short	height, width, offset;

	if (!is_top(wnum))	// not worth dealing with
		return(False);
	WindowDimensions(w, &height, &width, &offset);
	if (!wrect.Intersect(wrectp, height-1, width-1))
		return(True);
	switch (dir) {
		case D_UP:
			wrect.SetTop(max(wrect.Top(), dist));
			break;
		case D_DOWN:
			wrect.SetBot(min(wrect.Bot(), height-dist-1));
			break;
		case D_LEFT:
			wrect.SetLef(max(wrect.Lef(), dist));
			break;
		case D_RIGHT:
			wrect.SetRig(min(wrect.Rig(), width-dist-1));
			break;
		}
	if (wrect.Top() > wrect.Bot() || wrect.Lef() > wrect.Rig())
		return (True);
	srect.Set(w->srect.Top()+wrect.Top()+offset, w->srect.Lef()+wrect.Lef()+offset, w->srect.Top()+wrect.Bot()+offset, w->srect.Lef()+wrect.Rig()+offset);
	return (ScreenSlide(srect, dir, dist));
	}


// WindowTop -- force window to top

static void WindowTop(short wnum) {
	WINDOW	*w=wind[wnum];

	if (is_top(wnum))
		return;
	WindowRemove(wnum);
	WindowAdd(wnum);
	WindowReset(w->srect);
	WindowSetCursor(wnum, w->row, w->col);
	WindowShowCursor(wnum, w->on);
	}


// WindowVLine -- draw vertical line
//   wnum   -  window
//   wrow1  -  starting row
//   wrow2  -  ending row
//   wcol   -  column
//   ch     -  character
//   attr   -  attribute

static void WindowVLine(short wnum, short wrow1, short wrow2, short wcol, char ch, short attr) {
	Rect	wrect;

	wrect.Set(wrow1, wcol, wrow2, wcol);
	WindowFill(wnum, wrect, ch, attr);
	}


// WindowWrite -- write string
//   wnum   -  window
//   wrow   -  starting row
//   wcol   -  starting column
//   ncols  -  number of columns to write
//   str    -  string
//   attr   -  attribute

static void WindowWrite(short wnum, short wrow, short wcol, short ncols, char *str, short attr) {
	WINDOW	*w=wind[wnum];
	short	c, sr, sc1, sc2, height, width, offset, state, colstop;
	char	*s;

	if (wrow<0 || wcol<0)
		return;
	WindowDimensions(w, &height, &width, &offset);
	if (wrow>=height || wcol>=width)
		return;
	sr = w->srect.Top() + wrow + offset;
	if (is_top(wnum)) {
		ncols = min(ncols, width-wcol);
		ScreenWrite(sr, w->srect.Lef()+wcol+offset, ncols, str, attr);
		}
	else {
		state = 1;
		colstop = min(width, wcol+ncols);
		for (c=wcol; c<colstop && *str!='\0'; c++, str++) {
			sc2 = w->srect.Lef()+c+offset;
			switch (state) {
				case 1:
					if (wMask[sr][sc2] == (char)wnum) {
						sc1 = sc2;
						s = str;
						state = 2;
						}
					continue;
				case 2:
					if (wMask[sr][sc2] != (char)wnum) {
						ScreenWrite(sr, sc1, sc2-sc1, s, attr);
						state = 1;
						}
					break;
				}
			}
		if (state == 2) {
			sc2 = w->srect.Lef() + c + offset;
			ScreenWrite(sr, sc1, sc2-sc1, s, attr);
			}
		}
	}


// WindowWriteCells -- write vector of CELLs
//   wnum   -  window
//   wrow   -  starting row
//   wcol   -  starting column
//   captr  -  CELLs
//   ncols  -  number of columns to write

static void WindowWriteCells(short wnum, short wrow, short wcol, CELL *captr, short ncols) {
	WINDOW	*w=wind[wnum];
	short	i, sr, sc1, sc2, height, width, offset, state;
	CELL	*p;

	if (wrow<0 || wcol<0 || ncols<=0)
		return;
	WindowDimensions(w, &height, &width, &offset);
	if (wrow>=height || wcol>=width)
		return;
	ncols = min(ncols, width-wcol);
	sr = w->srect.Top() + wrow + offset;
	if (is_top(wnum))
		ScreenWriteCells(sr, w->srect.Lef()+wcol+offset, captr, ncols);
	else {
		state = 1;
		for (i=0; i<ncols; i++, captr++) {
			sc2 = w->srect.Lef()+wcol+i+offset;
			switch (state) {
				case 1:
					if (wMask[sr][sc2] == (char)wnum) {
						sc1 = sc2;
						p = captr;
						state = 2;
						}
					continue;
				case 2:
					if (wMask[sr][sc2] != (char)wnum) {
						ScreenWriteCells(sr, sc1, p, sc2-sc1);
						state = 1;
						}
					break;
				}
			}
		if (state == 2) {
			sc2 = w->srect.Lef() + wcol + i + offset;
			ScreenWriteCells(sr, sc1, p, sc2-sc1);
			}
		}
	}


// ========================= THE VIRTUAL SCREEN =============================

#define MAXVSCREEN	MAXWIND		// maximum number of virtual screens
#define BADVSCREEN	-1		// error return

#define VGetWinNum(vs)	(vscreen[vsnum]->winnum)

struct VSCREEN {
  short	winnum;		// associated window
  short	vwinrow1;	// location of window: first row
  short	vwincol1;	// location of window: first column
  short	vrow2;		// size of vscreen: last row
  short	vcol2;		// size of vscreen: last column
  short	vcurrow;	// cursor row
  short	vcurcol;	// cursor column
  CELL	**ca;		// cell array
  };

static VSCREEN	*vscreen[MAXVSCREEN];	// vscreen info table
static int	wtable[MAXWIND];	// window --> vscreen num mapping
static Boolean	virtInit=False;		// have we been initialized?


static void	VirtualCleanUp(void);
static short	VirtualGetVSNum(short wnum);
static void	VirtualRedraw(short wnum, Rect &wrectp);
static void	VirtualRepCell(CELL *cellp, char ch, short attr, short ncols);


// VirtualAvail -- returns True if we have been initialized

Boolean VirtualAvail(void) {
	return (virtInit);
	}


// VirtualCleanUp -- terminate virtual screen module

static void VirtualCleanUp(void) {
	short	i;

	if (virtInit) {
		for (i=0; i<MAXVSCREEN; i++)
			if (vscreen[i] != NULL)
				VirtualDispose(i);
		WindowCleanUp();
		virtInit = False;
		}
	}


// VirtualDispose -- eliminate virtual screen and window

void VirtualDispose(short vsnum) {
	VSCREEN	*vs;
	short	vr;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	if (vs != NULL) {
		WindowDispose(vs->winnum);
		if (vs->ca != NULL) {
			for (vr=0; vr<=vs->vrow2; vr++)
				if (vs->ca[vr] != NULL)
			r_free((char *)vs->ca[vr]);
			r_free((char *)vs->ca);
			}
		r_free((char *)vs);
		vscreen[vsnum] = NULL;
		}
	}


// VirtualFill -- fill rectangle

void VirtualFill(short vsnum, Rect &vrectp, char ch, short attr) {
	VSCREEN	*vs;
	Rect	vrect;
	short	vr, wcol1, wcol2;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	if (!vrect.Intersect(vrectp, vs->vrow2, vs->vcol2))
		return;
	for (vr=vrect.Top(); vr<=vrect.Bot(); vr++) {
		VirtualRepCell(&vs->ca[vr][vrect.Lef()], ch, attr, vrect.Width());
		wcol1 = max(0, vrect.Lef() - vs->vwincol1);
		wcol2 = vrect.Rig() - vs->vwincol1;
		WindowWriteCells(vs->winnum, vr-vs->vwinrow1, wcol1, &vs->ca[vr][vrect.Lef()], wcol2-wcol1+1);
		}
	}


// VirtualGetCursor -- get cursor position relative to virtual screen

void VirtualGetCursor(short vsnum, short *vrowp, short *vcolp) {
	VSCREEN	*vs;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	*vrowp = vs->vcurrow;
	*vcolp = vs->vcurcol;
	}


// VirtualGetNew() reads in the window resource file indexed by id.  If it
// has an associated video screen, it is read in and drawn as well.  The
// rectangular window dimensions are returned in r.

short VirtualGetNew(Word id, Rect *r) {
	ResWindow	*r_win;
	short		num;
	WTYPE		type;
	Byte		attr;

	if ((r_win = (ResWindow *) ResLoad(R_Window, id)) == NULL)
		return (SCR_WIND);
	*r = r_win->loc;
	attr = ColorGet(r_win->color_id, r_win->color_idx);
	type = (WTYPE) r_win->frame;
	if (type != WNONE)
		r->Inset(1, 1);
	r->Offset(-r->Top(), -r->Lef());
	num = VirtualNew(r->Height(), r->Width(), type, r_win->loc, r_win->title, attr);
	VirtualFill(num, GIANT_RECT, ' ', attr);
	if (r_win->video_id != 0)
		VirtualGetVideo(num, r_win->video_id, r_win->video_loc);
	return (num);
	}


void VirtualGetPhysical(short vsnum, Rect &srectp) {
	WindowGetPhys(VGetWinNum(vsnum), srectp, True);
	}


// VirtualGetSize -- get size of virtual screen

void VirtualGetSize(short vsnum, short *nrowsp, short *ncolsp) {
	if (vsnum == BADVSCREEN)
		return;
	*nrowsp = vscreen[vsnum]->vrow2+1;
	*ncolsp = vscreen[vsnum]->vcol2+1;
	}


char *VirtualGetTitle(short vsnum) {
	return (WindowGetTitle(VGetWinNum(vsnum)));
	}


Boolean VirtualGetVideo(short num, Word id, Point &loc) {
	ResVideo	*r_vid;
	short		count, i;
	char		*buff;
	Byte		attr;

	if ((r_vid = (ResVideo *) ResLoad(R_Video, id)) == NULL)
		return (False);
	attr = ColorGet(r_vid->color_id, r_vid->color_idx);
	buff = r_vid->video;
	for (count=r_vid->num, i=0; i<count; i++) {
		VirtualWrite(num, loc.V()+i, loc.H(), GIANT, buff, attr);
		buff += r_vid->len;
		}
	return (True);
	}


// VirtualGetVSNum -- get window's virtual screen number
// RETURN: virtual screen number

static short VirtualGetVSNum(short wnum) {
	return(wtable[wnum]);
	}


// VirtualGetWinLoc -- get window's location relative to virtual screen

void VirtualGetWinLoc(short vsnum, short *vrow1p, short *vcol1p) {
	*vrow1p = vscreen[vsnum]->vwinrow1;
	*vcol1p = vscreen[vsnum]->vwincol1;
	}


short VirtualGlobalRow(short vsnum, short pos) {
	WINDOW	*w;

	w = wind[vscreen[vsnum]->winnum];
	if (w->type == WNONE)
		return (w->srect.Top() + pos);
	else
		return (w->srect.Top() + pos + 1);
	}


short VirtualGlobalCol(short vsnum, short pos) {
	WINDOW	*w;

	w = wind[vscreen[vsnum]->winnum];
	if (w->type == WNONE)
		return (w->srect.Lef() + pos);
	else
		return (w->srect.Lef() + pos + 1);
	}


void VirtualHide(short vsnum) {
	WindowHide(VGetWinNum(vsnum));
	}


// VirtualInit -- initialize virtual screen module.  It is assumed that
// the C compiler allocates contiguous blocks of memory for arrays.

void VirtualInit(void) {
	short	i;

	atexit(VirtualCleanUp);			// register cleanup function
	WindowInit();
	for (i=0; i<MAXVSCREEN; i++)
		vscreen[i] = NULL;
	for (i=0; i<MAXWIND; i++)
		wtable[i] = BADVSCREEN;
	virtInit = True;
	}


// VirtualNew -- create virtual screen
// RETURN: virtual screen number or BADVSCREEN on error
//   nrows   =>  number of rows
//   ncols   =>  number of columns
//   type    =>  type of window frame
//   srectp  =>  window's physical size & location
//   title   =>  window's frame title
//   attr    =>  window's frame attribute

short VirtualNew(short nrows, short ncols, WTYPE type, Rect &srectp, char *title, short attr) {
	VSCREEN	*vs;
	short	vsnum, wnum, vr;
	Rect	srect;

	if (nrows==0 || ncols==0)
		Error(Err_Fatal, ErrorGet(_WindowERROR, _WindowERROR_OutOfVirt));
	for (vsnum=0; vsnum<MAXVSCREEN; vsnum++)
		if (vscreen[vsnum] == NULL)
			break;
	if (vsnum == MAXVSCREEN)
		Error(Err_Fatal, ErrorGet(_WindowERROR, _WindowERROR_OutOfVirt));
	vscreen[vsnum] = vs = (VSCREEN *) r_malloc(sizeof(VSCREEN));
	vs->vrow2 = nrows-1;
	vs->vcol2 = ncols-1;
	vs->vcurrow = vs->vcurcol = 0;
	vs->ca = (CELL **) r_malloc(nrows * sizeof(char *));
	for (vr=0; vr<nrows; vr++) {
		vs->ca[vr] = (CELL *) r_malloc(ncols * sizeof(CELL));
		VirtualRepCell(vs->ca[vr], ' ', attr, ncols);
		}
	vs->vwinrow1 = vs->vwincol1 = 0;
	vs->winnum = wnum = WindowNew(VirtualRedraw);
	wtable[wnum] = vsnum;
	srect = srectp;
	WindowSetFrame(wnum, type, attr, title);
	WindowSetPhysical(wnum, srect);
	WindowSetCursor(wnum, 0, 0);
	WindowShowCursor(wnum, False);
	return (vsnum);
	}


// VirtualPan -- move window relative to virtual screen

void VirtualPan(short wnum, DIRECTION dir, short dist) {
	VSCREEN	*vs;
	Rect	srect, wrect;

	vs = vscreen[VirtualGetVSNum(wnum)];
	WindowGetPhys(wnum, srect, False);
	wrect.Set(0, 0, srect.Height()-1, srect.Width()-1);
	switch (dir) {
		case D_UP:
			if ((dist = min(dist, vs->vwinrow1)) <= 0)
				return;
			vs->vwinrow1 -= dist;
			vs->vcurrow = min(vs->vcurrow, vs->vwinrow1+wrect.Bot());
			if (WindowSlide(wnum, wrect, D_DOWN, dist))
				wrect.SetBot(dist-1);
			break;
		case D_DOWN:
			if ((dist = min(dist, vs->vrow2-wrect.Bot()-vs->vwinrow1)) <= 0)
				return;
			vs->vwinrow1 += dist;
			vs->vcurrow = max(vs->vcurrow, vs->vwinrow1);
			if (WindowSlide(wnum, wrect, D_UP, dist))
				wrect.SetTop(wrect.Bot()+1-dist);
			break;
		case D_RIGHT:
			if ((dist = min(dist, vs->vcol2-wrect.Rig()-vs->vwincol1)) <= 0)
				return;
			vs->vwincol1 += dist;
			vs->vcurcol = max(vs->vcurcol, vs->vwincol1);
			if (WindowSlide(wnum, wrect, D_LEFT, dist))
				wrect.SetLef(wrect.Rig()+1-dist);
			break;
		case D_LEFT:
			if ((dist = min(dist, vs->vwincol1)) <= 0)
				return;
			vs->vwincol1 -= dist;
			vs->vcurcol = min(vs->vcurcol, vs->vwincol1+srect.Width()-1);
			if (WindowSlide(wnum, wrect, D_RIGHT, dist))
				wrect.SetRig(dist-1);
			break;
		}
	VirtualRedraw(wnum, wrect);
	VirtualSetCursor(wtable[wnum], GIANT, GIANT);
	}


// VirtualRedraw -- redraw damaged window
//   wnum    -  window
//   wrectp  -  rectangle to be redrawn

static void VirtualRedraw(short wnum, Rect &wrectp) {
	VSCREEN	*vs;
	Rect	srect, wrect, wrect2;
	short	vrow1, vcol1, vrow2, vcol2, vr, vc2;

	WindowGetPhys(wnum, srect, False);
	if (!wrect.Intersect(wrectp, srect.Height()-1, srect.Width()-1))
		return;
	vs = vscreen[VirtualGetVSNum(wnum)];
	vrow1 = vs->vwinrow1 + wrect.Top();
	vcol1 = vs->vwincol1 + wrect.Lef();
	vrow2 = vs->vwinrow1 + wrect.Bot();
	vcol2 = vs->vwincol1 + wrect.Rig();
	for (vr=vrow1; vr<=vrow2; vr++) {
		if (vr > vs->vrow2) {
			wrect.SetTop(vr - vs->vwinrow1);
			WindowFill(wnum, wrect, ' ', WHT);
			break;
			}
		vc2 = min(vcol2, vs->vcol2);
		WindowWriteCells(wnum, vr-vs->vwinrow1, wrect.Lef(), &vs->ca[vr][vcol1], vc2-vcol1+1);
		vc2 = max(vcol1, vc2+1);
		if (vc2 <= vcol2) {
			wrect2.Set(vr-vs->vwinrow1, vc2-vs->vwincol1, vr-vs->vwinrow1, wrect.Rig());
			WindowFill(wnum, wrect2, ' ', WHT);
			}
		}
	}


// VirtualRepCell -- replicate cell
//   cellp  -  vector of CELLs
//   ch     -  character to be replicated
//   attr   -  attribute to be replicated
//   ncols  -  columns to replicate

static void VirtualRepCell(CELL *cellp, char ch, short attr, short ncols) {
	CELL	cell;

	cell.ch = ch;
	cell.attr = (char)attr;
	repmem((char *)cellp, (char *)&cell, sizeof(CELL), ncols);
	}


void VirtualSelect(short vsnum) {
	WindowTop(VGetWinNum(vsnum));
	}


// VirtualSetCursor -- set cursor position relative to virtual screen

void VirtualSetCursor(short vsnum, short vrow, short vcol) {
	VSCREEN	*vs;
	Rect	srect, wrect;
	short	wrow, wcol;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	if (vrow == GIANT)
		vrow = vs->vcurrow;
	if (vcol == GIANT)
		vcol = vs->vcurcol;
	if (vrow<0 || vrow>vs->vrow2 || vcol<0 || vcol>vs->vcol2)
		return;
	vs->vcurrow = vrow;
	vs->vcurcol = vcol;
	WindowGetPhys(vs->winnum, srect, False);
	if (srect.Height()<=0 || srect.Width()<=0)
		return;
	if (vrow>=vs->vwinrow1 && vrow<vs->vwinrow1+srect.Height()) {
		if (vcol < vs->vwincol1)
			VirtualPan(vs->winnum, D_LEFT, vs->vwincol1-vcol);
		else if (vcol >= vs->vwincol1 + srect.Width())
			VirtualPan(vs->winnum, D_RIGHT, vcol-(vs->vwincol1+srect.Width())+1);
		}
	else if (vcol>=vs->vwincol1 && vcol<vs->vwincol1+srect.Width()) {
		if (vrow < vs->vwinrow1)
			VirtualPan(vs->winnum, D_UP, vs->vwinrow1-vrow);
		else if (vrow >= vs->vwinrow1 + srect.Height())
			VirtualPan(vs->winnum, D_DOWN, vrow-(vs->vwinrow1+srect.Height())+1);
		}
	else {
		vs->vwinrow1 = max(0, vrow-srect.Height()/2);
		vs->vwincol1 = max(0, vcol-srect.Width()/2);
		wrect.Set(0, 0, srect.Height(), srect.Width());
		VirtualRedraw(vs->winnum, wrect);
		}
	wrow = vrow - vs->vwinrow1;
	wcol = vcol - vs->vwincol1;
	WindowSetCursor(vs->winnum, wrow, wcol);
	}


void VirtualSetPhysical(short vsnum, Rect &srectp) {
	WindowSetPhysical(VGetWinNum(vsnum), srectp);
	}


void VirtualSetTitle(short vsnum, char *title) {
	WindowSetTitle(VGetWinNum(vsnum), title);
	}


void VirtualShow(short vsnum) {
	WindowTop(VGetWinNum(vsnum));
	}


// VirtualShowCursor -- turn cursor on or off

void VirtualShowCursor(short vsnum, Boolean on) {
	if (vsnum == BADVSCREEN)
		return;
	WindowShowCursor(vscreen[vsnum]->winnum, on);
	}


// VirtualSlide -- slide rectangle (always succeeds)

void VirtualSlide(short vsnum, Rect &vrectp, DIRECTION dir, short dist) {
	VSCREEN	*vs;
	Rect	vrect, srect, wrect;
	short	vr, vr1, vr2, vc1, vc2;
	CELL	*tmp;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	if (!vrect.Intersect(vrectp, vs->vrow2, vs->vcol2) || dist <= 0)
		return;
	WindowGetPhys(vs->winnum, srect, False);
	switch (dir) {
		case D_UP:
			vrect.SetTop(max(vrect.Top(), dist));
			if (vrect.Lef()==0 && vrect.Rig()==vs->vcol2) {
				for (vr=vrect.Top(); vr<=vrect.Bot(); vr++) {
					tmp = vs->ca[vr-dist];
					vs->ca[vr-dist] = vs->ca[vr];
					vs->ca[vr] = tmp;	// contents have no significance
					}
				}
			else
				for (vr=vrect.Top(); vr<=vrect.Bot(); vr++)
					movmem((char *)&vs->ca[vr][vrect.Lef()], (char *)&vs->ca[vr-dist][vrect.Lef()], sizeof(CELL)*vrect.Width());
			wrect.Set(vrect.Top()-vs->vwinrow1, vrect.Lef()-vs->vwincol1, vrect.Bot()-vs->vwinrow1, vrect.Rig()-vs->vwincol1);
			if (WindowSlide(vs->winnum, wrect, D_UP, dist))
				vr1 = max(vs->vwinrow1 + srect.Height(),vrect.Top()) - dist;
			else
				vr1 = vrect.Top()-dist;
			wrect.SetTop(vr1 - vs->vwinrow1);
			wrect.SetBot(vrect.Bot() - dist - vs->vwinrow1);
			break;
		case D_DOWN:
			vrect.SetBot(min(vrect.Bot(), vs->vrow2-dist));
			if (vrect.Lef()==0 && vrect.Rig()==vs->vcol2) {
				for (vr=vrect.Bot(); vr>=vrect.Top(); vr--) {
					tmp = vs->ca[vr+dist];
					vs->ca[vr+dist] = vs->ca[vr];
					vs->ca[vr] = tmp;
					}
				}
			else
				for (vr=vrect.Bot(); vr>=vrect.Top(); vr--)
					movmem((char *)&vs->ca[vr][vrect.Lef()], (char *)&vs->ca[vr+dist][vrect.Lef()], sizeof(CELL)*(vrect.Rig()-vrect.Lef()+1));
			wrect.Set(vrect.Top()-vs->vwinrow1, vrect.Lef()-vs->vwincol1, vrect.Bot()-vs->vwinrow1, vrect.Rig()-vs->vwincol1);
			if (WindowSlide(vs->winnum, wrect, D_DOWN, dist))
				vr2 = min(vs->vwinrow1-1, vrect.Bot()) + dist;
			else
				vr2 = vrect.Bot() + dist;
			wrect.SetTop(vrect.Top() + dist - vs->vwinrow1);
			wrect.SetBot(vr2 - vs->vwinrow1);
			break;
		case D_LEFT:
			vrect.SetLef(max(vrect.Lef(), dist));
			for (vr=vrect.Top(); vr<=vrect.Bot(); vr++)
				movmem((char *)&vs->ca[vr][vrect.Lef()], (char *)&vs->ca[vr][vrect.Lef()-dist], sizeof(CELL)*(vrect.Rig()-vrect.Lef()+1));
			wrect.Set(vrect.Top()-vs->vwinrow1, vrect.Lef()-vs->vwincol1, vrect.Bot()-vs->vwinrow1, vrect.Rig()-vs->vwincol1);
			if (WindowSlide(vs->winnum, wrect, D_LEFT, dist))
				vc1 = max(vs->vwincol1+srect.Width(), vrect.Lef()) - dist;
			else
				vc1 = vrect.Lef() - dist;
			wrect.SetLef(vc1 - vs->vwincol1);
			wrect.SetRig(vrect.Rig() - dist - vs->vwincol1);
			break;
		case D_RIGHT:
			vrect.SetRig(min(vrect.Rig(), vs->vcol2-dist));
			for (vr=vrect.Top(); vr<=vrect.Bot(); vr++)
				movmem((char *)&vs->ca[vr][vrect.Lef()], (char *)&vs->ca[vr][vrect.Lef()+dist], sizeof(CELL)*(vrect.Rig()-vrect.Lef()+1));
			wrect.Set(vrect.Top()-vs->vwinrow1, vrect.Lef()-vs->vwincol1, vrect.Bot()-vs->vwinrow1, vrect.Rig()-vs->vwincol1);
			if (WindowSlide(vs->winnum, wrect, D_RIGHT, dist))
				vc2 = min(vs->vwincol1-1, vrect.Rig()) + dist;
			else
				vc2 = vrect.Rig() + dist;
			wrect.SetLef(vrect.Lef() + dist - vs->vwincol1);
			wrect.SetRig(vc2 - vs->vwincol1);
			break;
		}
	VirtualRedraw(vs->winnum, wrect);
	}


// VirtualPrint -- write formated output to the virtual screen.  Restrict
// total length to 99 bytes.

void VirtualPrint(short vsnum, short vrow, short vcol, short ncols, short attr, char *format, ...) {
	va_list	ptr;
	char	buff[100];

	va_start(ptr, format);
	vsprintf(buff, format, ptr);
	va_end(ptr);
	VirtualWrite(vsnum, vrow, vcol, ncols, buff, attr);
	}


// VirtualWrite -- write string
//   vsnum  -  virtual screen
//   vrow1  -  starting row
//   vcol1  -  starting column
//   ncols  -  number of columns to write
//   str    -  string
//   attr   -  attribute

void VirtualWrite(short vsnum, short vrow1, short vcol1, short ncols, char *str, short attr) {
	VSCREEN	*vs;
	CELL	*cellp;
	short	i;
	Boolean	pad;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	if (vrow1<0 || vrow1>vs->vrow2 || vcol1<0 || vcol1>vs->vcol2)
		return;
	pad = (ncols == GIANT) ? False : True;
	if (vcol1 + ncols > vs->vcol2 + 1)
		ncols = vs->vcol2 - vcol1 + 1;
	for (i=0; i<ncols && str[i]!='\0'; i++) {
		cellp = &vs->ca[vrow1][vcol1+i];
		cellp->ch = str[i];
		cellp->attr = (char)attr;
		}
	if (pad && i<ncols)
		VirtualRepCell(&vs->ca[vrow1][vcol1+i], ' ', attr, ncols-i);
	else
		ncols = i;
	if (vcol1 < vs->vwincol1) {
		ncols -= vs->vwincol1 - vcol1;
		vcol1 = vs->vwincol1;
		}
	WindowWriteCells(vs->winnum, vrow1-vs->vwinrow1, vcol1-vs->vwincol1, &vs->ca[vrow1][vcol1], ncols);
	}


// VirtualWrite2 -- write string with dual attributes

void VirtualWrite2(short vsnum, short vrow1, short vcol1, short ncols, char *str, short lo, short hi) {
	VSCREEN	*vs;
	CELL	*cellp;
	short	len, color;
	Boolean	pad;

	if (vsnum == BADVSCREEN)
		return;
	vs = vscreen[vsnum];
	if (vrow1<0 || vrow1>vs->vrow2 || vcol1<0 || vcol1>vs->vcol2)
		return;
	pad = (ncols == GIANT) ? False : True;
	if (vcol1 + ncols > vs->vcol2 + 1)
		ncols = vs->vcol2 - vcol1 + 1;
	color = (char)lo;
	for (len=0; *str && len<ncols; str++) {
		if (*str == '{')
			color = (char)hi;
		else if (*str == '}')
			color = (char)lo;
		else {
			cellp = &vs->ca[vrow1][vcol1+len];
			cellp->ch = *str;
			cellp->attr = color;
			len++;
			}
		}
	if (pad && len<ncols)
		VirtualRepCell(&vs->ca[vrow1][vcol1+len], ' ', lo, ncols-len);
	else
		ncols = len;
	if (vcol1 < vs->vwincol1) {
		ncols -= vs->vwincol1 - vcol1;
		vcol1 = vs->vwincol1;
		}
	WindowWriteCells(vs->winnum, vrow1-vs->vwinrow1, vcol1-vs->vwincol1, &vs->ca[vrow1][vcol1], ncols);
	}
