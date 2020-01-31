#ifndef _KEYBOARD_H
#define _KEYBOARD_H

// *********************
// * Keyboard Routines *
// *********************

// This module provides a low level interface with the keyboard.  Program
// execution will not be terminated by a <Ctrl><Break> key sequence.

// values for special keys on IBM PC

const Word	kXF		= 0x100;	// extended key flag

const Word	kK_F1		= ( 59 | kXF);	// function keys
const Word	kK_F2		= ( 60 | kXF);
const Word	kK_F3		= ( 61 | kXF);
const Word	kK_F4		= ( 62 | kXF);
const Word	kK_F5		= ( 63 | kXF);
const Word	kK_F6		= ( 64 | kXF);
const Word	kK_F7		= ( 65 | kXF);
const Word	kK_F8		= ( 66 | kXF);
const Word	kK_F9		= ( 67 | kXF);
const Word	kK_F10		= ( 68 | kXF);
const Word	kK_F11		= (133 | kXF);
const Word	kK_F12		= (134 | kXF);

const Word	kK_SF1		= ( 84 | kXF);	// shifted function keys
const Word	kK_SF2		= ( 85 | kXF);
const Word	kK_SF3		= ( 86 | kXF);
const Word	kK_SF4		= ( 87 | kXF);
const Word	kK_SF5		= ( 88 | kXF);
const Word	kK_SF6		= ( 89 | kXF);
const Word	kK_SF7		= ( 90 | kXF);
const Word	kK_SF8		= ( 91 | kXF);
const Word	kK_SF9		= ( 92 | kXF);
const Word	kK_SF10		= ( 93 | kXF);
const Word	kK_SF11		= (135 | kXF);
const Word	kK_SF12		= (136 | kXF);

const Word	kK_CF1		= ( 94 | kXF);	// control function keys
const Word	kK_CF2		= ( 95 | kXF);
const Word	kK_CF3		= ( 96 | kXF);
const Word	kK_CF4		= ( 97 | kXF);
const Word	kK_CF5		= ( 98 | kXF);
const Word	kK_CF6		= ( 99 | kXF);
const Word	kK_CF7		= (100 | kXF);
const Word	kK_CF8		= (101 | kXF);
const Word	kK_CF9		= (102 | kXF);
const Word	kK_CF10		= (103 | kXF);
const Word	kK_CF11		= (137 | kXF);
const Word	kK_CF12		= (138 | kXF);

const Word	kK_AF1		= (104 | kXF);	// alternate function keys
const Word	kK_AF2		= (105 | kXF);
const Word	kK_AF3		= (106 | kXF);
const Word	kK_AF4		= (107 | kXF);
const Word	kK_AF5		= (108 | kXF);
const Word	kK_AF6		= (109 | kXF);
const Word	kK_AF7		= (110 | kXF);
const Word	kK_AF8		= (111 | kXF);
const Word	kK_AF9		= (112 | kXF);
const Word	kK_AF10		= (113 | kXF);
const Word	kK_AF11		= (139 | kXF);
const Word	kK_AF12		= (140 | kXF);

const Word	kK_HOME		= ( 71 | kXF);	// cursor keypad (NumLock off; not shifted)
const Word	kK_END		= ( 79 | kXF);
const Word	kK_PGUP		= ( 73 | kXF);
const Word	kK_PGDN		= ( 81 | kXF);
const Word	kK_LEFT		= ( 75 | kXF);
const Word	kK_RIGHT	= ( 77 | kXF);
const Word	kK_UP		= ( 72 | kXF);
const Word	kK_DOWN		= ( 80 | kXF);

const Word	kK_CHOME	= (119 | kXF);	// control cursor keypad
const Word	kK_CEND		= (117 | kXF);
const Word	kK_CPGUP	= (132 | kXF);
const Word	kK_CPGDN	= (118 | kXF);
const Word	kK_CLEFT	= (115 | kXF);
const Word	kK_CRIGHT	= (116 | kXF);

