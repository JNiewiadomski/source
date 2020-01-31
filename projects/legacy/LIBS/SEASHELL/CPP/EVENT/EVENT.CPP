// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\event.h>

// ********************
// * Static Variables *
// ********************

Boolean		C_Event::once = False;
C_Mouse		C_Event::mouse;
short		C_Event::numEvents;
C_Saver		C_Event::saver;
EventQueue	*C_Event::theEvQueue;
Word		C_Event::theMouseButton;
SysTaskFunc	C_Event::theTask;

// ***************
// * Event Class *
// ***************

C_Event::C_Event(void) {
	if (!once) {
		numEvents = 0;
		theEvQueue = NULL;
		theTask = NULL;
		theMouseButton = mouse.Button();
		once = True;
		}
	theEvMask = EVT_EVERY;
	Null();
	}
