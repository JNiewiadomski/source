// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\event.h>

Boolean C_Event::Avail(Word eventMask) {
	EventQueue	**ep;

	Register();
	ep = &theEvQueue;
	while (*ep) {
		if ((*ep)->ev.what & eventMask) {
			theEvent = (*ep)->ev;
			return (True);
			}
		ep = &((*ep)->link);
		}
	Null();
	return (False);
	}
