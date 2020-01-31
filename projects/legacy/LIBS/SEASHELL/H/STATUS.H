// The Status Manager takes care of manipulating the status bar on the last
// line of the display.

char	*StatusGet(short id, short index);
void	StatusInit(char *format, ...);
void	StatusPop(void);
void	StatusPrint(short pos, char *format, ...);
void	StatusPush(short id, short index);
void	StatusPush(char *format, ...);
void	StatusSet(char *msg);
void	StatusSet(short id, short index);
