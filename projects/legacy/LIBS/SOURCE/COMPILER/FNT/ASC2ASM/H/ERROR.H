enum ERR_CODE {	ERR_OK,			// no errors
		ERR_PARAMS,		// error in parameters
		ERR_SYNTAX,		// syntax error in file
		ERR_PROG		// I/O or program error
		};

#define e_OutOfMemory(msg)	ErrorFatalList(0, msg)
#define e_Internal(msg)		ErrorFatalList(1, msg)

void	ErrorFatal(char *format, ...);
void	ErrorFatalList(short index, char *msg);
