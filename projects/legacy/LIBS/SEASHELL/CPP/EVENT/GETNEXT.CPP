// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\event.h>


Boolean C_Event::GetNext(Word eventMask) {
	EventQueue	**ep, *tep;

	Register();
	ep = &theEvQueue;
	while (*ep) {
		if ((*ep)->ev.what & eventMask) {
			theEvent = (*ep)->ev;
			tep = (*ep)->link;
			delete *ep;
			*ep = tep;
			numEvents--;
			return (True);
			}
		ep = &((*ep)->link);
		}
	Null();
	return (False);
	}
