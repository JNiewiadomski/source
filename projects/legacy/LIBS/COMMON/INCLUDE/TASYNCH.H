#ifndef _TASYNCH_H
#define _TASYNCH_H

// *************************************
// * Asynchronous Communications Class *
// *************************************

class TAS_Q {
	Word	fCount;				// number of Bytes in buff
	Byte	fFront, fRear;			// variable sizes are magic
	Byte	fBuff[256];			// pointers automatically wrap around
public:
	TAS_Q(void) { fCount=0; fFront=fRear=0; }

	Word	Count(void) { return (fCount); }
	Boolean	Get(Byte *ch);
	void	Flush(void) { fCount = 0; }
	Boolean	Look(Byte *ch);
	Boolean	Put(Byte ch);
	};

enum { kAS_Err_OK, kAS_Err_Param, kAS_Err_Exist, kAS_Err_Open };
enum { kAS_COM1, kAS_COM2 };
enum { kAS_None, kAS_Odd, kAS_Even, kAS_Mark, kAS_Space };
enum { kAS_Unknown, kAS_8250, kAS_16450, kAS_16550, kAS_16550A };

class TAsynch {
	static TAsynch	*fAS_Table[];

	Boolean	fOpen		: 1;
	Boolean	fXmitting	: 1;
	TAS_Q	fQ_rx, fQ_tx;			// receive and transmit queues
	Word	fPort;
	Byte	fInt_Mask, fInt_Number;
	Word	fBase;				// serial port base pointer
	Word	fUART;
	Byte	fDelay_Char, fShift_Char;	// used by IO_DELAY macro
	Byte	fOldBaudHi, fOldBaudLo, fOldFormat, fOldMCR;
	isrFunc	fOld_Handler;
	Byte	fLine_Status, fModem_Status;	// status indicators

	Boolean	FormatGet(DWord *baud, Byte *parity, Byte *data, Byte *stop);
	Boolean	FormatSet(DWord baud, Byte parity, Byte data, Byte stop);
	Word	GetUART(void);
	void	Handler(void);
	void	StateRestore(void);
	void	StateSave(void);
	void	WaitTX(void);
	void	WaitTXEmpty(void);

	inline Boolean	Transmit(Byte ch);

	static void interrupt far	Handler_1(...);
	static void interrupt far	Handler_2(...);
public:
	TAsynch() { fOpen = False; }
	~TAsynch() { Close(); }

	void	Break(void);
	void	Close(void);
	Byte	CountRx(void);
	Byte	CountTx(void);
	Boolean	Get(Byte *ch, Boolean wait=True);
	Word	Open(Word port, DWord baud, Byte parity, Byte data, Byte stop);
	Boolean	Peek(Byte *ch, Boolean wait=True);
	Boolean	Put(Byte ch, Boolean wait=True);
	Word	Put(Word len, const Byte *buff, Boolean wait=True);
	void	SetDTR(Boolean enable);
	void	SetRTS(Boolean enable);
	};

Boolean	TAsynchExist(Word port);

#endif
