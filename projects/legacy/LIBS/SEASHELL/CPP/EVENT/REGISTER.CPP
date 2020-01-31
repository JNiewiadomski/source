// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\string.h>

static inline short EventGetKey(void) {
	short	ch;

	if ((ch = KeyboardRead()) == kK_CTRLBRK)
		Error(Err_Recover, StringGet(_EventSTRING, _EventSTRING_Break));
	return (ch);
	}


void C_Event::Register(void) {
	short	key;
	Word	button, change;

	while (KeyboardAvail())
		if ((key = EventGetKey()) == HelpGetKey())
			HelpGet();
		else
			Post(EVT_KEY, key);
	button = mouse.Button();
	change = button ^ theMouseButton;
	if (change) {
		if (change & MSG_MOUSE_LEFT)
			Post(button & MSG_MOUSE_LEFT ? EVT_MOUSE_DOWN : EVT_MOUSE_UP, MSG_MOUSE_LEFT);
		if (change & MSG_MOUSE_RIGHT)
			Post(button & MSG_MOUSE_RIGHT ? EVT_MOUSE_DOWN : EVT_MOUSE_UP, MSG_MOUSE_RIGHT);
		theMouseButton = button;
		}
	if (theTask)
		(*theTask)();
	}
