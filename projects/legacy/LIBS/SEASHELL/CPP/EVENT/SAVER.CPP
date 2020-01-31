// *****************
// * Event Manager *
// *****************

#include <common\common.h>
#include <common\old\coord.h>
#include <common\old\mouse.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\event.h>
#include <seashell\window.h>

// **********************
// * Screen Saver Class *
// **********************

Boolean C_Saver::Active(void) {
	timer.Reset(delay);
	if (!on)
		return (False);
	VirtualDispose(vnum);
	ScreenInvertActivate(False);
	ScreenSynch();
	on = False;
	return (True);
	}


void C_Saver::Idle(void) {
	Rect	r;

	if (on) {
		if (timer.Elapsed()) {
			ScreenInvert();
			timer.Reset(rate);
			ScreenSynch();
			}
		}
	else if (timer.Elapsed()) {
		vnum = VirtualGetNew(_EventWINDOW, &r);
		ScreenInvertActivate(True);
		ScreenSynch();
		on = True;
		timer.Reset(rate);
		}
	}

