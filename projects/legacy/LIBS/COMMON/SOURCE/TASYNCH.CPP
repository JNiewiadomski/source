// *************************************
// * Asynchronous Communications Class *
// *************************************

// The Asynchronous Communications class provides interrupt driven serial
// communications.

#include <dos.h>

#include <common\common.h>
#include <common\tasynch.h>

// **************
// * References *
// **************

// Crafting C Tools for the IBM PCs; Joe Campbell; Prentice Hall; 1986
// Structured Programming; Jeff Duntemann; Dr. Dobb's Journal; #176; May 1991
// Structured Programming; Jeff Duntemann; Dr. Dobb's Journal; #177; June 1991
// Structured Programming; Jeff Duntemann; Dr. Dobb's Journal; #178; July 1991
// Structured Programming; Jeff Duntemann; Dr. Dobb's Journal; #179; August 1991
// Structured Programming; Jeff Duntemann; Dr. Dobb's Journal; #180; September 1991
// The Waite Group's MS-DOS Developer's Guide, Second Edition; Howard W. Sams & Company; 1988
// Data Communications Local Area Networks UARTs Handbook; National Semiconductor; 1988 Edition
// PC UART Device Driver; Steven Rothkin; The C Users Journal; December 1991

// ******************
// * UART Constants *
// ******************

// The base port address of the UART for each of the serial devices is
// maintained in the ROM BIOS communications area beginning at UART_PTR.

const Word	kBIOS_Page=0x0040;	// beginning of UART pointer table

// The UART has 10 registers accessible through 7 port addresses.  Here are
// their addresses relative to base.  Note that the Baud rate registers (DLL)
// and (DLM) are active only when the Divisor Latch Access Bit (DLAB) is set.

const Word	kUART_RBR=0;		// (R)   receive buffer register
const Word	kUART_THR=0;		// (W)   transmit holding register
const Word	kUART_DLL=0;		// (R/W) divisor latch, low byte
const Word	kUART_IER=1;		// (R/W) interrupt enable register
const Word	kUART_DLM=1;		// (R/W) divisor latch, high byte
const Word	kUART_IIR=2;		// (R)   interrupt ID register
const Word	kUART_FCR=2;		// (W)   FIFO control register (PS/2 only)
const Word	kUART_LCR=3;		// (R/W) line control register
const Word	kUART_MCR=4;		// (R/W) modem control register
const Word	kUART_LSR=5;		// (R)   line status register
const Word	kUART_MSR=6;		// (R/W) modem status register
const Word	kUART_SCR=7;		// (R/W) scratch register (AT/PS2 only)

// Interrupt Enable Register

const Byte	kIER_RDA=0x01;		// received data available
const Byte	kIER_THRE=0x02;		// transmitter holding register empty
const Byte	kIER_RLS=0x04;		// receiver line status
const Byte	kIER_MS=0x08;		// modem status

const Byte	kIER_ALL=kIER_RDA | kIER_THRE | kIER_RLS | kIER_MS;
const Byte	kIER_NONE=0x00;

// Interrupt Identification Register values

const Byte	kIIR_MS=0x00;		// modem status
const Byte	kIIR_THRE=0x02;		// transmitter holding register empty
const Byte	kIIR_RDA=0x04;		// received data available
const Byte	kIIR_RLS=0x06;		// receiver line status

// Line Control Register bit masks

const Byte	kLCR_WLS0=0x01;		// word length select bit 0
const Byte	kLCR_WLS1=0x02;		// word length select bit 1
const Byte	kLCR_STB=0x04;		// number of stop bits
const Byte	kLCR_PEN=0x08;		// parity enable (0=off, 1=on)
const Byte	kLCR_EPS=0x10;		// even parity select (0=odd, 1=even)
const Byte	kLCR_STP=0x20;		// stick parity
const Byte	kLCR_BRK=0x40;		// set break
const Byte	kLCR_DLAB=0x80;		// divisor latch access bit

// Parity

const Byte	kLCR_NONE=0x00;
const Byte	kLCR_ODD=0x08;
const Byte	kLCR_EVEN=0x18;
const Byte	kLCR_MARK=0x28;
const Byte	kLCR_SPACE=0x38;

