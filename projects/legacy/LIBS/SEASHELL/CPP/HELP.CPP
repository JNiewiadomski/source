#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\printer.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\status.h>
#include <seashell\string.h>
#include <seashell\window.h>

struct HelpStruct {
	Word		num;
	char		*title;
	HelpStruct	*prev;
	};

static HelpStruct	*theHelp=NULL;
static short		helpLow, helpHigh, helpHot, vsPeekHelp;
static Boolean		inHelp=False, peekHelp=False;
static int		helpHotKey;
static Rect		rPeek;

static void	HelpShowPeek(void);


// HelpGet -- call this routine to display the help text on screen.  Pass
// along helpID to identify which help resource text will be displayed,
// and helpTitle for the title to be displayed.  The screen is restored
// when the <Esc> key is pressed.  If <^P> is pressed and a printer had
// been selected through the Printer Manager, then the entire help text
// will be transmitted to the printer.

void HelpGet(void) {
	ResHelp	*r_help;
	int	hCount;
	Rect	r;
	short	vsnum, hNum, hPtr, hNextPtr, hPages, i;
	Boolean	done, ok;
	char	hText[RES_HELP_LEN+1], *htp, **hLines;
	char	hMessage[RES_HELP_LEN+1];
	C_Event	event;

	if (inHelp || theHelp == NULL)
		return;
	if ((r_help = (ResHelp *) ResLoad(R_Help, theHelp->num)) == NULL)
		return;
	inHelp = True;
	hCount = r_help->num;
	hLines = (char **) r_malloc(sizeof(char *) * hCount);
	htp = (char *) r_help->help;
	for (i=0; i<hCount; i++) {
		hLines[i] = htp;
		while (*htp++)
			;
		}
	strcpy(hMessage, StringGet(_HelpSTRING, _HelpSTRING_Page));
	vsnum = VirtualGetNew(_HelpWINDOW, &r);
	sprintf(hText, StringGet(_HelpSTRING, _HelpSTRING_Title), theHelp->title);
	VirtualSetTitle(vsnum, hText);
	StatusPush("");
	hNum = (hCount > r.Height()) ? r.Height() : hCount;
	hPtr = -1;
	hNextPtr = hPtr+1;
	hPages = ((hCount-1)/hNum)+1;
	done = False;
	while (!done) {
		if (hPtr != hNextPtr) {
			VirtualFill(vsnum, GIANT_RECT, ' ', helpHigh);
			hPtr = hNextPtr;
			for (i=0; i<hNum; i++)
				VirtualWrite2(vsnum, i, 1, GIANT, hLines[hPtr+i], helpHigh, helpHot);
			sprintf(hText, hMessage, ((hPtr+hNum-1)/hNum)+1, hPages);
			StatusSet(hText);
			ScreenSynch();
			}
		while (!event.GetNext(EVT_EVERY))
			;
		if (event.What() == EVT_KEY)
			switch (event.Message()) {
				case kK_ESCAPE:
					done = True;
					break;
				case kK_HOME:
					hNextPtr = 0;
					break;
				case kK_UP:
					if (hPtr > 0)
						hNextPtr = hPtr-1;
					break;
				case kK_PGUP:
					hNextPtr = hPtr-hNum+1;
					if (hNextPtr < 0)
						hNextPtr = 0;
					break;
				case kK_DOWN:
					if (hPtr < hCount-hNum)
						hNextPtr = hPtr+1;
					break;
				case kK_PGDN:
					hNextPtr = hPtr+hNum-1;
					if (hNextPtr > hCount-hNum)
						hNextPtr = hCount-hNum;
					break;
				case kK_END:
					hNextPtr = hCount-hNum;
					break;
				default:
					if ((event.Message() == PrinterGetKey()) && (PrinterOpen())) {
						ok = PrinterWrite(StringGet(_HelpSTRING, _HelpSTRING_Title), theHelp->title);
						PrinterWrite(".\n\n");
						for (i=0; i<hCount && ok; i++) {
							ok = PrinterWrite(hLines[i]);
							PrinterWrite("\n");
							}
						PrinterWrite("\f");
						PrinterClose();
						}
					break;
				}
		}
	StatusPop();
	r_free(hLines);
	VirtualDispose(vsnum);
	ResUnLoad(R_Help, theHelp->num);
	inHelp = False;
	}


int HelpGetKey(void) {
	return (helpHotKey);
	}


Boolean HelpGetPeek(void) {
	return (peekHelp);
	}


// HelpInit -- call this routine once before using any other routines
// from the Help Manager.  This routine assignes the appropriate color
// attributes with regards to the current video mode.

void HelpInit(int key) {
	helpLow = ColorGet(_HelpCOLOR, _COLOR_Low);
	helpHigh = ColorGet(_HelpCOLOR, _COLOR_High);
	helpHot = ColorGet(_HelpCOLOR, _COLOR_Hot);
	helpHotKey = key;
	}


void HelpPop(void) {
	HelpStruct	*hp;

	if (!theHelp)
		return;
	hp = theHelp;
	theHelp = hp->prev;
	r_free(hp->title);
	r_free(hp);
	HelpShowPeek();
	}


void HelpPush(Word num, char *title) {
	HelpStruct	*hp;

	hp = (HelpStruct *) r_malloc(sizeof(HelpStruct));
	hp->num = num;
	hp->title = (char *) r_malloc(strlen(title)+1);
	strcpy(hp->title, (title==NULL) ? "?" : title);
	hp->prev = theHelp;
	theHelp = hp;
	HelpShowPeek();
	}


void HelpSet(Word num, char *title) {
	HelpStruct	*hp;

	if (theHelp)
		r_free(theHelp->title);
	else {
		hp = (HelpStruct *) r_malloc(sizeof(HelpStruct));
		hp->prev = theHelp;
		theHelp = hp;
		}
	theHelp->num = num;
	theHelp->title = (char *) r_malloc(strlen(title)+1);
	strcpy(theHelp->title, (title==NULL) ? "?" : title);
	HelpShowPeek();
	}


void HelpSetKey(int key) {
	helpHotKey = key;
	}


void HelpSetPeek(Boolean on) {
	if (peekHelp == on)
		return;
	peekHelp = on;
	if (peekHelp) {
		vsPeekHelp = VirtualGetNew(_HelpPeekWINDOW, &rPeek);
		HelpShowPeek();
		}
	else
		VirtualDispose(vsPeekHelp);
	}


static void HelpShowPeek(void) {
	ResHelp	*r_help;
	int	hCount;
	short	i;
	char	*htp;

	if (!peekHelp || inHelp || theHelp == NULL)
		return;
	if ((r_help = (ResHelp *) ResLoad(R_Help, theHelp->num)) == NULL)
		return;
	hCount = r_help->num;
	VirtualFill(vsPeekHelp, GIANT_RECT, ' ', helpHigh);
	htp = r_help->help;
	for (i=0; i<rPeek.Height() && i<hCount; i++) {
		VirtualWrite2(vsPeekHelp, i, 1, GIANT, htp, helpHigh, helpHot);
		while (*htp++)
			;
		}
	ScreenSynch();
	}
