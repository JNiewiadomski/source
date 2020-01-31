// The Error module provides a mechanism to issue warnings and errors to
// the user.  The individual occurences of warnings and errors are tallied,
// and may be displayed as statistics.  All messages are issued to stdout.
// This allows the caller to redirect stdout and capture all warnings and
// errors.

enum {				// return error codes
	ERR_NONE,		// no errors
	ERR_PARAMS,		// error in parameters
	ERR_SYNTAX,		// syntax error in file
	ERR_PROG		// I/O or program error
	};

#define e_OutOfMemory(msg)	ErrorFatalList(0, msg)
#define e_Internal(msg)		ErrorFatalList(1, msg)

void	ErrorFatal(char *format, ...);
void	ErrorFatalList(short index, char *msg);
Boolean	ErrorNoSevere(void);
void	ErrorSevere(char *format, ...);
Boolean	ErrorStats(void);
void	ErrorWarning(char *format, ...);