// Data Bits

const Byte	kLCR_5=0x00;
const Byte	kLCR_6=0x01;
const Byte	kLCR_7=0x02;
const Byte	kLCR_8=0x03;

// Stop Bits

const Byte	kLCR_1=0x00;
const Byte	kLCR_2=0x04;

// Modem Control Register bit masks

const Byte	kMCR_DTR=0x01;		// data terminal ready
const Byte	kMCR_RTS=0x02;		// request to send
const Byte	kMCR_OUT1=0x04;		// out 1
const Byte	kMCR_OUT2=0x08;		// out 2; must be set for serial to send interrupts to system
const Byte	kMCR_LOOP=0x10;		// loop

// Line Status Register bit masks

const Byte	kLSR_DR=0x01;		// data ready
const Byte	kLSR_OE=0x02;		// overrun error
const Byte	kLSR_PE=0x04;		// parity error
const Byte	kLSR_FE=0x08;		// framing error
const Byte	kLSR_BI=0x10;		// break interrupt
const Byte	kLSR_THRE=0x20;		// transmitter holding register empty
const Byte	kLSR_TSRE=0x40;		// transmitter shift register empty

// Modem Status Register bit masks

const Byte	kMSR_DCTS=0x01;		// delta clear to send
const Byte	kMSR_DDSR=0x02;		// delta data set ready
const Byte	kMSR_TERI=0x04;		// trailing edge ring indicator
const Byte	kMSR_DDCD=0x08;		// delta data carrier detect
const Byte	kMSR_CTS=0x10;		// clear to send
const Byte	kMSR_DSR=0x20;		// data set ready
const Byte	kMSR_RI=0x40;		// ring indicator
const Byte	kMSR_DCD=0x80;		// data carrier detect

// *****************
// * PIC Constants *
// *****************

// Port addresses of the 8259 Programmable Interrupt Controller (PIC).

const Byte	kPIC_ICR=0x20;		// interrupt control register port
const Byte	kPIC_IMR=0x21;		// interrupt mask register port

// An end of interrupt needs to be sent to the Control Port of the 8259 when
// a hardware interrupt ends.

const Byte	kPIC_EOI=0x20;		// end of interrupt

// The IMR tells the PIC to service an interrupt only if it is not masked
// (false).

const Byte	kPIC_IRQ3=0xF7;		// com2
const Byte	kPIC_IRQ4=0xEF;		// com1

// ************************************
// * Interrupt Vector Table Constants *
// ************************************

const Byte	kIVT_COM1=0x0C;		// asynchronous communications port controller 0
const Byte	kIVT_COM2=0x0B;		// asynchronous communications port controller 1


// ********************
// * Static Variables *
// ********************

TAsynch	*TAsynch::fAS_Table[] = { 0, 0 };


// *****************************
// * Baud Rate / Divisor Table *
// *****************************

static struct {
	DWord	baud;
	Word	divisor;
	} baud_div[] = {
		{ 50L,		2304	},
		{ 75L,		1536	},
		{ 110L,		1047	},
		{ 134L,		857	},
		{ 150L,		768	},
		{ 300L,		384	},
		{ 600L,		192	},
		{ 1200L,	96	},
		{ 1800L,	64	},
		{ 2000L,	58	},
		{ 2400L,	48	},
		{ 3600L,	32	},
		{ 4800L,	24	},
		{ 7200L,	16	},
		{ 9600L,	12	},
		{ 19200L,	6	},
		{ 38400L,	3	},
		{ 57600L,	2	},
		{ 115200L,	1	},
		{ 0L,		0	}
		};


// **********
// * Macros *
// **********

// Chew up CPU cycles while I/O port catches up

#define IO_DELAY(delay_loops) {						\
	for (fDelay_Char=0; fDelay_Char < delay_loops; fDelay_Char++)	\
		fShift_Char &= fDelay_Char;				\
	}


// ******************************
// * Inline Function Prototypes *
// ******************************

inline Word	TAsynchGetBase(Word port);


// ********************
// * Inline Functions *
// ********************

