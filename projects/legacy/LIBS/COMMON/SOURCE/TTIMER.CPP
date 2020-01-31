// ****************
// * TTimer Class *
// ****************

#include <dos.h>

#include <common\common.h>
#include <common\dosint.h>
#include <common\ttimer.h>

// ******************************
// * TTimer Static Data Members *
// ******************************

DWord	TTimer::current=0L;		// updated by Time()

// ************************
// * TTimer Class Methods *
// ************************

// Elapsed returns true if we have timed out or the time is midnight when the
// clock turns over.  Otherwise false is returned.

Boolean TTimer::Elapsed(void) {
	DWord	previous;

	previous = current;
	Time();
	return (((current > timeout) || (previous > current)) ? True : False);
	}


// Time() sets the variable current to the current time in centiseconds.

void TTimer::Time(void) {
	union REGS	r;

	r.h.ah = GET_TIME;
	int86(BDOS_REQ, &r, &r);
	current  = r.h.dl;
	current += r.h.dh * 100L;
	current += r.h.cl * 6000L;
	current += r.h.ch * 360000L;
	}


// Wait() returns after the timer has timed out.  If idle is not NULL, the
// function pointed to it is repeatedly called.

void TTimer::Wait(TimerIdleFunc idle) {
	while (!Elapsed())
		if (idle)
			(*idle)();
	}
