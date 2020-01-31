// *****************
// * TBitSet Class *
// *****************

#include <common\common.h>
#include <common\tbitset.h>


// Class constructor to allocate

TBitSet::TBitSet(DWord size) {
	Word	alloc;

	alloc = Bits2Bytes(size);
	if ((fData = new Byte[alloc]) == NULL)
		return;
	fLen = size;
	memset(fData, '\x00', alloc);
	}


TBitSet::TBitSet(TBitSet &bs) {
	Word	alloc;

	alloc = Bits2Bytes(bs.fLen);
	if ((fData = new Byte[alloc]) == NULL)
		return;
	fLen = bs.fLen;
	memcpy(fData, bs.fData, alloc);
	}


TBitSet::~TBitSet(void) {
	if (fData != NULL)
		delete (fData);
	}


void TBitSet::operator=(TBitSet &bs) {
	Word	alloc;

	alloc = Bits2Bytes(bs.fLen);
	if (fLen != bs.fLen) {
		fLen = bs.fLen;
		if (fData != NULL)
			delete fData;
		fData = new Byte[alloc];
		}
	if (fData)
		memcpy(fData, bs.fData, alloc);
	else
		fLen = 0;
	}


TBitSet TBitSet::operator&(TBitSet &bs) {
	TBitSet	result;
	DWord	bit;

	if (fLen < bs.fLen) {
		result = bs;
		for (bit=0; bit < fLen; ++bit)
			if ((*this)[bit])
				result.Include(bit);
		}
	else {
		result = *this;
		for (bit = 0; bit < bs.fLen; ++bit)
			if (bs[bit])
				result.Include(bit);
		}
	return result;
	}


TBitSet TBitSet::operator&=(TBitSet &bs) {
	*this = *this & bs;
	return *this;
	}


TBitSet TBitSet::operator+(TBitSet &bs) {
	TBitSet	result;

	result = *this & bs;
	return result;
	}


TBitSet TBitSet::operator+=(TBitSet &bs) {
	TBitSet	result;

	result = *this &= bs;
	return result;
	}


TBitSet TBitSet::operator|(TBitSet &bs) {
	TBitSet	result;
	DWord	max;

	if (fLen > bs.fLen) {
		result = TBitSet(fLen);
		max    = bs.fLen;
		}
	else {
		result = TBitSet(bs.fLen);
		max    = fLen;
		}
	for (DWord bit = 0; bit < max; ++bit)
		if ((*this)[bit] &bs[bit])
			result.Include(bit);
	return result;
	}


TBitSet TBitSet::operator|=(TBitSet &bs) {
	*this = *this | bs;
	return *this;
	}


TBitSet TBitSet::operator-(TBitSet &bs) {
	TBitSet	result = *this;
	DWord	stop;

	stop = (fLen < bs.fLen) ? fLen : bs.fLen;
	for (DWord bit = 0; bit < stop; ++bit)
		if (bs[bit])
			result.Exclude(bit);
	return result;
	}


TBitSet TBitSet::operator-=(TBitSet &bs) {
	*this = *this - bs;
	return *this;
	}


TBitSet TBitSet::operator~() {
	TBitSet	result(fLen);

	for (DWord bit = 0; bit < fLen; ++bit)
		if ((*this)[bit])
			result.Exclude(bit);
		else
			result.Include(bit);
	return result;
	}


Boolean TBitSet::operator==(TBitSet &bs) {
	if (fLen != bs.fLen)
		return (False);
	for (DWord bit = 0; bit < fLen; ++bit)
		if ((*this)[bit] != bs[bit])
			return (False);
	return (True);
	}


Boolean TBitSet::operator!=(TBitSet &bs) {
	if (fLen != bs.fLen)
		return (True);

	DWord	bit = 0;
	while (bit < fLen)
		if ((*this)[bit] == bs[bit])
			++bit;
		else
			return (True);
	return (False);
	}