// Returns base address of com port.

inline Word TAsynchGetBase(Word port) {
	DWord	*rs232_adr;

	rs232_adr = (DWord *) MK_FP(kBIOS_Page, (2 * port));
	return (*rs232_adr);
	}


inline Boolean TAsynch::Transmit(Byte ch) {
	Boolean	status;

	disable();
	if (fXmitting)
		status = fQ_tx.Put(ch);
	else {
		outportb(fBase + kUART_THR, ch);	// startup transmit interrupt
		status = fXmitting = True;
		}
	enable();
	return (status);
	}


// ********************
// * Static Functions *
// ********************

// COM1 Serial interrupt handler.

void interrupt far TAsynch::Handler_1(...) {
	fAS_Table[kAS_COM1]->Handler();
	}


// COM2 Serial interrupt handler.

void interrupt far TAsynch::Handler_2(...) {
	fAS_Table[kAS_COM2]->Handler();
	}


// ********************
// * Support Routines *
// ********************

// TAsynchExist returns true if a National Semiconductor UART (one of 8250B,
// 16450, 16550 or 16550A) is present at the port specified.

Boolean TAsynchExist(Word port) {
	return (TAsynchGetBase(port) == 0 ? False : True);
	}


// ***********************
// * TAS_Q Class Methods *
// ***********************

// If queue is empty, return false.  Otherwise copy next data element in
// queue to memory that ch is pointing at and return true.  The character is
// removed from the queue.

Boolean TAS_Q::Get(Byte *ch) {
	if (fCount == 0)		// is queue empty?
		return (False);
	*ch = fBuff[fFront++];		// get data from queue
	fCount--;
	return (True);
	}


// If queue is empty, return false.  Otherwise copy next data element in
// queue to memory that ch is pointing at and return true.  The character is
// not removed from the queue.

Boolean TAS_Q::Look(Byte *ch) {
	if (fCount == 0)		// is queue empty?
		return (False);
	*ch = fBuff[fFront];		// get data from queue
	return (True);
	}


// If queue is full, return false.  Otherwise insert data into queue and
// return true.

Boolean TAS_Q::Put(Byte ch) {
	if (fCount == sizeof(fBuff))	// is queue full?
		return (False);
	fBuff[fRear++] = ch;		// save character in queue
	fCount++;
	return (True);
	}


// *************************
// * TAsynch Class Methods *
// *************************

// Break -- transmits BREAK on the serial line.

void TAsynch::Break(void) {
	if (!fOpen)
		return;
	Put('\0');
	WaitTX();
	outportb(fBase + kUART_LCR, (inportb(fBase + kUART_LCR) & ~kLSR_TSRE) | kLSR_TSRE);
	WaitTXEmpty();
	Put('\0');
	WaitTXEmpty();
	outportb(fBase + kUART_LCR, inportb(fBase + kUART_LCR) & ~kLSR_TSRE);
	}


// Close the current serial stream.

void TAsynch::Close(void) {
	Byte	cur_imr;

	if (!fOpen)
		return;
	WaitTXEmpty();				// make sure we're done transmitting
	disable();
	cur_imr = inportb(kPIC_IMR);
	IO_DELAY(3);				// delay for port to catch up
	outportb(kPIC_IMR, cur_imr | ~fInt_Mask);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_IER, kIER_NONE);	// disable serial interrupts
	enable();
	StateRestore();
	fAS_Table[fPort] = 0;			// clear jump table pointer
	fOpen = False;
	}


// Returns the number of characters received.

Byte TAsynch::CountRx(void) {
	Byte	count;

	disable();
	count = fQ_rx.Count();
	enable();
	return (count);
	}


// Returns the number of characters left to transmit.

Byte TAsynch::CountTx(void) {
	Byte	count;

	disable();
	count = fQ_tx.Count();
	enable();
	return (count);
	}


