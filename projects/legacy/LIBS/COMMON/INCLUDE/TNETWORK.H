#ifndef _NETWORK_H
#define _NETWORK_H

#ifndef _TDLINK_H
#include <common\tdlink.h>
#endif

// ***************************
// * Version 2 Network Layer *
// ***************************

enum NetStat { kNetError, kNetUnselected, kNetOverflow, kNetTimeout, kNetOK };

class TNetwork {
	TDLink		fDLink;		// datalink layer
	DL_IdleFunc	fIdle;		// idle function called when waiting
	Byte		fType;		// message manager type
	Byte		fAddress;	// message manager address
public:
	TNetwork(Word size, Byte *tbuff, Byte *rbuff, DL_IdleFunc idleFunc) : fDLink(size, tbuff, rbuff, idleFunc) { fIdle=idleFunc; }
	~TNetwork() { Close(); }

	void	Close(void) { fDLink.Close(); }
	Boolean	Connect(Byte type, Byte address);
	NetStat	Data(Word *len);
	Boolean	Disconnect(void);
	Word	DumbTerminal(Word len, const Byte *tbuff) { return (fDLink.DumbTerminal(len, tbuff)); }
	Boolean	Open(Word port, DWord baud) { return (fDLink.Open(port, baud)); }
	void	Reboot(void) { fDLink.SendBoot(); }
	};

#endif
