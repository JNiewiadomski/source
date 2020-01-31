// ****************
// * TBreak Class *
// ****************

// The TBreak class takes care of disabling the DOS generic Ctrl-Break and
// Ctrl-C exit mechanism.  This prevents programs from terminating
// prematurely.  The keys will be returned by the BIOS keyboard routines,
// allowing the application to recognize the keys and cleaning up before
// terminating.

#include <dos.h>

#include <common\common.h>
#include <common\biosint.h>
#include <common\tbreak.h>

// ******************************
// * Static Function Prototypes *
// ******************************

static void interrupt	ControlBreakDummy(...);


// ********************************
// * Dummy control break handler. *
// ********************************

static void interrupt ControlBreakDummy(...) {
	}


// ******************
// * TBreak Methods *
// ******************

// Restores original control break handler.

void TBreak::CleanUp(void) {
	if (!fInstalled)
		return;
	setvect(CTRL_BREAK, fOld_Adr);
	fInstalled = False;
	}


// Init() installs a dummy control break handler.  Since MS-DOS automatically
// restores the previous contents of the Int 23H vector from the information
// saved in the program segment prefix when the program terminates, we do not
// need to save its previous contents.  The Ctrl-Break handler, on the other
// hand, must be restored.  Thus, we save the previous vector contents before
// changing it.

void TBreak::Init(void) {
	if (fInstalled)
		return;
	fOld_Adr = getvect(CTRL_BREAK);
	setvect(CTRL_BREAK, ControlBreakDummy);
	setvect(CTRL_C, ControlBreakDummy);
	fInstalled = True;
	}