Boolean TAsynch::FormatGet(DWord *baud, Byte *parity, Byte *data, Byte *stop) {
	Byte	format;
	Word	curBaud;
	short	i=0;

	if (!fOpen)
		return (False);
	disable();
	format = inportb(fBase + kUART_LCR);
	outportb(fBase + kUART_LCR, format | kLCR_DLAB);
	IO_DELAY(3);				// delay for port to catch up
	curBaud  = (inportb(fBase + kUART_DLM) << 8) | inportb(fBase + kUART_DLL);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_LCR, format & ~kLCR_DLAB);
	enable();
	for (i=0; baud_div[i].divisor>0 && baud_div[i].divisor!=curBaud; i++)
		;
	*baud = baud_div[i].baud;
	switch (format & (kLCR_PEN | kLCR_EPS | kLCR_STP)) {
		case kLCR_ODD:
			*parity = kAS_Odd;
			break;
		case kLCR_EVEN:
			*parity = kAS_Even;
			break;
		case kLCR_MARK:
			*parity = kAS_Mark;
			break;
		case kLCR_SPACE:
			*parity = kAS_Space;
			break;
		default:
			*parity = kAS_None;
			break;
		}
	switch (format & (kLCR_WLS0 | kLCR_WLS1)) {
		case kLCR_5:
			*data = 5;
			break;
		case kLCR_6:
			*data = 6;
			break;
		case kLCR_7:
			*data = 7;
			break;
		default:
			*data = 8;
			break;
		}
	switch (format & (kLCR_STB)) {
		case kLCR_2:
			*stop = 2;
			break;
		default:
			*stop = 1;
			break;
		}
	return (True);
	}


Boolean TAsynch::FormatSet(DWord baud, Byte parity, Byte data, Byte stop) {
	Word	curBaud;
	Byte	format=0;
	short	i;

	if (!fOpen)
		return (False);
	for (i=0; baud_div[i].baud>0 && baud_div[i].baud!=baud; i++)
		;
	curBaud = baud_div[i].divisor;
	if (curBaud == 0)
		return (False);
	switch (parity) {
		case kAS_None:
			break;
		case kAS_Odd:
			format |= kLCR_ODD;
			break;
		case kAS_Even:
			format |= kLCR_EVEN;
			break;
		case kAS_Mark:
			format |= kLCR_MARK;
			break;
		case kAS_Space:
			format |= kLCR_SPACE;
			break;
		default:
			return (False);
		}
	switch (data) {
		case 5:
			format |= kLCR_5;
			break;
		case 6:
			format |= kLCR_6;
			break;
		case 7:
			format |= kLCR_7;
			break;
		case 8:
			format |= kLCR_8;
			break;
		defualt:
			return (False);
		}
	switch (stop) {
		case 1:
			format |= kLCR_1;
			break;
		case 2:
			format |= kLCR_2;
			break;
		default:
			return (False);
		}
	disable();
	outportb(fBase + kUART_LCR, format | kLCR_DLAB);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_DLM, curBaud >> 8);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_DLL, curBaud & 0x00FF);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_LCR, format & ~kLCR_DLAB);
	enable();
	return (True);
	}


// Returns a character from the receive queue.  

Boolean TAsynch::Get(Byte *ch, Boolean wait) {
	Boolean	status;

	if (!fOpen)
		return (False);
	do {
		disable();
		status = fQ_rx.Get(ch);
		enable();
		} while (wait && !status);
	return (status);
	}


// GetUART returns the type of UART chip that is present at the selected
// port.  The types of UARTS detected are as follows:
//
// kAS_8250   : generally (but not always!) present in PC or XT
// kAS_16450  : generally present in AT-class machines
// kAS_16550  : in PS/2 models 50/60/early 80.  FIFOs don't work!
// kAS_16550A : in later PS/2's.  FIFOs fully operative.

Word TAsynch::GetUART(void) {
	Byte	holder;

	if (!fOpen)
		return (kAS_Unknown);
	outportb(fBase+kUART_SCR, 0xAA);	// write pattern to scratch register
	if (inportb(fBase+kUART_SCR) != 0xAA)
		return (kAS_8250);		// UART without scratch register is 8250
	outportb(fBase+kUART_FCR, 0x01);	// attempt to enable FIFOs
	holder = inportb(fBase+kUART_IIR) & 0xC0;
	outportb(fBase+kUART_FCR, 0x00);	// don't leave FIFOs enabled!
	switch (holder) {
		case 0xC0: return (kAS_16550A);
		case 0x80: return (kAS_16550);
		case 0x00:
 		default:   return (kAS_16450);
		}
	}


