// The Printer Manager handles the process of transmitting data to either
// the parallel or serial printer, displaying the current printer status
// on screen, and recognizing the <Esc> key as the command to abort
// printing.

enum P_Type { P_None, P_Parallel, P_Serial };

void	PrinterClose(void);
int	PrinterGetKey(void);
void	PrinterInit(int key);
Boolean	PrinterOpen(void);
Boolean	PrinterSelectParallel(void);
Boolean	PrinterSelectSerial(int port, int baud, int parity, int data, int stop);
void	PrinterSetKey(int key);
Boolean	PrinterWrite(char *format, ...);
