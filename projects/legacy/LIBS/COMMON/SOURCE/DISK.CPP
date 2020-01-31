// *******************
// * Disk Statistics *
// *******************

#include <dos.h>

#include <common\old\disk.h>


// DiskFreeSpace -- returns the number of bytes of disk space left.

long DiskFreeSpace(int drive) {
	struct dfree dp;

	getdfree(drive, &dp);
	if (dp.df_sclus == 0xFFFF)
		return (-1);
	else
		return ((long)(dp.df_bsec)*(long)(dp.df_sclus)*(long)(dp.df_avail));
	}


// DiskTotalSize -- returns the number of bytes that disk can store.

long DiskTotalSize(int drive) {
	struct dfree dp;

	getdfree(drive, &dp);
	if (dp.df_sclus == 0xFFFF)
		return (-1);
	else
		return ((long)(dp.df_bsec)*(long)(dp.df_sclus)*(long)(dp.df_total));
	}