// Serial interrupt handler.

void TAsynch::Handler(void) {
	register Byte	int_id, ch;

	enable();			// enable interrupts immediately
	while (True) {
		// Read the interrupt identification register, IIR.
		int_id = inportb(fBase + kUART_IIR);
		if (int_id & 0x0001) {
			// If bit 0 is 1, then no interrupts pending.  Send
			// an end of interrupt signal to the 8259A
			// Programmable Interrupt Controller and then return.
			outportb(kPIC_ICR, kPIC_EOI);
			return;
			}
		// Process interrupt according to ID.  The following list is
		// in increasing order of priority.
		switch(int_id) {
			case kIIR_MS:		// read modem status
				disable();
				fModem_Status = inportb(fBase + kUART_MSR);
				enable();
				break;
			case kIIR_THRE:
				disable();
				if (fQ_tx.Get(&ch))
					outportb(fBase + kUART_THR, ch);
				else
					fXmitting = False;
				enable();
				break;
			case kIIR_RDA:
				disable();
				ch = inportb(fBase + kUART_RBR);
				fQ_rx.Put(ch);
				enable();
				break;
			case kIIR_RLS:		// read line status
				disable();
				fLine_Status = inportb(fBase + kUART_LSR);
				enable();
				break;
			}
		}
	}


// Open up a serial port.

Word TAsynch::Open(Word port, DWord baud, Byte parity, Byte data, Byte stop) {
	Byte	cur_imr;

	if (fOpen)
		return (kAS_Err_OK);
	if (port != kAS_COM1 && port != kAS_COM2)
		return (kAS_Err_Param);
	if (!TAsynchExist(port))
		return (kAS_Err_Exist);
	fPort = port;
	if (fAS_Table[fPort] != 0)
		return (kAS_Err_Open);

	// Set up masks for 8259A PIC.  To enable interrupt from the port
	// this mask is ANDed with the mask register as 21h.  To disable, OR
	// the mask with the mask register.  The interrupt number is
	// 8 + the IRQ level of the interrupt.  Com port 1 has IRQ 4, port 2
	// has IRQ 3.
	fInt_Mask = (fPort == kAS_COM1) ? kPIC_IRQ4 : kPIC_IRQ3;
	fInt_Number = (fPort == kAS_COM1) ? kIVT_COM1 : kIVT_COM2;

	// flush the transmit and receive buffers
	fQ_rx.Flush();
	fQ_tx.Flush();

	fBase = TAsynchGetBase(fPort);
	fOpen = True;
	fUART = GetUART();

	outportb(fBase+kUART_IER, kIER_NONE);	// turn off interrupts
	StateSave();				// record current UART settings
	if (!FormatSet(baud, parity, data, stop)) {
		fOpen = False;
		return (kAS_Err_Param);
		}
	fAS_Table[fPort] = this;		// set up jump table pointer
	setvect(fInt_Number, (fPort==kAS_COM1) ? Handler_1 : Handler_2);

	disable();
	outportb(fBase + kUART_MCR, kMCR_OUT2);	// allow com card to send interrupts
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_IER, kIER_ALL);	// enable serial interrupts
	IO_DELAY(3);				// delay for port to catch up
	cur_imr = inportb(kPIC_IMR);
	IO_DELAY(3);				// delay for port to catch up
	outportb(kPIC_IMR, cur_imr & fInt_Mask);
	IO_DELAY(3);				// delay for port to catch up
	inportb(fBase + kUART_RBR);		// clear any pending interrupts
	IO_DELAY(3);				// delay for port to catch up
	inportb(fBase + kUART_IIR);
	IO_DELAY(3);				// delay for port to catch up
	inportb(fBase + kUART_LSR);
	IO_DELAY(3);				// delay for port to catch up
	inportb(fBase + kUART_MSR);
	enable();

	fXmitting = False;
	return (kAS_Err_OK);
	}


