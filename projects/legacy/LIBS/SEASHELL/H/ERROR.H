// The Error module provides routines to issue error messages to the user.
// The Resource and Window managers are utilized if they have been
// initialized.

typedef void (*CleanUpFunc)(void);

enum Err_Type {	Err_Fatal, Err_Recover, Err_Warning };

void	Error(Err_Type type, char *format, ...);
char	*ErrorGet(short id, short index);
void	ErrorInit(CleanUpFunc func);
void	ErrorSound(void);
