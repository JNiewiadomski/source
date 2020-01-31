// **********
// * TVideo *
// **********

// The Video class takes care of saving the contents of the video screen and
// the cursor location.  When the program terminates, the Video class will
// restore the screen contents and cursor location.

#include <common\tvideo.h>

// ********************
// * Global Variables *
// ********************

TVideo	gVideo;

// ***************
// * Video Class *
// ***************

TVideo::~TVideo(void) {
	if (fScreen == 0)
		return;
	puttext(fTI.winleft, fTI.wintop, fTI.winright, fTI.winbottom, fScreen);
	delete fScreen;
	gotoxy(fTI.curx, fTI.cury);
	fScreen = 0;
	}


void TVideo::Save(void) {
	gettextinfo(&fTI);
	fScreen = new char[fTI.screenheight * fTI.screenwidth * 2];
	if (!fScreen)
		return;
	gettext(fTI.winleft, fTI.wintop, fTI.winright, fTI.winbottom, fScreen);
	}
