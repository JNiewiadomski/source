// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\event.h>


void C_Event::Flush(Word eventMask, Word stopMask) {
	EventQueue	**ep, *tep;

	Register();
	ep = &theEvQueue;
	while (*ep && ((*ep)->ev.what & ~stopMask))
		if ((*ep)->ev.what & eventMask) {
			tep = (*ep)->link;
			delete *ep;
			*ep = tep;
			numEvents--;
			}
		else
			ep = &((*ep)->link);
	if (eventMask == EVT_KEY)
	while (KeyboardAvail())			// flush the keyboard buffer
		KeyboardRead();
	}
