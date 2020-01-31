#ifndef __CONIO_H
#include <conio.h>
#endif

#ifndef __DOS_H
#include <dos.h>
#endif

#ifndef _BIOS_H
#include <..\base\inc\bios.h>
#endif

enum { kAttrLow, kAttrHigh, kAttrInv, kAttrNum };

// ****************
// * CVideo Class *
// ****************

class CVideo {
	Byte	fRows, fCols;		// screen size
	short	fPage;			// video page
	short	fMode;			// video mode
	short	fAttr[kAttrNum];	// video attributes
public:
	CVideo();

	void	ClearScreen() { clrscr(); }
	void	Box(short top, short lef, short bot, short rig, short attr);
	short	GetCols() { return (fCols); }
	short	GetMode() { return (fMode); }
	short	GetRows() { return (fRows); }
	void	Goto(short row, short col) { _AH=kCur_Pos; _BH=fPage; _DL=col; _DH=row; geninterrupt(kVideo_IO); }
	void	HideCursor() { _AH=kCur_Pos; _BH=fPage; _DH=fRows; _DL=0; geninterrupt(kVideo_IO); }
	void	PutAttr(short row, short col, short attr, short num);
	void	PutCharAttr(short row, short col, short ch, short attr, short num);
	void	PutStrAttr(short row, short col, const char *s, short attr);
	void	SetAttr(short low, short high);
	};
