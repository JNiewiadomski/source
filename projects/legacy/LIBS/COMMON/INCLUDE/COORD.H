#ifndef _COORD_H
#define _COORD_H

// ******************************
// * Coordinate System Routines *
// ******************************

// Point coordinate system class

class Point {
	short	v, h;			// vertical and horizontal coordinates
public:
	short	H(void) { return (h); }
	short	V(void) { return (v); }
	void	Set(short vert, short horiz) { v=vert; h=horiz; }
	void	SetH(short horiz) { h=horiz; }
	void	SetV(short vert) { v=vert; }
	};


// Rectangle coordinate system class

// Inset() shrinks or expands a rectangle while leaving it centered at the
// same position.  The given horizontal and vertical insets are added to the
// rectangle's top-left corner and subtracted from its bottom-right corner.
// A positive inset in either dimension shrinks the rectangle in that
// dimension; a negative inset expands it.

// Offset() moves a rectangle to a new position within its coordinate system
// without affecting its width amd height.  The given horizontal and vertical
// offsets are added to both the rectangle's top-left and bottom-right
// corners.  A positive horizontal offset moves the rectangle to the right,
// negative to the left; a positive vertical offset moves the rectangle
// downward, a negative one moves it upward.

class Rect {
	short	top, lef, bot, rig;	// top, left, bottom and right coordinates
public:
	short	Bot(void) { return (bot); }
	short	Height(void) { return (bot - top + 1); }
	void	Inset(short dv, short dh) { top+=dv; lef+=dh; bot-=dv; rig-=dh; }
	Boolean	Intersect(Rect &src, short maxrow, short maxcol);
	short	Lef(void) { return (lef); }
	void	Offset(short dv, short dh) { top+=dv; lef+=dh; bot+=dv; rig+=dh; }
	short	Rig(void) { return (rig); }
	void	Set(short t, short l, short b, short r) { top=t; lef=l; bot=b; rig=r; }
	void	Set(Point top_lef, Point bot_rig) { top=top_lef.V(); lef=top_lef.H(); bot=bot_rig.V(); rig=bot_rig.H(); }
	void	SetBot(short b) { bot=b; }
	void	SetLef(short l) { lef=l; }
	void	SetRig(short r) { rig=r; }
	void	SetTop(short t) { top=t; }
	short	Size(void) { return ((rig - lef + 1) * (bot - top + 1)); }
	short	Top(void) { return (top); }
	void	Union(Rect &src1, Rect &src2);
	short	Width(void) { return (rig - lef + 1); }
	};

#endif
