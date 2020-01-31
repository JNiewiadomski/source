/* button.h -- December 27, 1989 */

/* Button Object Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   27-NOV-1989
 * ----------------------------------------------------------------------- */

#define BUTTON_BAD  -1
#define BUTTON_NONE -1

typedef void (*buttonFunc)(void);

extern short buttonPressed;

void    ButtonAddFunc(short index, buttonFunc func);
void    ButtonAddLED(short index, Point *p, short radius, L_STATE state);
void    ButtonAddText(short index, Point *p, short color, short size, char *text);
short   ButtonCreate(Rect *pos, short color, short led, short text, buttonFunc func);
void    ButtonDown(short index);
void    ButtonDraw(short index);
void    ButtonDrawAll(void);
short   ButtonFind(Point *p);
BOOLEAN ButtonReleased(Point *p);
void    ButtonSetLED(short index, L_STATE state);
BOOLEAN ButtonStillDown(short index);
void    ButtonUp(void);
