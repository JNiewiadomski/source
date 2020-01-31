#ifndef _CRC_H
#define _CRC_H

// *******************
// * CCITT CRC Class *
// *******************

class TCRC {
	Word	fCRC;
public:
	TCRC(void) { Init(); }

	Boolean	Calc(short size, void *block);
	Boolean	Check(Word val) { return ((fCRC == val) ? True : False); }
	Boolean	Check(Byte hi, Byte lo) { return ((fCRC == ((hi<<8) | lo)) ? True : False); }
	void	Init(void) { fCRC = 0; }
	Word	Get(void) { return (fCRC); }
	Byte	GetHi(void) { return (fCRC >> 8); }
	Byte	GetLo(void) { return (fCRC & 0x00FF); }
	void	Update(Byte data);
	};

#endif
