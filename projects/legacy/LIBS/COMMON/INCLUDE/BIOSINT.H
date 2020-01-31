#ifndef _BIOSINT_H
#define _BIOSINT_H

// ***************************
// * BIOS Interrupt Services *
// ***************************

#define KEYBOARD_IO		0x16
#define CTRL_BREAK		0x1B
#define CTRL_C			0x23
#define MOUSE_SERVICE		0x33

// *********************
// * Keyboard Services *
// *********************

#define KEYBOARD_READ		0x00
#define KEYBOARD_READY		0x01
#define KEYBOARD_STATUS		0x02

// ******************
// * Mouse Services *
// ******************

#define MOUSE_RESET		0x00
#define MOUSE_SHOW_CURSOR	0x01
#define MOUSE_HIDE_CURSOR	0x02
#define MOUSE_STATUS		0x03

#endif
