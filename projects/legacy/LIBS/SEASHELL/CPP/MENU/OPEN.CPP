#include "menu\_menu.h"

#include <seashell\window.h>

short MenuOpen(short top, short lef, short num, short width) {
	Rect	r;

	r.Set(top, lef, top+num+1, lef+width+1);
	return (VirtualNew(num, width, WMENU, r, NULL, menuLow));
	}
