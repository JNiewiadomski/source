/* state.c -- February 5, 1990 */

#define DEBUG ShiftStackVal();

/* Shift State Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   05-JAN-1990
 * ----------------------------------------------------------------------- */

#include "state.h"

#define STACK_MAX 10

static S_STATE shiftState=S_OFF, shiftStack[STACK_MAX];
static short   shiftStackPos=0;

#ifdef DEBUG
  #include <graphics.h>
  #include <stdio.h>

  #include <..\seachest\seachest.h>
  #include <..\seachest\mouse.h>

  static void ShiftStackVal(void);
#endif


#ifdef DEBUG
  static void ShiftStackVal(void)
    {
      char buff[30];

      setcolor(WHITE);
      setfillstyle(SOLID_FILL, BLACK);
      settextstyle(SMALL_FONT, HORIZ_DIR, 5);
      mouseHideCursor();
      bar(0, 40, 150, 60);
      sprintf(buff, "State=%2d, Stack=%2d", shiftState, shiftStackPos);
      outtextxy(0, 42, buff);
      mouseShowCursor();
      }
#endif


S_STATE ShiftStateGet(void)
  {
    return (shiftState);
    }


S_STATE ShiftStatePop(void)
  {
    if (shiftStackPos > 0) {
      shiftStackPos--;
      shiftState = shiftStack[shiftStackPos];
DEBUG
      }
    return (shiftState);
    }


void ShiftStatePush(S_STATE state)
  {
    if (shiftStackPos <= STACK_MAX) {
      shiftStack[shiftStackPos] = shiftState;
      shiftStackPos++;
      if (state != S_KEEP_STATE)
        shiftState = state;
DEBUG
      }
    }


void ShiftStateReset(void)
  {
    if (shiftStackPos > 0) {
      shiftStackPos = 0;
      shiftState = shiftStack[shiftStackPos];
      }
DEBUG
    }


void ShiftStateSet(S_STATE state)
  {
    if (state != S_KEEP_STATE)
      shiftState = state;
DEBUG
    }
