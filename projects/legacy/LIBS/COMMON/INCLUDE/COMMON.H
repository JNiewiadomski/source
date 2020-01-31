#ifndef _COMMON_H
#define _COMMON_H

#ifndef __IO_H
#include <io.h>
#endif

// **********************
// * Common Definitions *
// **********************

// Define NULL just in case standard headers are not used.

//#ifndef NULL
//#	if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
//#		define	NULL	0
//#	else
//#		define	NULL	0L
//#	endif
//#endif


// offsetof allows you to find the starting location of an identifier
// inside of a structure.

#ifndef offsetof
#define offsetof(type, identifier) ((unsigned)(&(((type *)0)->identifier)))
#endif


// NUMELE(a) evaluates to the array size in elements.  This only works for
// actual arrays, and not pointers to arrays.

#define NUMELE(a)	(sizeof(a) / sizeof(*(a)))


// The max and min macros return the larger and smaller of the two values
// respectively.

#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define min(a,b)        (((a) < (b)) ? (a) : (b))


// Definitions of commonly used data types.

typedef	unsigned char	Byte;		// 1 byte
typedef	unsigned short	Word;		// 2 bytes
typedef	unsigned long	DWord;		// 4 bytes


typedef void interrupt (*isrFunc)(...);


// Definition of the Boolean data type.

#ifndef __TTYPES_H
enum Boolean { False, True };
#endif


// ***********************
// * Function Prototypes *
// ***********************

// AlertSound() turns on the speaker at a certain frequency for a short
// duration.

void	AlertSound(void);

// FileExists(file_name) returns True if file_name exists.

inline Boolean FileExists(const char *path) { return ((access(path, 0) == 0) ? True : False); }

// TempName() returns a new nonconflicting temporary file name.  If the DOS
// environment variables TMP or TEMP are defined, its values are used as the
// path, in that order.  If they are not defined, the current directory is
// used.  Use delete to dispose of the free store used to hold the file name
// when it is no longer needed.  If there wasn't enough free store to hold
// the name or a unique name could not be generated, 0 is returned.

char	*TempName(void);


#endif
