// ***************
// * Mouse Class *
// ***************

#include <dos.h>

#include <common\common.h>
#include <common\biosint.h>
#include <common\old\mouse.h>

#define INSTRUCTION_IRET	0xCF

#define MOUSE_BUTTON_NONE	0x00
#define MOUSE_BUTTON_TWO	0x03

typedef void interrupt (*IntHandlerFunc)(...);

// ********************
// * Static Variables *
// ********************

// Only allow one instance of C_Mouse to exist.

Boolean	C_Mouse::once = False;

// **********************************************
// * C_Mouse Class Constructors and Destructors *
// **********************************************

C_Mouse::C_Mouse(void) {
	IntHandlerFunc	intHandler;
	Byte		firstByte;
	long		vector;
	int		ax;

	active = False;
	if (once)				// allow only one instance
		return;
	intHandler = getvect(MOUSE_SERVICE);
	firstByte = *(Byte far *) intHandler;
	vector = (long) intHandler;
	if ((vector == 0) || (firstByte == INSTRUCTION_IRET))
		return;
	_AX = MOUSE_RESET;
	geninterrupt(MOUSE_SERVICE);
	ax = _AX;
	once = active = (ax == -1) ? True : False;
	}


C_Mouse::~C_Mouse(void) {
	if (!active)				// class is not active
		return;
	_AX = MOUSE_HIDE_CURSOR;
	geninterrupt(MOUSE_SERVICE);
	_AX = MOUSE_RESET;
	geninterrupt(MOUSE_SERVICE);
	once = active = False;
	}


Word C_Mouse::Button(void) {
	Word	bx;

	if (active) {
		_AX = MOUSE_STATUS;
		geninterrupt(MOUSE_SERVICE);
		bx = _BX;
		}
	else
		bx = MOUSE_BUTTON_NONE;
	return (bx & MOUSE_BUTTON_TWO);
	}


void C_Mouse::Hide(void) {
	if (!active)
		return;
	_AX = MOUSE_HIDE_CURSOR;
	geninterrupt(MOUSE_SERVICE);
	}


void C_Mouse::Location(Point *loc) {
	if (active) {
		_AX = MOUSE_STATUS;
		geninterrupt(MOUSE_SERVICE);
		loc->Set(_DX, _CX);
		}
	else
		loc->Set(0, 0);
	}


void C_Mouse::Show(void) {
	if (!active)
		return;
	_AX = MOUSE_SHOW_CURSOR;
	geninterrupt(MOUSE_SERVICE);
	}
