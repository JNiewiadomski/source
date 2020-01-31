// *********************
// * Keyboard Routines *
// *********************

// This module provides a low level interface with the keyboard.

#include <dos.h>

#include <common\common.h>
#include <common\biosint.h>
#include <common\dosint.h>
#include <common\old\keyboard.h>

// *************
// * Key Codes *
// *************

const Word	kSCAN_ESCAPE	= 0x011B;	// scan code for escape
const Word	kSCAN_BACKSPACE	= 0x0E08;	// scan code for backspace
const Word	kSCAN_TAB	= 0x0F09;	// scan code for tab
const Word	kSCAN_RETURN	= 0x1C0D;	// scan code for carriage return
const Word	kSCAN_HELP	= 0x2308;	// scan code for ctrl-h / help key

// *********************
// * Keyboard Routines *
// *********************

// Determines if a character is ready for input.

Boolean KeyboardAvail(void) {
	_AH = KEYBOARD_READY;		// check if key ready to be read
	geninterrupt(KEYBOARD_IO);	// transfer to ROM driver
	asm jz KeyboardAvailFalse
	return (True);
KeyboardAvailFalse:
	return (False);
	}


// Set cook output mode for standard output device.  With all MS-DOS console
// drivers, this slows down output by enabling checking for Ctrl-C, Ctrl-S
// and Ctrl-P.

void KeyboardCookMode(void) {
	_AH = IOCTL;			// read "device information" using
	_AL = 0x00;			// function 44h, subfunction 00h
	_BX = 0;			// handle for standard input
	geninterrupt(BDOS_REQ);		// places device information in DX
	_DH = 0;			// zero out upper byte of DX
	_DL &= 0xDF;			// turn off "raw" bit (#5)
	_AH = IOCTL;			// write back "device information"
	_AL = 0x01;			// using function 44h, subfunction 01h
	_BX = 0;			// handle for standard input
	geninterrupt(BDOS_REQ);
	}


// Set raw output mode for standard output device.  With all MS-DOS console
// drivers, this speeds up output by disabling checking for Ctrl-C, Ctrl-S
// and Ctrl-P.

void KeyboardRawMode(void) {
	_AH = IOCTL;			// read "device information" using
	_AL = 0x00;			// function 44h, subfunction 00h
	_BX = 0;			// handle for standard input
	geninterrupt(BDOS_REQ);		// place device information in DX
	_DH = 0;			// zero out upper byte of DX
	_DL |= 0x20;			// turn on "raw" bit (#5)
	_AH = IOCTL;			// write back "device information"
	_AL = 0x01;			// using function 44h, subfunction 01h
	_BX = 0;			// handle for standard input
	geninterrupt(BDOS_REQ);
	}


// *********************
// * Keyboard Routines *
// *********************

// Reads a character from the keyboard, returning a short value that
// represents the character that was pressed.  The values are defined in
// keyboard.h and begin with "K_".  The BIOS keyboard routine returns the
// ASCII characeter and the keyboard scan code in the format:
// _______________________  _______________________
// 15 14 13 12 11 10  9  8   7  6  5  4  3  2  1  0
//    Keyboard Scan Code         ASCII Character

Word KeyboardRead(void) {
	Word	ch;

	_AH = KEYBOARD_READ;		// function 0=read character
	geninterrupt(KEYBOARD_IO);	// transfer to ROM driver
	if (_AX == 0)			// is it a Ctrl-Break?
		return (kK_CTRLBRK);
	if (_AL == 0)			// is it a special character?
		return (kXF | (_AX >> 8));
	ch = _AX;
	switch (ch) {
		case kSCAN_ESCAPE:
			return (kK_ESCAPE);
		case kSCAN_HELP:
			return (kK_HELP);
		case kSCAN_BACKSPACE:
			return (kK_BACKSPACE);
		case kSCAN_TAB:
			return (kK_TAB);
		case kSCAN_RETURN:
			return (kK_RETURN);
		}
	return (ch & 0x00FF);		// get rid of scan code
	}


// *********************
// * Keyboard Routines *
// *********************

// Returns the BIOS flags byte that describes the state of the various
// keyboard toggles and shift keys.
//
// Returned flags byte:
// 	bit 7  =  insert on
// 	bit 6  =  caps lock on
// 	bit 5  =  num lock on
// 	bit 4  =  scroll lock on
// 	bit 3  =  alt key down
// 	bit 2  =  ctrl key down
// 	bit 1  =  left-shift key down
// 	bit 0  =  right-shift key down

Word KeyboardStatus(Word mask) {
	_AH = KEYBOARD_STATUS;		// function 2=get flags byte
	geninterrupt(KEYBOARD_IO);	// transfer to ROM driver
	_AX &= mask;			// mask off any undesired info
	return (_AX);
	}
