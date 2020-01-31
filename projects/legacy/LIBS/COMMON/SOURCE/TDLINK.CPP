// ****************************
// * Version 2 DataLink Layer *
// ****************************

#include <common\common.h>
#include <common\ascii.h>
#include <common\tdlink.h>
#include <common\ttimer.h>

// Operation of the Data Link
//
// In a multipoint environment, the control station either polls or selects
// the tributary stations.  Polling is an "invitation to send" transmitted
// from the control station to a specific tributary station.  Selection is a
// "request to receive" notification from the control station to one of the
// tributary stations instructing it to receive the following message(s).

// Default TDlink idle handler.

void TDLink::DefaultIdle(void) {
	}


// TDlink class constructor.

TDLink::TDLink(Word size, Byte *tbuff, Byte *rbuff, DL_IdleFunc idleFunc) {
	fOpen = False;
	fSize = size;
	fTBuff = tbuff;
	fRBuff = rbuff;
	fIdleHandler = (idleFunc == 0) ? DefaultIdle : idleFunc;
	}


void TDLink::Close(void) {
	if (!fOpen)
		return;
	fAsynch.Close();
	fOpen = False;
	}


Boolean TDLink::Get(Byte *ch, Boolean calcCRC) {
	const DWord	kTimeOut=100L;		// set timeout for 1 second
	TTimer		timer;

	fGotDLE = False;
	while (fAsynch.CountTx() > 0)
		fIdleHandler();
	timer.Reset(kTimeOut);
	while (!fAsynch.Get(ch, False)) {
		fIdleHandler();
		if (timer.Elapsed())
			return (False);
		}
	if (calcCRC)
		fCRC.Update(*ch);
	if (*ch == A_DLE) {
		fGotDLE = True;
		timer.Reset(kTimeOut);
		while (!fAsynch.Get(ch, False)) {
			fIdleHandler();
			if (timer.Elapsed())
				return (False);
			}
		if (calcCRC)
			fCRC.Update(*ch);
		*ch &= 0x001F;			// make it a control character
		}
	return (True);
	}


Boolean TDLink::GetCRC(void) {
	Byte	hi, lo;

	if (!Get(&hi, False) || !Get(&lo, False))
		return (False);
	return (fCRC.Check(hi, lo));
	}


Boolean	TDLink::Open(Word port, DWord baud) {
	Close();
	if (fAsynch.Open(port, baud, kAS_None, 8, 1) != kAS_Err_OK)
		return (False);
	fDLERev = False;
	fGotDLE = False;
	fNum = 0;
	fOpen = True;
	return (fOpen);
	}


void TDLink::PutCtrl(Byte ch, Boolean calcCRC) {
	fAsynch.Put(ch);
	if (calcCRC)
		fCRC.Update(ch);
	}


// Control characters (rows 0 and 1) have the value of 0 in the 3 most
// significant bits.  Upper case characters (rows 4 and 5) have the value of
// 2 in the 3 most significant bits.

void TDLink::Put(Byte ch, Boolean calcCRC) {
	if (fDLERev) {
		if ((ch >> 5) == 2) {
			PutCtrl(A_DLE, calcCRC);
			PutCtrl(ch, calcCRC);
			}
		else if ((ch >> 5) == 0)
			PutCtrl(ch | 0x40, calcCRC);	// make it an ASCII character
		else					// it's a control character
			PutCtrl(ch, calcCRC);
		}
	else {
		if ((ch >> 5) == 0) {
			PutCtrl(A_DLE, calcCRC);
			PutCtrl(ch | 0x40, calcCRC);	// make it an ASCII character
			}
		else					// it's a control character
			PutCtrl(ch, calcCRC);
		}
	}


void TDLink::PutCRC(void) {
	Put(fCRC.GetHi(), False);
	Put(fCRC.GetLo(), False);
	}


// Receive() finds the start of a packet and passes off the accumulation of
// the individual packet checking to the appropriate routine.  A timeout
// or an invalid packet type returns kDL_TImeout.  Otherwise the result of the
// called function is returned.

DL_Resp TDLink::Receive(void) {
	Byte	count_down, ch;

	if (!fOpen)
		return (kDL_Unselected);
	count_down = 255;
	while (True) {				// keep getting new characters
		if (!Get(&ch, False))		// don't compute CRC just yet
			return (kDL_Timeout);	// timeouts return NUL
		if (ch == A_SOH)		// if it's an SOH, continue
			break;
		if (count_down-- == 0)
			return (kDL_Timeout);
		}
	count_down = 255;
	while (True) {				// eat up all SOH's
		GetInit();			// initialize CRC computation
		if (!Get(&ch))			// we want a running CRC from now on
			return (kDL_Timeout);	// timeouts return NUL
		if (ch != A_SOH)		// if it's not an SOH, continue
			break;
		if (count_down-- == 0)
			return (kDL_Timeout);
		}
	switch (ch) {				// let function handle rest of packet
		case 'A': return (ReceiveAck());
		case 'E': return (ReceiveError());
		case 'F': return (ReceiveFormat());
		case 'N': return (ReceiveNak());
		case 'R': return (ReceiveReply());
		case 'S': return (ReceiveSeq());
		case 'U': return (ReceiveUnselected());
		case 'W': return (ReceiveWait());
		}
	return (kDL_Timeout);			// unrecognized packet type
	}


