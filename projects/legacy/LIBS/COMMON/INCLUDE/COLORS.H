#ifndef _COLORS_H
#define _COLORS_H

// *********************
// * Color Definitions *
// *********************

// attribute modifiers
#define BRIGHT	8
#define BLINK	128

// primary video attributes
#define BLU	1
#define GRN	2
#define RED	4

// composite video attributes
#define BLK	0
#define CYN	(BLU | GRN)		// 3
#define MAG	(BLU | RED)		// 5
#define BRN	(GRN | RED)		// 6
#define WHT	(BLU | GRN | RED)	// 7
#define GRAY	(BLK | BRIGHT)
#define LBLU	(BLU | BRIGHT)
#define LGRN	(GRN | BRIGHT)
#define LCYAN	(CYN | BRIGHT)
#define LRED	(RED | BRIGHT)
#define LMAG	(MAG | BRIGHT)
#define YEL	(BRN | BRIGHT)
#define BWHT	(WHT | BRIGHT)

#define NORMAL	WHT
#define REVERSE	112

#endif
