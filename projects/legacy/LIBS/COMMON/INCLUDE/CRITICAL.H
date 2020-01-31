#ifndef _CRITICAL_H
#define _CRITICAL_H

// ***************************
// * Critical Error Routines *
// ***************************

// Critical allows the running program to regain control when a critical
// error occurs.  The terminateFunc function will be called if the user
// chooses to ABORT program execution due to a critical error.  If
// CriticalInstall is called, the calling program must define a function
// named CriticalErrorHandler of the form:
//		Byte CriticalErrorHandler(Word errCode);
// The parameter errCode indicates the code of the error.  See 'The DOS
// Technical Reference Manual' for details.  The global errInfo indicates
// specifics about the error and which responses are valid.
// 		Bit 15 = 0 if there was a disk error
//		Bit 13 = 1 if IGNORE is allowed
//		Bit 12 = 1 if RETRY is allowed
//		Bit 11 = 1 if FAIL is allowed
// At this point, an error message should be displayed and the user should
// be prompted for an action, such as Abort, Retry, Fail or Ignore.
// The critical error handler should return:
//		'I' for ignore
//		'R' for retry
//		'F' for fail
//		'A' for abort
// If an ABORT was returned, then program execution will continue at the
// function terminateFunc declared when CriticalInstall was called.  The
// global errFlag will be set to TRUE to indicate that a FAIL has been
// issued.
//
// NOTE: the critical error handler may only use DOS calls 01h through 0Ch,
// 30h, and 59h.  Use of any other call destroys the DOS stack and leaves DOS
// in an unpredictable state.

#define BIT_FAIL	0x0800
#define BIT_RETRY	0x1000
#define BIT_IGNORE	0x2000

extern Byte	errFlag;
extern Word	errInfo;

Byte	CriticalErrorHandler(Word errCode);
void	CriticalInstall(void (*terminateFunc)());

#endif