DL_Resp TDLink::ReceiveAck(void) {
	Byte	seq, etx;

	if (!Get(&seq) || !Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	if (!fSeqNum.Match(seq))
		return (kDL_ReSend);
	fSeqNum.Next();
	return (kDL_Confirm);
	}


DL_Resp TDLink::ReceiveError(void) {
	Byte	seq, etx;

	if (!Get(&seq) || !Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	// Sequence number tells us which packet couldn't be processed.
	return (kDL_Error);
	}


DL_Resp TDLink::ReceiveFormat(void) {
	Byte	seq, etx;

	if (!Get(&seq) || !Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	// Even if sequence number is bad, resend.  So don't check it.
	return (kDL_ReSend);
	}


DL_Resp TDLink::ReceiveNak(void) {
	Byte	seq, etx;

	if (!Get(&seq) || !Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	// Even if sequence number is bad, resend.  So don't check it.
	return (kDL_ReSend);
	}


DL_Resp TDLink::ReceiveReply(void) {
	Byte	seq, ch, *bp;

	if (!Get(&seq) || !Get(&ch) || ch != A_ETB || !GetCRC())
		return (kDL_Timeout);
	if (!fSeqNum.Match(seq))
		return (kDL_ReSend);
	if (!Get(&ch, False) || ch != A_STX)
		return (kDL_Timeout);
	GetInit();				// initialize CRC calculation
	bp = fRBuff;
	fNum = 0;
	while (True) {
		if (!Get(&ch))
			return (kDL_Timeout);
		if (ch == A_ETX && !fGotDLE)	// we stop on an ETX
			break;
		if (fNum == fSize)
			return (kDL_Overflow);	// too many characters
		*bp++ = ch;
		fNum++;
		}
	if (!GetCRC())
		return (kDL_Timeout);
	fSeqNum.Next();
	return (kDL_Indication);
	}


DL_Resp TDLink::ReceiveSeq(void) {
	Byte	seq, etx;

	if (!Get(&seq) || !Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	fSeqNum.Set(seq);
	return (kDL_ReSend);
	}


DL_Resp TDLink::ReceiveUnselected(void) {
	Byte	etx;

	if (!Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	return (kDL_Unselected);
	}


DL_Resp TDLink::ReceiveWait(void) {
	Byte	seq, etx;

	if (!Get(&seq) || !Get(&etx) || etx != A_ETX || !GetCRC())
		return (kDL_Timeout);
	if (!fSeqNum.Match(seq))
		return (kDL_ReSend);
	return (kDL_Wait);
	}


void TDLink::SendBoot(void) {
	if (!fOpen)
		return;
	PutInit();
	PutCtrl(A_SOH, False);
	Put('B');
	Put(fSeqNum.Get());
	PutCtrl(A_ETX);
	PutCRC();
	}


void TDLink::SendClose(void) {
	if (!fOpen)
		return;
	PutInit();
	PutCtrl(A_SOH, False);
	Put('C');
	PutCtrl(A_ETX);
	PutCRC();
	}


void TDLink::SendData(Word len) {
	Word		ctrl, alpha, count;
	const Byte	*b;

	if (!fOpen)
		return;
	for (ctrl=0, alpha=0, count=len, b=fTBuff; count--; )
		switch (*b++ >> 5) {
			case 0:			// rows 0 and 1
				ctrl++;
				break;
			case 2:			// rows 4 and 5
				alpha++;
				break;
			}
	PutInit();
	PutCtrl(A_SOH, False);
	Put(ctrl > alpha ? 'd' : 'D');
	Put(fSeqNum.Get());
	PutCtrl(A_ETB);
	PutCRC();
	PutCtrl(A_STX, False);
	PutInit();
	if (ctrl > alpha)
		fDLERev = True;
	for (b=fTBuff; len--; )
		Put(*b++);
	fDLERev = False;
	PutCtrl(A_ETX);
	PutCRC();
	}


void TDLink::SendOpen(Byte type, Byte address) {
	if (!fOpen)
		return;
	fSeqNum.Init();
	PutInit();
	PutCtrl(A_SOH, False);
	Put('O');
	Put(type);
	Put(address);
	PutCtrl(A_ETX);
	PutCRC();
	}


void TDLink::SendQuery(Byte type, Byte address) {
	if (!fOpen)
		return;
	PutInit();
	PutCtrl(A_SOH, False);
	Put('Q');
	Put(type);
	Put(address);
	PutCtrl(A_ETX);
	PutCRC();
	}
