#ifndef _TTIMER_H
#define _TTIMER_H

// ****************
// * TTimer Class *
// ****************

typedef void (*TimerIdleFunc)(void);

class TTimer {
	static DWord	current;		// updated by Time()
	DWord		timeout;		// time when we time out

	void	Time(void);
public:
	TTimer(DWord delay=0L) { Reset(delay); }

	Boolean	Elapsed(void);
	void	Reset(DWord delay) { Time(); timeout = current + delay; }
	void	Wait(TimerIdleFunc idle);
	};

#endif
