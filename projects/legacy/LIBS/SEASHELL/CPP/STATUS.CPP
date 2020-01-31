#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\error.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\status.h>
#include <seashell\video.h>
#include <seashell\window.h>

struct StatusStruct {
	char		*status;		// text string
	Boolean		free;			// free space when popped?
	StatusStruct	*prev;			// previous status link
	};

static StatusStruct	*theStatus=NULL;
static short		statusLow, statusHigh, statusHot, statusInvLow, statusInvHigh;
static short		statusVS;


static void	StatusCleanUp(void);
static void	StatusDisplay(short mLen);


static void StatusCleanUp(void) {
	while (theStatus != NULL)
		StatusPop();
	VirtualDispose(statusVS);
	}


static void StatusDisplay(short mLen) {
	short	sLen;
	Rect	r;

	if (theStatus == NULL)
		sLen = -1;
	else {
		VirtualWrite2(statusVS, 0, 1, GIANT, theStatus->status, statusInvLow, statusInvHigh);
		sLen = HiliteStrLen(theStatus->status);
		}
	if (mLen > sLen) {
		r.Set(0, sLen+1, 0, mLen);
		VirtualFill(statusVS, r, ' ', statusInvLow);
		}
	ScreenSynch();
	}


// StatusGet() returns a pointer to the requested string from the current
// resource file.  If id or index do not reference a status resource, a
// pointer to the string "undefined" is returned.
//
// CAUTION: all pointers reference unique strings in memory, so no
// contentions should occur when referencing multiple strings.  However,
// the pointer may be invalidated whenever the resource manager needs to
// read in a new resource and memory needs to be purged.  If a status
// resource is marked as purgeable, always use StatusGet() as a pointer to
// the resource.  If the resource is not marked as purgeable, it is correct
// programming practice to assign the pointer returned to a character
// pointer and reference the string by the character pointer.

char *StatusGet(short id, short index) {
	ResStatus	*r_stat;
	register char	*s;

	r_stat = (ResStatus *) ResLoad(R_Status, id);
	if (r_stat && index < r_stat->num) {
		s = r_stat->status;
		while (index--)
			s = strchr(s, '\0') + 1;	// scan for next string
		}
	else
		s = "undefined";
	return (s);
	}


// StatusInit -- initializes the Status manager.

void StatusInit(char *format, ...) {
	Rect	r;
	va_list	ptr;
	char	buff[RES_STAT_LEN+1];

	statusLow = ColorGet(_StatusCOLOR, _COLOR_Low);
	statusHigh = ColorGet(_StatusCOLOR, _COLOR_High);
	statusInvLow = ColorGet(_StatusCOLOR, _COLOR_InvLow);
	statusInvHigh = ColorGet(_StatusCOLOR, _COLOR_InvHigh);
	statusHot = ColorGet(_StatusCOLOR, _COLOR_Hot);
	statusVS = VirtualGetNew(_StatusWINDOW, &r);
	VirtualFill(statusVS, r, ' ', statusInvLow);
	va_start(ptr, format);
	vsprintf(buff, format, ptr);
	va_end(ptr);
	StatusPush(buff);
	atexit(StatusCleanUp);
	}


void StatusPop(void) {
	StatusStruct	*sp;
	short		i;

	if (theStatus == NULL)
		Error(Err_Warning, ErrorGet(_StatusERROR, _StatusERROR_Empty));
	sp = theStatus;
	i = HiliteStrLen(sp->status);
	theStatus = sp->prev;
	if (sp->free)
		r_free(sp->status);
	r_free(sp);
	StatusDisplay(i);
	}


void StatusPrint(short pos, char *format, ...) {
	va_list	ptr;
	char	buff[RES_STAT_LEN+1];

	va_start(ptr, format);
	vsprintf(buff, format, ptr);
	va_end(ptr);
	VirtualWrite2(statusVS, 0, pos, GIANT, buff, statusInvLow, statusInvHigh);
	ScreenSynch();
	}


void StatusPush(char *format, ...) {
	StatusStruct	*sp;
	short		i;
	char		status[RES_STAT_LEN+1];
	va_list		ptr;

	va_start(ptr, format);
	vsprintf(status, format, ptr);
	va_end(ptr);
	sp = (StatusStruct *) r_malloc(sizeof(StatusStruct));
	sp->status = (char *) r_malloc(strlen(status)+1);
	strcpy(sp->status, status);
	sp->free = True;
	sp->prev = theStatus;
	i = (theStatus == NULL) ? 0 : HiliteStrLen(theStatus->status);
	theStatus = sp;
	StatusDisplay(i);
	}


void StatusPush(short id, short index) {
	StatusStruct	*sp;
	short		i;

	sp = (StatusStruct *) r_malloc(sizeof(StatusStruct));
	sp->status = StatusGet(id, index);
	sp->free = False;
	sp->prev = theStatus;
	i = (theStatus == NULL) ? 0 : HiliteStrLen(theStatus->status);
	theStatus = sp;
	StatusDisplay(i);
	}


void StatusSet(char *status) {
	short	i;

	if (theStatus == NULL)
		Error(Err_Warning, ErrorGet(_StatusERROR, _StatusERROR_None));
	i = HiliteStrLen(theStatus->status);
	if (theStatus->free)
		r_free(theStatus->status);
	theStatus->status = (char *) r_malloc(strlen(status)+1);
	strcpy(theStatus->status, status);
	theStatus->free = True;
	StatusDisplay(i);
	}


void StatusSet(short id, short index) {
	short	i;

	if (theStatus == NULL)
		Error(Err_Warning, ErrorGet(_StatusERROR, _StatusERROR_None));
	i = HiliteStrLen(theStatus->status);
	if (theStatus->free)
		r_free(theStatus->status);
	theStatus->status = StatusGet(id, index);
	theStatus->free = False;
	StatusDisplay(i);
	}
