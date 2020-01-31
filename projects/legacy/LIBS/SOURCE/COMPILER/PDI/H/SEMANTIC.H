#define DMA_BYTE_TRANSFER	1
#define DMA_WORD_TRANSFER	5

#define DEV_BYTE		0
#define DEV_WORD		1

#define DEV_INPUT		0
#define DEV_OUTPUT		1

#define DEV_AUTO_OFF		0
#define DEV_AUTO_ON		1

#define DEV_CLOCK_EXTERNAL	0
#define DEV_CLOCK_TIMER		1

#define DEV_INTER_DISABLE	-1
#define DEV_INTER_EXT_POS	0
#define DEV_INTER_EXT_NEG	1
#define DEV_INTER_TERMINAL	2
#define DEV_INTER_TIMER		3

#define DEV_CONFIG_NO_CHANGE	0
#define DEV_CONFIG_CHANGE	1

struct dev_rec {
	L_TOKEN	select;			/* hardware device selected */
	double	xtalSpeed;		/* crystal clock speed of timer */
	int	baseAddress;		/* 0x100 - 0x3F0 */
	int	dmaLevel;		/* 0-1, 3, 5-7 */
	int	interruptLevel;		/* 9-12, 15 */
	int	portMode;		/* 0=byte I/O, 1=word I/O */
	Word	numTransfers;		/* number of transfers in bytes or words */
	int	direction;		/* 0=input, 1=output */
	int	autoRecycle;		/* 0=off, 1=on */
	int	clockSource;		/* 0=external, 1=timer */
	Word	buffSeg;		/* transfer segment of D.M.A. buffer */
	Word	buffOff;		/* transfer offset  of D.M.A. buffer */
	Word	counter0;		/* 2 - 65535 */
	Word	counter1;		/* 2 - 65535 */
	double	frequency;		/* actual base rate in Hz */
	double	unitTime;		/* smallest time frame in uS */
	int	interruptSource;	/* -1=disable, 0=extern pos, 1=extern neg, 2=D.M.A. terminal, 3=timer */
	Word	aux[3];			/* auxiliary data bits */
	};

extern struct dev_rec	dev;

void	SemanticAction(Symbol *sym, char *mask, Q_TYPE qualify, long val);
void	SemanticDeviceBase(int adr);
void	SemanticDeviceClock(int source, long desired);
void	SemanticDeviceInterrupt(int source);
void	SemanticDeviceMode(int mode);
void	SemanticDeviceSelect(L_TOKEN select);
Symbol	*SemanticID(T_TYPE type);
void	SemanticState(Symbol *sym, char *mask);
void	SemanticText(Symbol *sym);
