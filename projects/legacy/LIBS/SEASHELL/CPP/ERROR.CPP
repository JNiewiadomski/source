// The Error module provides routines to issue error messages to the user.
// The Resource and Window managers are utilized if they have been
// initialized.

#include <alloc.h>
#include <ctype.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>
#include <common\old\critical.h>
#include <common\old\keyboard.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\error.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\string.h>
#include <seashell\window.h>

#define CRIT_ERR_MAX	13

#define ERR_CENTER	((eRect.Rig() - eRect.Lef()) / 2)

static short		eVirt, errLow, errHigh, errInv;
static Rect		eRect;
static char		*critErr[CRIT_ERR_MAX], *critCritErr, *critAbort;
static char		*critRetry, *critFail;
static CleanUpFunc	CleanUp=NULL;


Byte CriticalErrorHandler(Word errCode) {
	Byte	ch;
	char	buff[RES_ERR_LEN+1];
	Boolean	done;

	VirtualSelect(eVirt);
	VirtualFill(eVirt, GIANT_RECT, ' ', errLow);
	VirtualWrite(eVirt, 0, 1, GIANT, " ! ", errInv | BLINK);
	VirtualWrite(eVirt, 0, ERR_CENTER-(strlen(critCritErr)/2), GIANT, critCritErr, errLow);
	VirtualPrint(eVirt, 0, 47, GIANT, errLow, "0x%04X", errInfo);
	VirtualWrite(eVirt, 2, ERR_CENTER-(strlen(critErr[errCode])/2), GIANT, critErr[errCode], errHigh);
	strcpy(buff, critAbort);
	if (errInfo & BIT_RETRY)
		strcat(buff, critRetry);
	if ((_version >= 0x0310) && (errInfo & BIT_FAIL))
		strcat(buff, critFail);
	VirtualWrite2(eVirt, 4, ERR_CENTER-(HiliteStrLen(buff)/2), GIANT, buff, errLow, errHigh);
	ScreenSynch();
	ErrorSound();
	done = False;
	while (!done) {
		_AH = 0x07;
		geninterrupt(0x21);
		ch = _AL;
		ch = toupper(ch);
		if (ch == critAbort[1])
			done = True;
		else if (ch == critFail[3]) {
			if ((_version >= 0x0310) && (errInfo & BIT_FAIL))
				done = True;
			}
		else if (ch == critRetry[3]) {
			if (errInfo & BIT_RETRY)
				done = True;
			}
		}
	VirtualHide(eVirt);
	ScreenSynch();
	return (ch);
	}


// Error() issues an error message.  If type is Err_Fatal, program execution
// is terminated, otherwise Error() returns to the caller.

