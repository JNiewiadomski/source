#ifndef _TFILE_H
#define _TFILE_H

#ifndef __FCNTL_H
#include <fcntl.h>
#endif

#ifndef __IO_H
#include <io.h>
#endif

#ifndef __STAT_H
#include <sys\stat.h>
#endif

// ***************
// * TFile Class *
// ***************

class TFile {
	short	fHandle;			// handle to opened file
public:
	TFile() { fHandle = -1; }
	~TFile() { Close(); }

	void	Close(void) { if (fHandle == -1) return; close(fHandle); fHandle = -1; }
	int	GetFTime(struct ftime *ftimep) { return (getftime(fHandle, ftimep)); }
	Boolean	IsOpen(void) { return (fHandle == -1 ? False : True); }
	Boolean	Open(const char *name, int access, Word mode=0x0000) { fHandle = open(name, access, mode); return (IsOpen()); }
	Boolean	Print(const char *format, ...);
	Boolean	Read(void *buf, Word len) { return (read(fHandle, buf, len) == len ? True : False); }
	Boolean	ReadLine(void *buf, Word len, Byte nl='\n');
	long	Seek(long offset, int fromwhere) { return (lseek(fHandle, offset, fromwhere)); }
	long	Tell(void) { return (tell(fHandle)); }
	Boolean	Write(const void *buf, Word len) { return (write(fHandle, buf, len) == len ? True : False); }
	};

#endif
