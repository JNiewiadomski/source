#include <ctype.h>

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\event.h>
#include <seashell\list.h>
#include <seashell\window.h>

#define LS_NONE	-1

static short	listLow, listHigh, listInvLow;


static void ListHilite(ListStruct *ls, Boolean hilite);


void ListDisplay(ListStruct *ls) {
	short	num, i;

	num = (ls->count > ls->height) ? ls->height : ls->count;
	for (i=0; i<num; i++)
		VirtualWrite(ls->vsnum, ls->top+i, ls->lef+1, ls->width, ls->list[ls->loc+i], listHigh);
	}


static void ListHilite(ListStruct *ls, Boolean hilite) {
	VirtualPrint(ls->vsnum, ls->top+ls->pos-ls->loc, ls->lef, ls->width+2, hilite ? listInvLow : listHigh, " %-*s ", ls->width, ls->list[ls->pos]);
	}


void ListInit(void) {
	listLow = ColorGet(_DialogCOLOR, _COLOR_Low);
	listHigh = ColorGet(_DialogCOLOR, _COLOR_High);
	listInvLow = ColorGet(_DialogCOLOR, _COLOR_InvLow);
	}


short ListScroll(ListStruct *ls) {
	Rect	r;
	short	num, nextPos, pos;
	Boolean	done;
	C_Event	event;
	char	ch;

	r.Set(ls->top, ls->lef, ls->top+ls->height-1, ls->lef+ls->width-1);
	VirtualFill(ls->vsnum, r, ' ', listHigh);
	num = (ls->count > ls->height) ? ls->height : ls->count;
	nextPos = ls->pos;
	ls->pos = LS_NONE;
	ListDisplay(ls);
	done = False;
	while (!done) {
		if (ls->pos != nextPos) {
			if (ls->pos != LS_NONE)
				ListHilite(ls, False);
			ls->pos = nextPos;
			if (ls->pos != LS_NONE) {
				if (ls->pos < ls->loc) {
					ls->loc = ls->pos;
					ListDisplay(ls);
					}
				else if (ls->pos > (ls->loc+num-1)) {
					ls->loc = ls->pos-num+1;
					ListDisplay(ls);
					}
				ListHilite(ls, True);
				}
			}
		while (!event.GetNext(EVT_KEY))
			;
		switch (event.Message()) {
			case kK_ESCAPE:
			case kK_TAB:
			case kK_BACKTAB:
				done = True;
				break;
			case kK_RETURN:
				if (ls->count > 0)
					done = True;
				break;
			case kK_HOME:
				nextPos = 0;
				break;
			case kK_PGUP:
				nextPos = ls->pos-num+1;
				if (nextPos < 0)
					nextPos = 0;
				break;
			case kK_UP:
				if (ls->pos > 0)
					nextPos = ls->pos-1;
				break;
			case kK_DOWN:
				if (ls->pos < ls->count-1)
					nextPos = ls->pos+1;
				break;
			case kK_PGDN:
				nextPos = ls->pos+num-1;
				if (nextPos > ls->count-1)
					nextPos = ls->count-1;
				break;
			case kK_END:
				nextPos = ls->count-1;
				break;
			case kK_SPACE:
				nextPos = ls->pos+1;
				if (nextPos > ls->count-1)
					nextPos = 0;
				break;
			default:
				ch = toupper(event.Message());
				pos = ls->pos;
				do {
					pos = (pos == ls->count) ? 0 : pos+1;
					} while ((pos != ls->pos) && (*(ls->list[pos]) != ch));
				if (pos != ls->pos)
					nextPos = pos;
				break;
			}
		}
	if (ls->pos != LS_NONE)
	ListHilite(ls, False);
	return (event.Message());
	}
