#include <bios.h>
#include <dos.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\old\coord.h>

#include <seashell\clock.h>
#include <seashell\iu.h>
#include <seashell\resclass.h>

#define I_TIC		0x1C
#define MAX_TICS	1080
#define TIMER_HI	0x046E
#define TIMER_LO	0x046C
#define BLINK_RATE	18

#define TIME_GET	0
#define TIME_SET	1

typedef void interrupt (*OldClockFunc)(...);

Boolean	clockChanged;
char	clockStr[6];

static char		clSep;
static Word		tics, blinkCountdown;
static OldClockFunc	oldClock;
static Boolean		blinkOn;

static void	ClockCleanUp(void);
static void	interrupt iClock(...);


// ClockCleanUp -- restores the clock vector that was found before clock
// was installed.

static void ClockCleanUp(void) {
	setvect(I_TIC, oldClock);
	clockChanged = False;
	}


// Call ClockInit with the location of the clock as its arguments.
// The time (hours and minutes) will be updated once every minute by
// interrupt.

void ClockInit(void) {
	ResInternat	*r_iu;

	r_iu = (ResInternat *) IUGet();
	clSep = r_iu->time;
	blinkCountdown = BLINK_RATE;
	blinkOn = True;
	clockStr[5] = '\0';
	clockChanged = False;
	tics = MAX_TICS;
	oldClock = (OldClockFunc)getvect(I_TIC);
	setvect(I_TIC, iClock);
	atexit(ClockCleanUp);
	}


// clock -- called approximately 18.2 times a second via interrupt.
// blinkCountdown is used to regulate the time separator to blink
// about once every two seconds.

static void interrupt iClock(...) {
	int	clTime, clHrs, clMin;

	(*oldClock)();
	tics++;
	blinkCountdown--;
	if (tics >= MAX_TICS) {
		clTime = (int)((biostime(TIME_GET, 0L) * 100L) / 109239L);
		clHrs = clTime / 60;
		clMin = clTime % 60;
		clockStr[0] = (clHrs > 9) ? (clHrs / 10)+'0' : ' ';
		clockStr[1] = (clHrs % 10) + '0';
		clockStr[3] = (clMin / 10) + '0';
		clockStr[4] = (clMin % 10) + '0';
		clockChanged = True;
		tics = 0;
		}
	if (blinkCountdown<1) {
		blinkOn = blinkOn ? False : True;
		clockStr[2] = blinkOn ? ' ' : clSep;
		clockChanged = True;
		blinkCountdown = BLINK_RATE;
		}
	}
