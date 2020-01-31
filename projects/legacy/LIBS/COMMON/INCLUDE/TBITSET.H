// *****************
// * TBitSet Class *
// *****************

#ifndef _TBITSET_H
#define _TBITSET_H

#ifndef __LIMITS_H
#include <limits.h>
#endif

#ifndef __MEM_H
#include <mem.h>
#endif

class TBitSet {
	Byte	BitInByte(DWord bit) { return (1 << (bit & (CHAR_BIT-1))); }
	Word	Bits2Bytes(DWord len) { return ((len + CHAR_BIT - 1) / CHAR_BIT); }
	Word	BytePos(DWord bit) { return (bit / CHAR_BIT); }
protected:
	DWord	fLen;			// number of bits stored in the set
	Byte	*fData;			// pointer to the bit set memory

	TBitSet() { fLen=0L; fData=NULL; }
public:
	TBitSet(DWord size);
	TBitSet(TBitSet &bs);
	~TBitSet();

	void	AllOff(void) { memset(fData, '\x00', Bits2Bytes(fLen)); }
	void	AllOn(void) { memset(fData, '\xFF', Bits2Bytes(fLen)); }
	void	Exclude(DWord bit) { if (bit < fLen) fData[BytePos(bit)] &= ~BitInByte(bit); }
	void	Include(DWord bit) { if (bit < fLen) fData[BytePos(bit)] |=  BitInByte(bit); }
	DWord	Size(void) { return fLen; }

	void	operator=(TBitSet &bs);

	TBitSet	operator&(TBitSet &bs);		// union of two TBitSets
	TBitSet	operator&=(TBitSet &bs);

	TBitSet	operator+(TBitSet &bs);		// union of two TBitSets
	TBitSet	operator+=(TBitSet &bs);

	TBitSet	operator|(TBitSet &bs);		// intersection of two TBitSets
	TBitSet	operator|=(TBitSet &bs);

	TBitSet	operator-(TBitSet &bs);		// intersection of two TBitSets
	TBitSet	operator-=(TBitSet &bs);

	TBitSet	operator~(void);		// reverses the settings of the bits in a set

	Boolean	operator==(TBitSet &bs);	// comparison between two sets
	Boolean	operator!=(TBitSet &bs);

	Boolean	operator[](DWord bit) { if (bit < fLen) return (fData[BytePos(bit)] & BitInByte(bit) ? True : False); else return (False); }
	};

#endif
