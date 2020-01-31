#define SCREEN_HEIGHT	25	// height of physical screen
#define SCREEN_WIDTH	80	// width  of physical screen

#define GIANT	9999

enum DIRECTION { D_UP, D_DOWN, D_LEFT, D_RIGHT };
enum WTYPE { WNONE, W0000, W1111, W2222, W2111 };

extern Rect GIANT_RECT;

void	ScreenInvert(void);
void	ScreenInvertActivate(Boolean on);
void	ScreenSynch(void);

short	HiliteStrLen(char *s);

Boolean	VirtualAvail(void);
void	VirtualDispose(short vsnum);
void	VirtualFill(short vsnum, Rect &vrectp, char ch, short attr);
void	VirtualGetCursor(short vsnum, short *vrowp, short *vcolp);
short	VirtualGetNew(Word id, Rect *r);
void	VirtualGetPhysical(short vsnum, Rect &srectp);
void	VirtualGetSize(short vsnum, short *nrowsp, short *ncolsp);
char	*VirtualGetTitle(short vsnum);
Boolean	VirtualGetVideo(short num, Word id, Point &loc);
void	VirtualGetWinLoc(short vsnum, short *vrow1p, short *vcol1p);
short	VirtualGlobalCol(short vsnum, short pos);
short	VirtualGlobalRow(short vsnum, short pos);
void	VirtualHide(short vsnum);
void	VirtualInit(void);
short	VirtualNew(short nrows, short ncols, WTYPE type, Rect &srectp, char *title, short attr);
void	VirtualPan(short wnum, DIRECTION dir, short dist);
void	VirtualPrint(short vsnum, short vrow, short vcol, short ncols, short attr, char *format, ...);
void	VirtualSelect(short vsnum);
void	VirtualSetCursor(short vsnum, short vrow, short vcol);
void	VirtualSetPhysical(short vsnum, Rect &srectp);
void	VirtualSetTitle(short vsnum, char *title);
void	VirtualShow(short vsnum);
void	VirtualShowCursor(short vsnum, Boolean on);
void	VirtualSlide(short vsnum, Rect &vrectp, DIRECTION dir, short dist);
void	VirtualWrite(short vsnum, short vrow1, short vcol1, short ncols, char *str, short attr);
void	VirtualWrite2(short vsnum, short vrow1, short vcol1, short ncols, char *str, short lo, short hi);
