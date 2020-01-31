// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\event.h>
#include <seashell\window.h>

void C_Event::Null(void) {
	theEvent.what = EVT_NULL;
	mouse.Location(&theEvent.where);
	theEvent.modifiers = KeyboardStatus(kKS_EVERY) | (mouse.Button() << 8);
	saver.Idle();
	}
