#ifndef _ASCII_H
#define _ASCII_H

// *******************************
// * ASCII Character Definitions *
// *******************************

#define A_NUL	0		//	null \0 delimeter
#define A_SOH	1		// ^A -	start of heading
#define A_CTRLA	A_SOH
#define A_STX	2		// ^B -	start of text
#define A_CTRLB	A_STX
#define A_ETX	3		// ^C -	end of text
#define A_CTRLC	A_ETX
#define A_EOT	4		// ^D -	end of transmission
#define A_CTRLD	A_EOT
#define A_ENQ	5		// ^E -	inquiry
#define A_CTRLE	A_ENQ
#define A_ACK	6		// ^F -	affirm acknowledge
#define A_CTRLF	A_ACK
#define A_BEL	7		// ^G -	audible bell
#define A_CTRLG	A_BEL
#define A_BS	8		// ^H -	backspace
#define A_CTRLH	A_BS
#define A_HT	9		// ^I -	horizontal tab
#define A_CTRLI	A_HT
#define A_LF	10		// ^J -	line feed
#define A_CTRLJ	A_LF
#define A_VT	11		// ^K -	vertical tab
#define A_CTRLK	A_VT
#define A_FF	12		// ^L -	form feed
#define A_CTRLL	A_FF
#define A_CR	13		// ^M -	carriage return
#define A_CTRLM	A_CR
#define A_SO	14		// ^N -	shift out
#define A_CTRLN	A_SO
#define A_SI	15		// ^O -	shift in
#define A_CTRLO	A_SI
#define A_DLE	16		// ^P -	data link escape
#define A_CTRLP	A_DLE
#define A_DC1	17		// ^Q -	device control 1
#define A_CTRLQ	A_DC1
#define A_DC2	18		// ^R -	device control 2
#define A_CTRLR	A_DC2
#define A_DC3	19		// ^S -	device control 3
#define A_CTRLS	A_DC3
#define A_DC4	20		// ^T -	device control 4
#define A_CTRLT	A_DC4
#define A_NAK	21		// ^U -	neg acknowledge
#define A_CTRLU	A_NAK
#define A_SYN	22		// ^V -	synchronous idle
#define A_CTRLV	A_SYN
#define A_ETB	23		// ^W -	end of transmission
#define A_CTRLW	A_ETB
#define A_CAN	24		// ^X -	cancel
#define A_CTRLX	A_CAN
#define A_EM	25		// ^Y -	end of medium
#define A_CTRLY	A_EM
#define A_SUB	26		// ^Z -	substitute
#define A_CTRLZ	A_SUB
#define A_ESC	27		//	escape
#define A_FS	28		//	file separator
#define A_GS	29		//	group separator
#define A_RS	30		//	record separator
#define A_US	31		//	unlinked separator
#define A_SP	32		//	space
#define A_DEL	127		//	delete

#endif
