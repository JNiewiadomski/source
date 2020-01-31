/* led.h -- December 5, 1989 */

/* LED Object Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   05-DEC-1989
 * ----------------------------------------------------------------------- */

#define LED_BAD -1

typedef enum { L_OFF, L_RED, L_GREEN, L_FL_RED, L_FL_GREEN, L_NEXT, L_PREV } L_STATE;


short LEDCreate(Point *pos, short radius, L_STATE state);
void  LEDDraw(short index);
void  LEDDrawAll(void);
void  LEDInit(short off, short on1, short on2);
void  LEDSet(short index, L_STATE state);
void  LEDSetAll(L_STATE state);
void  LEDUpdate(void);
