#ifndef _SYSTEM_H
#define _SYSTEM_H

// *************************
// * System Level Routines *
// *************************

// The SYSTEM module provides assembly routines which give the caller access
// to low-level functions.

struct EquipRecord {
	short	parallel;			// number of parallel ports
	short	serial;				// number of serial ports
	short	drives;				// number of block devices
	};

extern EquipRecord	equip;

void	EquipInit(void);			// initialize equip record
short	ShortHi(long val);
short	ShortLo(long val);
long	ShortShort(short hi, short lo);
Word	SysDate(void);				// current date
Word	SysTime(void);				// current time

#endif
