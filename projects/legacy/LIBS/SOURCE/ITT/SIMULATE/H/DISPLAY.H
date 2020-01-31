/* display.h -- January 3, 1990 */

/* Display Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   03-JAN-1990
 * ----------------------------------------------------------------------- */

#define DISP_ROWS 2
#define DISP_COLS 20

#define DISP_ALL_ON     128
#define DISP_ALT_ON     129
#define DISP_DELTA      130
#define DISP_DEGREE     131
#define DISP_UP_ARROW   132
#define DISP_DOWN_ARROW 133

typedef enum { DISP_LINE1, DISP_LINE2 };

void CursorEnable(BOOLEAN on);
void CursorSet(short row, short col, short len);

void    DisplayCleanUp(void);
void    DisplayClear(void);
void    DisplayClearLine(short row);
void    DisplayFill(char ch);
void    DisplayFlashLine(short row, BOOLEAN on);
BOOLEAN DisplayInit(Point *p, short fore, short back);
BOOLEAN DisplayPop(void);
BOOLEAN DisplayPush(void);
void    DisplayReset(void);
void    DisplayScrollLineOn(short row, char *text);
void    DisplayScrollLineOff(short row);
void    DisplayScrollDown(void);
void    DisplayScrollUp(void);
void    DisplaySetChar(short row, short col, char ch);
void    DisplaySetLine(short row, short col, char *text);
void    DisplayShowMessage(short line, char *text, JIFF delay);
void    DisplayTestSegments(void);
void    DisplayUpdate(void);
