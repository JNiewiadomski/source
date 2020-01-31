#ifndef _MOUSE_H
#define _MOUSE_H

#include <common\old\coord.h>

// ******************
// * Mouse Routines *
// ******************

class C_Mouse {
	static Boolean	once;

	Boolean	active;			// is mouse driver active?
public:
	C_Mouse(void);
	~C_Mouse(void);

	Boolean	Active(void) { return (active); }
	Word	Button(void);
	void	Hide(void);
	void	Location(Point *loc);
	void	Show(void);
	};

#endif
