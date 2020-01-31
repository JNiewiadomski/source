// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\error.h>
#include <seashell\event.h>

#define MAX_EVENTS	20

Boolean C_Event::Post(Word what, Word message) {
	EventQueue	*ep, **nep;

	if (what==EVT_KEY) {
		if (saver.Active())
			return (False);
		}
	if ((what & theEvMask) == 0)
		return (False);
	if (numEvents >= MAX_EVENTS) {
		ErrorSound();
		return (False);
		}
	if ((ep = new EventQueue) == NULL)
		return (False);
	ep->link = NULL;
	ep->ev.what = what;
	ep->ev.message = message;
	mouse.Location(&ep->ev.where);
	ep->ev.modifiers = KeyboardStatus(kKS_EVERY) | (mouse.Button() << 8);
	nep = &theEvQueue;
	while (*nep)
		nep = &((*nep)->link);
	*nep = ep;
	numEvents++;
	return (True);
	}
