#ifndef _TDLINK_H
#define _TDLINK_H

// ****************************
// * Version 2 DataLink Layer *
// ****************************

#ifndef _TASYNCH_H
#include <common\tasynch.h>
#endif

#ifndef _TCRC_H
#include <common\tcrc.h>
#endif

// *************************
// * Sequence Number Class *
// *************************

class TDL_Seq {
	Byte	fSeqBase, fNum;
public:
	TDL_Seq(void) { fSeqBase=0x80; Init(); }

	Byte	Get(void) { return (fNum); }
	void	Init(void) { fNum = fSeqBase; }
	Boolean	Match(Byte seq) { return (seq == fNum ? True : False); }
	void	Next(void) { fNum = (fNum + 1) | fSeqBase; }
	void	Set(Byte seq) { fNum = seq | fSeqBase; }
	};

// ******************
// * DataLink Class *
// ******************

typedef void (*DL_IdleFunc)(void);

enum DL_Resp { kDL_Confirm, kDL_Error, kDL_Indication, kDL_Overflow, kDL_ReSend, kDL_Timeout, kDL_Unselected, kDL_Wait };

class TDLink {
	Boolean		fOpen	: 1;	// has datalink been opened?
	Boolean		fDLERev	: 1;	// DLE 'NUL'..'US' or '@'..'_'
	Boolean		fGotDLE	: 1;	// did we just get a DLE?
	DL_IdleFunc	fIdleHandler;	// idle time function pointer
	TAsynch		fAsynch;	// physical layer for serial communications
	Word		fSize;		// maximum size of fRBuff
	Word		fNum;		// number of bytes in fTBuff & fRBuff
	Byte		*fTBuff;	// transmit buffer
	Byte		*fRBuff;	// receive buffer
	TCRC		fCRC;		// transmit/receive checksum
	TDL_Seq		fSeqNum;	// sequence number

	Boolean	Get(Byte *ch, Boolean calcCRC=True);
	Boolean	GetCRC(void);
	void	GetInit(void) { fCRC.Init(); }
	void	Put(Byte ch, Boolean calcCRC=True);
	void	PutCRC(void);
	void	PutCtrl(Byte ch, Boolean calcCRC=True);
	void	PutInit(void) { fCRC.Init(); }
	DL_Resp	ReceiveAck(void);
	DL_Resp	ReceiveError(void);
	DL_Resp	ReceiveFormat(void);
	DL_Resp	ReceiveNak(void);
	DL_Resp	ReceiveReply(void);
	DL_Resp	ReceiveSeq(void);
	DL_Resp	ReceiveUnselected(void);
	DL_Resp	ReceiveWait(void);

	static void	DefaultIdle(void);
public:
	TDLink(Word size, Byte *tbuff, Byte *rbuff, DL_IdleFunc idleFunc);
	~TDLink() { Close(); }

	void	Close(void);
	Word	DumbTerminal(Word len, const Byte *tbuff) { return (fAsynch.Put(len, tbuff)); }
	Boolean	Open(Word port, DWord baud);
	DL_Resp	Receive(void);
	Word	ReceiveData(void) { return (fNum); }
	void	SendBoot(void);
	void	SendClose(void);
	void	SendData(Word len);
	void	SendOpen(Byte type, Byte address);
	void	SendQuery(Byte type, Byte address);
	};

#endif
