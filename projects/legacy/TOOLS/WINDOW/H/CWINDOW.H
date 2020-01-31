#ifndef _CWINDOW_H
#define _CWINDOW_H

#ifndef __CONIO_H
#include <conio.h>
#endif

#ifndef __IOSTREAM_H
#include <iostream.h>
#endif

#ifndef _CBASE_H
#include <base\include\cbase.h>
#endif

// *****************
// * CWindow Class *
// *****************

class CWindow {
	struct text_info	fOriginalScreen;
	char			*fOriginalData;
	Boolean			fIsOpen;
	int			fHeight, fWidth;
	int			fTop, fLef, fBot, fRig;
	int			fStyle;
	int			fTextAttr, fBorderAttr;

	void	Box();
public:
	enum { kDouble, kSingle, kStyleC, kStyleD, kSolid, kMax };

	CWindow() { fIsOpen = false; }
	~CWindow() { Close(); }

	void	Close();
	void	Goto(int y, int x) { gotoxy(x, y); }
	Boolean	Open(int height, int width, int style, int text_attr, int border_attr);

	friend CWindow	&operator<<(CWindow &win, char c);
	friend CWindow	&operator<<(CWindow &win, const char *s);
	friend CWindow	&operator<<(CWindow &win, istream &in);
	};

#endif