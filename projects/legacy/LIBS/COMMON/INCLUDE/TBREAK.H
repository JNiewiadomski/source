#ifndef _TBREAK_H
#define _TBREAK_H

// ****************
// * TBreak Class *
// ****************

// The TBreak class takes care of disabling the DOS generic Ctrl-Break and
// Ctrl-C exit mechanism.  This allows the keys to be recognized by the
// KeyboardRead routine.

class TBreak {
	Boolean	fInstalled;		// has Init() been called?
	isrFunc	fOld_Adr;		// old handlers' vector
public:
	~TBreak(void) { CleanUp(); }

	void	CleanUp(void);
	void	Init(void);
	};

#endif