// Peek at next character in buffer.

Boolean TAsynch::Peek(Byte *ch, Boolean wait) {
	Boolean	status;

	if (!fOpen)
		return (False);
	do {
		disable();
		status = fQ_rx.Look(ch);
		enable();
		} while (wait && !status);
	return (status);
	}


// Puts a character into transmit queue.  Returns true if all went well.

Boolean TAsynch::Put(Byte ch, Boolean wait) {
	Boolean	status;

	if (!fOpen)
		return (False);
	do {
		status = Transmit(ch);
		} while (wait && !status);
	return (status);
	}


// Puts a string of characters into transmit queue.  Returns the number of
// characters that have actually been transmitted.

Word TAsynch::Put(Word len, const Byte *buff, Boolean wait) {
	Word	count=0;

	if (!fOpen)
		return (count);
	while (len) {
		if (Transmit(*buff)) {
			buff++;
			count++;
			len--;
			}
		else if (!wait)
			break;
		}
	return (count);
	}


// RS485Board -- enables/disables the internal RS485 board to receive and
// transmit.

//void TAsynch::RS485Board(short mode) {
//	if (!fOpen)
//		return;
//	WaitTXEmpty();
//	disable();
//	outportb(fBase + kUART_SCR, mode);
//	enable();
//	}


// Setting enable to true will set DTR, while false will clear it.

void TAsynch::SetDTR(Boolean enable) {
	if (!fOpen)
		return;
	if (enable)
		outportb(fBase + kUART_MCR, inportb(fBase + kUART_MCR) |  kMCR_DTR);
	else
		outportb(fBase + kUART_MCR, inportb(fBase + kUART_MCR) & ~kMCR_DTR);
	}


// Setting enable to true will set RTS, while false will clear it.

void TAsynch::SetRTS(Boolean enable) {
	if (!fOpen)
		return;
	if (enable)
		outportb(fBase + kUART_MCR, inportb(fBase + kUART_MCR) |  kMCR_RTS);
	else
		outportb(fBase + kUART_MCR, inportb(fBase + kUART_MCR) & ~kMCR_RTS);
	}


// Restores the UART to the way it was found.

void TAsynch::StateRestore(void) {
	if (!fOpen)
		return;
	setvect(fInt_Number, fOld_Handler);
	disable();
	outportb(fBase + kUART_MCR, fOldMCR);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_LCR, fOldFormat | kLCR_DLAB);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_DLM, fOldBaudHi);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_DLL, fOldBaudLo);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_LCR, fOldFormat & ~kLCR_DLAB);
	enable();
	}


// Saves the current state of the UART so that is can be restored later.

void TAsynch::StateSave(void) {
	if (!fOpen)
		return;
	disable();
	fOldFormat = inportb(fBase + kUART_LCR);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_LCR, fOldFormat | kLCR_DLAB);
	IO_DELAY(3);				// delay for port to catch up
	fOldBaudHi = inportb(fBase + kUART_DLM);
	IO_DELAY(3);				// delay for port to catch up
	fOldBaudLo = inportb(fBase + kUART_DLL);
	IO_DELAY(3);				// delay for port to catch up
	outportb(fBase + kUART_LCR, fOldFormat & ~kLCR_DLAB);
	IO_DELAY(3);				// delay for port to catch up
	fOldMCR = inportb(fBase + kUART_MCR);
	enable();
	fOld_Handler = getvect(fInt_Number);
	}


// Return true if transmit holding register is empty

void TAsynch::WaitTX(void) {
	if (!fOpen)
		return;
	while (fXmitting)		// wait till tx interrupt is done
		;
	while ((inportb(fBase + kUART_LSR) & kLSR_THRE) == 0)
		IO_DELAY(3);		// delay for port to catch up
	}


// Return true if transmitter is empty

void TAsynch::WaitTXEmpty(void) {
	if (!fOpen)
		return;
	while (fXmitting)		// wait till tx interrupt is done
		;
	while ((inportb(fBase + kUART_LSR) & kLSR_TSRE) == 0)
		IO_DELAY(3);		// delay for port to catch up
	}