void Error(Err_Type type, char *format, ...) {
	va_list	ptr;
	char	eBuff[RES_ERR_LEN+1];

	if (format==NULL || strlen(format)==0)
		strcpy(eBuff, StringGet(_ErrorSTRING, _ErrorSTRING_Sorry));
	else {
		va_start(ptr, format);
		vsprintf(eBuff, format, ptr);
		va_end(ptr);
		}
	if (VirtualAvail()) {
		VirtualSelect(eVirt);
		VirtualFill(eVirt, GIANT_RECT, ' ', errLow);
		VirtualWrite(eVirt, 0, 1, GIANT, (type==Err_Fatal) ? " ! " : " * ", errInv);
		VirtualWrite(eVirt, 2, ERR_CENTER-(strlen(eBuff)/2), GIANT, eBuff, errHigh);
		if (type == Err_Recover || type == Err_Warning)
			VirtualWrite2(eVirt, 4, 1, GIANT, StringGet(_ErrorSTRING, _ErrorSTRING_Resume), errLow, errHigh);
		if (type == Err_Fatal || type == Err_Recover) {
			strcpy(eBuff, StringGet(_ErrorSTRING, _ErrorSTRING_Abort));
			VirtualWrite2(eVirt, 4, eRect.Rig()-strlen(eBuff), GIANT, eBuff, errLow, errHigh);
			}
		sprintf(eBuff, StringGet(_ErrorSTRING, _ErrorSTRING_Free), coreleft());
		VirtualWrite(eVirt, 4, ERR_CENTER-(strlen(eBuff)/2), GIANT, eBuff, errLow);
		ScreenSynch();
		}
	else {
		fprintf(stderr, "%s\n", eBuff);
		if (type == Err_Recover || type == Err_Warning)
			fprintf(stderr, "%s  ", StringGet(_ErrorSTRING, _ErrorSTRING_Resume));
		if (type == Err_Fatal || type == Err_Recover)
			fprintf(stderr, "%s  ", StringGet(_ErrorSTRING, _ErrorSTRING_Abort));
		fprintf(stderr, StringGet(_ErrorSTRING, _ErrorSTRING_Free), coreleft());
		fprintf(stderr, "\n");
		}
	ErrorSound();
	while (KeyboardAvail())
		KeyboardRead();
	while (True) {
		while (!KeyboardAvail())
			;
		switch (KeyboardRead()) {
			case kK_RETURN:
				if (type == Err_Fatal || type == Err_Recover) {
					if (CleanUp)
						(*CleanUp)();
					exit(EXIT_FAILURE);
					}
				break;
			case kK_ESCAPE:
				if (type == Err_Recover || type == Err_Warning) {
					if (VirtualAvail()) {
						VirtualHide(eVirt);
						ScreenSynch();
						}
					return;
					}
				break;
			}
		}
	}


// ErrorGet() returns a pointer to the requested string from the current
// resource file.  If id or index do not reference an error resource, a
// pointer to the string "undefined" is returned.
//
// CAUTION: all pointers reference unique strings in memory, so no
// contentions should occur when referencing multiple strings.  However,
// the pointer may be invalidated whenever the resource manager needs to
// read in a new resource and memory needs to be purged.  If an error
// resource is marked as purgeable, always use ErrorGet() as a pointer to
// the resource.  If the resource is not marked as purgeable, it is correct
// programming practice to assign the pointer returned to a character
// pointer and reference the string by the character pointer.

char *ErrorGet(short id, short index) {
	ResError	*r_err;
	register char	*s;

	r_err = (ResError *) ResLoad(R_Error, id);
	if (r_err && index < r_err->num) {
		s = r_err->error;
		while (index--)
			s = strchr(s, '\0') + 1;	// scan for next string
		}
	else
		s = "undefined";
	return (s);
	}


// ErrorInit -- initializes the error message window and registers a critical
// error handler.

void ErrorInit(CleanUpFunc func) {
	short	i;

	errLow = ColorGet(_ErrorCOLOR, _COLOR_Low);
	errHigh = ColorGet(_ErrorCOLOR, _COLOR_High);
	errInv = ColorGet(_ErrorCOLOR, _COLOR_InvLow);
	CleanUp = func;
	CriticalInstall(CleanUp);
	eVirt = VirtualGetNew(_ErrorWINDOW, &eRect);
	VirtualHide(eVirt);
	critCritErr = ErrorGet(_CriticalERROR, _CriticalERROR_Message);
	critAbort = ErrorGet(_CriticalERROR, _CriticalERROR_Abort);
	critRetry = ErrorGet(_CriticalERROR, _CriticalERROR_Retry);
	critFail = ErrorGet(_CriticalERROR, _CriticalERROR_Fail);
	for (i=0; i<CRIT_ERR_MAX; i++)
		critErr[i] = ErrorGet(_CriticalCodeERROR, i);
	}


// ErrorSound() turns on the speaker at a certain frequency for a short
// duration.

void ErrorSound(void) {
	TTimer	delay;

	sound(400);
	delay.Reset(3L);
	delay.Wait(NULL);
	sound(300);
	delay.Reset(3L);
	delay.Wait(NULL);
	nosound();
	}
