#define e_OutOfMemory(msg)	ErrorFatalList(0, msg)
#define e_Internal(msg)		ErrorFatalList(1, msg)

void	ErrorFatal(char *format, ...);
void	ErrorFatalList(short index, char *msg);
Boolean	ErrorNoSevere(void);
void	ErrorRunTime(char *format, ...);
void	ErrorSevere(char *format, ...);
Boolean	ErrorStats(void);
void	ErrorWarning(char *format, ...);