const Word	kK_CTRLA	=  1;		// standard control keys
const Word	kK_CTRLB	=  2;
const Word	kK_CTRLC	=  3;
const Word	kK_CTRLD	=  4;
const Word	kK_CTRLE	=  5;
const Word	kK_CTRLF	=  6;
const Word	kK_CTRLG	=  7;
const Word	kK_CTRLH	=  8;
const Word	kK_CTRLI	=  9;
const Word	kK_CTRLJ	= 10;
const Word	kK_CTRLK	= 11;
const Word	kK_CTRLL	= 12;
const Word	kK_CTRLM	= 13;
const Word	kK_CTRLN	= 14;
const Word	kK_CTRLO	= 15;
const Word	kK_CTRLP	= 16;
const Word	kK_CTRLQ	= 17;
const Word	kK_CTRLR	= 18;
const Word	kK_CTRLS	= 19;
const Word	kK_CTRLT	= 20;
const Word	kK_CTRLU	= 21;
const Word	kK_CTRLV	= 22;
const Word	kK_CTRLW	= 23;
const Word	kK_CTRLX	= 24;
const Word	kK_CTRLY	= 25;
const Word	kK_CTRLZ	= 26;

const Word	kK_ALTA		= ( 30 | kXF);	// alternate keys
const Word	kK_ALTB		= ( 48 | kXF);
const Word	kK_ALTC		= ( 46 | kXF);
const Word	kK_ALTD		= ( 32 | kXF);
const Word	kK_ALTE		= ( 18 | kXF);
const Word	kK_ALTF		= ( 33 | kXF);
const Word	kK_ALTG		= ( 34 | kXF);
const Word	kK_ALTH		= ( 35 | kXF);
const Word	kK_ALTI		= ( 23 | kXF);
const Word	kK_ALTJ		= ( 36 | kXF);
const Word	kK_ALTK		= ( 37 | kXF);
const Word	kK_ALTL		= ( 38 | kXF);
const Word	kK_ALTM		= ( 50 | kXF);
const Word	kK_ALTN		= ( 49 | kXF);
const Word	kK_ALTO		= ( 24 | kXF);
const Word	kK_ALTP		= ( 25 | kXF);
const Word	kK_ALTQ		= ( 16 | kXF);
const Word	kK_ALTR		= ( 19 | kXF);
const Word	kK_ALTS		= ( 31 | kXF);
const Word	kK_ALTT		= ( 20 | kXF);
const Word	kK_ALTU		= ( 22 | kXF);
const Word	kK_ALTV		= ( 47 | kXF);
const Word	kK_ALTW		= ( 17 | kXF);
const Word	kK_ALTX		= ( 45 | kXF);
const Word	kK_ALTY		= ( 21 | kXF);
const Word	kK_ALTZ		= ( 44 | kXF);

const Word	kK_ALT1		= (120 | kXF);	// additional alternate key combinations
const Word	kK_ALT2		= (121 | kXF);
const Word	kK_ALT3		= (122 | kXF);
const Word	kK_ALT4		= (123 | kXF);
const Word	kK_ALT5		= (124 | kXF);
const Word	kK_ALT6		= (125 | kXF);
const Word	kK_ALT7		= (126 | kXF);
const Word	kK_ALT8		= (127 | kXF);
const Word	kK_ALT9		= (128 | kXF);
const Word	kK_ALT0		= (129 | kXF);
const Word	kK_ALTDASH	= (130 | kXF);
const Word	kK_ALTEQU	= (131 | kXF);

const Word	kK_NULL		= (  3 | kXF);

const Word	kK_CTRL_PRTSC	= (114 | kXF);	// printer echoing toggle

const Word	kK_SPACE	= 32;		// miscellaneous special keys

const Word	kK_BACKTAB	= ( 15 | kXF);
const Word	kK_INS		= ( 82 | kXF);
const Word	kK_DEL		= ( 83 | kXF);

const Word	kK_CTRLBRK	= (192 | kXF);
const Word	kK_TAB		= (193 | kXF);
const Word	kK_BACKSPACE	= (194 | kXF);
const Word	kK_RETURN	= (195 | kXF);
const Word	kK_ESCAPE	= (196 | kXF);
const Word	kK_HELP		= (197 | kXF);

const Word	kKS_RSHIFT	= 0x0001;	// keyboard toggles and shift keys
const Word	kKS_LSHIFT	= 0x0002;
const Word	kKS_CTRL	= 0x0004;
const Word	kKS_ALT		= 0x0008;
const Word	kKS_SCROLL	= 0x0010;
const Word	kKS_NUM		= 0x0020;
const Word	kKS_CAPS	= 0x0040;
const Word	kKS_INS		= 0x0080;
const Word	kKS_EVERY	= 0x00FF;

Boolean	KeyboardAvail(void);
Word	KeyboardRead(void);
Word	KeyboardStatus(Word mask);

#endif
