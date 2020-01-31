#include "h\cwindow.h"

// *****************
// * CWindow Class *
// *****************

const char	kBOX_TOP_LEF = 0;
const char	kBOX_TOP_MID = 1;
const char	kBOX_TOP_RIG = 2;
const char	kBOX_MID_LEF = 3;
const char	kBOX_MID_RIG = 4;
const char	kBOX_BOT_LEF = 5;
const char	kBOX_BOT_MID = 6;
const char	kBOX_BOT_RIG = 7;

const char pStyles[][8] = {
	{ 'É', 'Í', '»', 'º', 'º', 'È', 'Í', '¼' },
	{ 'Ú', 'Ä', '¿', '³', '³', 'À', 'Ä', 'Ù' },
	{ 'Õ', 'Í', '¸', '³', '³', 'À', 'Ä', 'Ù' },
	{ 'Ö', 'Ä', '·', 'º', 'º', 'Ó', 'Ä', '½' },
	{ 'Ü', 'Ü', 'Ü', 'Ý', 'Þ', 'ß', 'ß', 'ß' }
	};


void CWindow::Box() {
	char	*s, *box;
	short	i;

	box = new char[fWidth+1];
	if (!box)
		return;

	s = box;
	*s++ = pStyles[fStyle][kBOX_TOP_LEF];
	for (i=0; i < fWidth - 2; i++)
		*s++ = pStyles[fStyle][kBOX_TOP_MID];
	*s++ = pStyles[fStyle][kBOX_TOP_RIG];
	*s = '\0';
	gotoxy(1, 1);
	cprintf(box);
	for (i=2; i<fHeight; i++) {
		gotoxy(1, i);
		cprintf("%c%*s%c", pStyles[fStyle][kBOX_MID_LEF], fWidth-2, " ", pStyles[fStyle][kBOX_MID_RIG]);
		}
	s = box;
	*s++ = pStyles[fStyle][kBOX_BOT_LEF];
	for (i=0; i < fWidth-2; i++)
		*s++ = pStyles[fStyle][kBOX_BOT_MID];
	*s++ = pStyles[fStyle][kBOX_BOT_RIG];
	*s = '\0';
	gotoxy(1, fHeight);
	cprintf(box);

	delete[] box;
	}


void CWindow::Close() {
	if (!fIsOpen)
		return;
	//
	// Restore the old screen contents.
	//
	window(fOriginalScreen.winleft, fOriginalScreen.wintop, fOriginalScreen.winright, fOriginalScreen.winbottom);
	puttext(fLef, fTop, fRig, fBot, fOriginalData);
	gotoxy(fOriginalScreen.curx, fOriginalScreen.cury);
	//
	// clean up
	//
	delete[] fOriginalData;
	fIsOpen = false;
	}


Boolean CWindow::Open(int height, int width, int style, int text_attr, int border_attr) {
	if (fIsOpen || height < 3 || width < 3)
		return (false);
	fOriginalData = new char[height * width * 2];
	if (!fOriginalData)
		return (false);
	//
	// Record current state of the text screen, save the window contents,
	// and clear the window.
	//
	fHeight = height;
	fWidth = width;
	fStyle = style;
	fTextAttr = text_attr;
	fBorderAttr = border_attr;

	gettextinfo(&fOriginalScreen);
	fTop = (fOriginalScreen.screenheight / 2) - (fHeight / 2) + 1;
	fLef = (fOriginalScreen.screenwidth / 2) - (fWidth  / 2) + 1;
	fBot = fTop + fHeight - 1;
	fRig = fLef + fWidth  - 1;

	if (fTop < 1) fTop = 1;
	if (fLef < 1) fLef = 1;
	if (fBot > fOriginalScreen.screenheight) fBot = fOriginalScreen.screenheight;
	if (fRig > fOriginalScreen.screenwidth) fRig = fOriginalScreen.screenwidth;
	if (fStyle > kMax) fStyle = kDouble;

	gettext(fLef, fTop, fRig, fBot, fOriginalData);
	//
	// Draw the window boarder.
	//
	window(fLef, fTop, fRig, fBot);
	clrscr();
	textcolor(border_attr);
	_wscroll = 0;				// turn off auto-scroll
	Box();
	window(fLef+2, fTop+1, fRig-2, fBot-1);
	textcolor(text_attr);
	gotoxy(1, 1);

	fIsOpen = true;
	return (true);
	}


CWindow	&operator<<(CWindow &win, char c) {
	switch (c) {
		case '\f':
			clrscr();
			break;
		case '\n':
		case '\v':
			cprintf("\r\n");
			break;
		case '®':
			textbackground(win.fBorderAttr);
			break;
		case '¯':
			textbackground(BLACK);
			break;
		default:
			cprintf("%c", c);
			break;
		}
	return win;
	}


CWindow	&operator<<(CWindow &win, const char *s) {
	while (*s)
		win << *s++;
	return win;
	}


CWindow	&operator<<(CWindow &win, istream &in) {
	char	c;

	while (in.get(c))
		win << c;
	return win;
	}
