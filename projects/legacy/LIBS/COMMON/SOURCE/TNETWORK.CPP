// ***************************
// * Version 2 Network Layer *
// ***************************

#include <common\common.h>
#include <common\tnetwork.h>
#include <common\ttimer.h>

// ****************
// * TRetry Class *
// ****************

class TRetry {
	short	count;
public:
	TRetry(void) { count = 5; }

	Boolean	Next(void) { if (count == 0) return (False); count--; return (True); }
	};


// **************************
// * TNetwork Class Methods *
// **************************

Boolean TNetwork::Connect(Byte type, Byte address) {
	TRetry	retry;

	fDLink.SendOpen(type, address);
	do {
		switch (fDLink.Receive()) {
			case kDL_Confirm:
				fType = type;
				fAddress = address;
				return (True);
			case kDL_Timeout:
				fDLink.SendQuery(type, address);
				break;
			default:
				fDLink.SendOpen(type, address);
				break;
			}
		} while (retry.Next());
	fDLink.SendClose();
	return (False);
	}


NetStat TNetwork::Data(Word *len) {
	const DWord	kWaitDelay=12L;			// wait for 120 mS
	TRetry		retry;
	TTimer		timer;

	fDLink.SendData(*len);
	while (True) {
		switch (fDLink.Receive()) {
			case kDL_Confirm:
				*len = 0;
				return (kNetOK);
			case kDL_Error:
				*len = 0;
				return (kNetError);
			case kDL_Indication:
				*len = fDLink.ReceiveData();
				return (kNetOK);
			case kDL_Overflow:
				*len = 0;
				return (kNetOverflow);
			case kDL_Timeout:
				fDLink.SendQuery(fType, fAddress);
				break;
			case kDL_Wait:
				timer.Reset(kWaitDelay);
				timer.Wait(fIdle);
				fDLink.SendQuery(fType, fAddress);
				continue;
			case kDL_Unselected:
				*len = 0;
				return (kNetUnselected);
			default:
				fDLink.SendData(*len);
				break;
			}
		if (!retry.Next())
			break;
		}
	*len = 0;
	return (kNetTimeout);
	}


Boolean TNetwork::Disconnect(void) {
	TRetry	retry;

	fDLink.SendClose();
	fDLink.SendQuery(fType, fAddress);
	do {
		switch (fDLink.Receive()) {
			case kDL_Confirm:		// seq packet type
			case kDL_Unselected:
				return (True);
			case kDL_Timeout:
				fDLink.SendQuery(fType, fAddress);
				break;
			default:
				fDLink.SendClose();
				fDLink.SendQuery(fType, fAddress);
				break;
			}
		} while (retry.Next());
	fDLink.SendClose();
	return (False);
	}
