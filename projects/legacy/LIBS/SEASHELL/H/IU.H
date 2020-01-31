// The International conversion package provides the routines needed to
// display the time, date, and decimal numbers in the format used in
// different countries.

#define ERR_INTL_OK	0
#define ERR_INTL_DATE	-1
#define ERR_INTL_TIME	-2

#define IU_DATE_LEN	10
#define IU_TIME_LEN	8
#define IU_LTIME_LEN	10

void	*IUGet(void);
void	*IUGet(short id);
char	*IUGetDatePrompt(void);
char	IUGetDecimal(void);
char	*IUGetTimePrompt(void);
DWord	IUHrs2LTime(float htime);
void	IUInit(void);
void	IUIDate2Str(Word idate, char *sdate);
void	IUInt2LTime(Word hour, Word min, Word sec, DWord *ltime);
void	IUITime2Str(Word itime, char *stime);
DWord	IULTimeAdd(DWord ltime1, DWord ltime2);
float	IULTime2Hrs(DWord ltime);
void	IULTime2Str(DWord ltime, char *stime);
Word	IUPacked2IDate(char *sdate);
Word	IUPacked2ITime(char *stime);
DWord	IUPacked2LTime(char *stime);
int	IUStr2IDate(char *sdate, Word *idate);
int	IUStr2ITime(char *stime, Word *itime);
