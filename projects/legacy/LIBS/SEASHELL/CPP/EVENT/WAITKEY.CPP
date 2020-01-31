// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\event.h>

void C_Event::WaitKey(void) {
	while (!GetNext(EVT_KEY))
		;
	}
