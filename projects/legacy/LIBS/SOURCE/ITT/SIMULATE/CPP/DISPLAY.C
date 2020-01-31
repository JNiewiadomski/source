/* display.c -- February 5, 1990 */

#define DEBUG DisplayShowStack();

/* Display Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   10-JAN-1990
 * ----------------------------------------------------------------------- */

#include <graphics.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>
#include <..\seachest\font.h>
#include <..\seachest\graphics.h>
#include <..\seachest\mouse.h>

#include "display.h"
#include "hardware.h"
#include "led.h"

#define DISP_FONT  "itt.fnt"

#define DISP_FLASH_TIME   75
#define DISP_SCROLL_TIME  25

#define CURSOR_TIME       75

#define DISP_TEXT_SIZE 9

#define CHAR_LEAD   4
#define CHAR_HEIGHT (FONT_HEIGHT + CHAR_LEAD)
#define CHAR_WIDTH  FONT_WIDTH

#define DISP_MAX_STACK  5

#define DISP_POS_TOP(row)  (display.pos.v + ((row) * CHAR_HEIGHT))
#define DISP_POS_LEF(col)  (display.pos.h + ((col) * CHAR_WIDTH))

typedef struct {
  Point   pos;                            /* location of display on screen */
  char    buff[DISP_ROWS][DISP_COLS];     /* display character buffer */
  BOOLEAN fOn[DISP_ROWS];                 /* flash line on ? */
  BOOLEAN fPhase;                         /* display blank phase */
  JIFF    fTimer;                         /* flash countdown timer */
  BOOLEAN sOn[DISP_ROWS];                 /* scroll line on ? */
  JIFF    sTimer;                         /* scroll countdown timer */
  short   sPos[DISP_ROWS];                /* next character to scroll on line */
  char    *sBuff[DISP_ROWS];              /* pointer to characters to scroll on line */
  short   sLen[DISP_ROWS];                /* length of line to scroll */
  } DisplayObject;

typedef struct {
  short row;                  /* row where cursor resides */
  short col;                  /* current cursor position */
  short len;                  /* length of cursor */
  Rect  r;                    /* physical boundary of cursor */
  JIFF  timer;                /* count down till next blink change */
  WORD  on    : 1;            /* cursor turned on ? */
  WORD  blink : 1;            /* cursor is in blink mode ? */
  } CursorObject;


static DisplayObject  display, displayStack[DISP_MAX_STACK];
static short          displayStackPos=0;
static GrafPort       displayPort;
static CursorObject   cursor;


static void CursorCalcRect(void);
static void CursorInit(void);
static void CursorInverse(void);
static void CursorUpdate(void);
static void DisplayDrawChar(short row, short col, BYTE ch);
static void DisplayRedraw(void);
static void DisplayUpdateFlash(void);
static void DisplayUpdateScroll(void);

#ifdef DEBUG
  #include <stdio.h>

  static void DisplayShowStack(void);


  static void DisplayShowStack(void)
    {
      GrafPort *old, new;
      char     buff[30];

      old = GraphicsGetPort();
      GraphicsNewPort(&new);
      GraphicsForeColor(WHITE);
      GraphicsFillColor(BLACK);
      GraphicsTextSize(5);
      sprintf(buff, "Display Stack=%2d", displayStackPos);
      mouseHideCursor();
      bar(0, 20, 150, 40);
      outtextxy(0, 22, buff);
      mouseShowCursor();
      GraphicsSetPort(old);
      }
#endif


static void CursorCalcRect(void)
  {
    short top, lef, bot, rig;

    top = DISP_POS_TOP(cursor.row);
    lef = DISP_POS_LEF(cursor.col);
    bot = top + CHAR_HEIGHT - 1;
    rig = lef + (cursor.len * CHAR_WIDTH) - 1;
    RASG(&cursor.r, top, lef, bot, rig);
    }


void CursorEnable(BOOLEAN on)
  {
    if (on == cursor.on)
      return;
    if (on) {
      cursor.on = TRUE;
      cursor.blink = TRUE;
      cursor.timer = EventJiffy() + CURSOR_TIME;
      }
    else {
      if (cursor.on && !cursor.blink)
        CursorInverse();
      cursor.on = FALSE;
      }
    }


static void CursorInit(void)
  {
    cursor.row = 0;
    cursor.col = 0;
    cursor.len = 1;
    CursorCalcRect();
    cursor.on = FALSE;
    cursor.blink = TRUE;
    }


static void CursorInverse(void)
  {
    mouseHideCursor();
    GraphicsReverse(&cursor.r);
    mouseShowCursor();
    }


void CursorSet(short row, short col, short len)
  {
    BOOLEAN on;

    on = cursor.on;
    CursorEnable(FALSE);
    cursor.row = row;
    cursor.col = col;
    cursor.len = len;
    CursorCalcRect();
    if (on)
      CursorEnable(TRUE);
    }


static void CursorUpdate(void)
  {
    if (!cursor.on)
      return;
    if (EventElapsed(cursor.timer)) {
      cursor.blink = !cursor.blink;
      CursorInverse();
      cursor.timer = EventJiffy() + CURSOR_TIME;
      }
    }


void DisplayCleanUp(void)
  {
    short row;

    for (row=0; row<DISP_ROWS; row++)
      if (display.sBuff[row])
        free(display.sBuff[row]);
    }


void DisplayClear(void)
  {
    GrafPort *old;
    short    top, lef, bot, rig, row;

    memset(display.buff, ' ', DISP_ROWS*DISP_COLS);
    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    top = display.pos.v;
    lef = display.pos.h;
    bot = top + (DISP_ROWS*CHAR_HEIGHT) - 1;
    rig = lef + (DISP_COLS*CHAR_WIDTH) - 1;
    for (row=0; row<=DISP_ROWS; row++)
      display.fOn[row] = FALSE;
    mouseHideCursor();
    bar(lef, top, rig, bot);
    mouseShowCursor();
    GraphicsSetPort(old);
    }


void DisplayClearLine(short row)
  {
    GrafPort *old;
    short    top, lef, bot, rig;

    if (row<0 || row>=DISP_ROWS)
      return;
    memset(display.buff[row], ' ', DISP_COLS);
    if (display.sOn[row]) {
      display.sOn[row] = FALSE;
      free(display.sBuff[row]);
      display.sBuff[row] = NULL;
      }
    display.fOn[row] = FALSE;
    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    top = DISP_POS_TOP(row);
    lef = display.pos.h;
    bot = top + CHAR_HEIGHT - 1;
    rig = lef + (DISP_COLS*CHAR_WIDTH) - 1;
    mouseHideCursor();
    bar(lef, top, rig, bot);
    mouseShowCursor();
    GraphicsSetPort(old);
    }


static void DisplayDrawChar(short row, short col, BYTE ch)
  {
    char buff[2];

    if (ch < 128) {
      buff[0] = ch;
      buff[1] = '\0';
      outtextxy(col, row-3, buff);
      }
    else
      FontDraw(row, col, ch);
    }


void DisplayFill(char ch)
  {
    GrafPort    *old;
    short       top, lef, bot, rig, row, col;
    EventRecord event;

    memset(display.buff, ch, DISP_ROWS*DISP_COLS);
    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    top = display.pos.v;
    lef = display.pos.h;
    bot = top + (DISP_ROWS*CHAR_HEIGHT) - 1;
    rig = lef + (DISP_COLS*CHAR_WIDTH) - 1;
    mouseHideCursor();
    bar(lef, top, rig, bot);
    for (row=0; row<DISP_ROWS; row++) {
      display.fOn[row] = FALSE;
      display.sOn[row] = FALSE;
      if (display.sBuff[row]) {
        free(display.sBuff[row]);
        display.sBuff[row] = NULL;
        }
      top = DISP_POS_TOP(row);
      lef = display.pos.h;
      for (col=0; col<DISP_COLS; col++) {
        DisplayDrawChar(top, lef, ch);
        lef += CHAR_WIDTH;
        }
      EventAvail(EVT_EVERY, &event);
      }
    mouseShowCursor();
    GraphicsSetPort(old);
    }


void DisplayFlashLine(short row, BOOLEAN on)
  {
    if (row<0 || row>=DISP_ROWS || on==display.fOn[row])
      return;
    display.fOn[row] = on;
    if (!on)
      DisplayUpdateFlash();
    }


BOOLEAN DisplayInit(Point *p, short fore, short back)
  {
    GrafPort *old;
    short    row;

    FontInit(fore);
    if (!FontRead(DISP_FONT))
      return (FALSE);
    old = GraphicsGetPort();
    GraphicsNewPort(&displayPort);
    GraphicsForeColor(fore);
    GraphicsFillColor(back);
    GraphicsTextSize(DISP_TEXT_SIZE);
    GraphicsSetPort(old);
    PCPY(&display.pos, p);
    for (row=0; row<DISP_ROWS; row++)
      display.sBuff[row] = NULL;
    display.fPhase = FALSE;
    display.fTimer = 0L;
    display.sTimer = 0L;
    DisplayClear();
    CursorInit();
    return (TRUE);
    }


BOOLEAN DisplayPop(void)
  {
    short row;

    if (displayStackPos < 1)
      return (FALSE);
    for (row=0; row<DISP_ROWS; row++)
      if (display.sBuff[row])
        free(display.sBuff[row]);
    displayStackPos--;
DEBUG
    memmove(&display, &displayStack[displayStackPos], sizeof(DisplayObject));
    DisplayRedraw();
    return (TRUE);
    }


BOOLEAN DisplayPush(void)
  {
    short row;

    if (displayStackPos >= DISP_MAX_STACK)
      return (FALSE);
    memmove(&displayStack[displayStackPos++], &display, sizeof(DisplayObject));
DEBUG
    for (row=0; row<DISP_ROWS; row++) {
      memset(display.buff[row], ' ', DISP_COLS);
      display.sBuff[row] = NULL;
      display.fOn[row] = FALSE;
      display.sOn[row] = FALSE;
      }
    return (FALSE);
    }


static void DisplayRedraw(void)
  {
    GrafPort *old;
    short    top, lef, bot, rig, row, col;
    char     *s;

    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    top = display.pos.v;
    lef = display.pos.h;
    bot = top + (DISP_ROWS*CHAR_HEIGHT) - 1;
    rig = lef + (DISP_COLS*CHAR_WIDTH) - 1;
    mouseHideCursor();
    bar(lef, top, rig, bot);
    for (row=0; row<DISP_ROWS; row++) {
      lef = display.pos.h;
      if (!display.fOn[row] || !display.fPhase) {
        s = display.buff[row];
        for (col=0; col<DISP_COLS; col++) {
          DisplayDrawChar(top, lef, *s++);
          lef += CHAR_WIDTH;
          }
        }
      top += CHAR_HEIGHT;
      }
    mouseShowCursor();
    GraphicsSetPort(old);
    }


void DisplayReset(void)
  {
    short row;

    if (displayStackPos > 0) {
      while (displayStackPos > 0) {
        for (row=0; row<DISP_ROWS; row++)
          if (display.sBuff[row])
            free(display.sBuff[row]);
        displayStackPos--;
        }
      memmove(&display, &displayStack[displayStackPos], sizeof(DisplayObject));
      DisplayRedraw();
      }
DEBUG
    }


void DisplayScrollLineOn(short row, char *text)
  {
    if (row<0 || row>=DISP_ROWS)
      return;
    DisplayClearLine(row);
    if ((display.sBuff[row] = strdup(text)) == NULL)
      return;
    display.sOn[row] = TRUE;
    display.sPos[row] = 0;
    display.sLen[row] = strlen(text);
    }


void DisplayScrollLineOff(short row)
  {
    DisplayClearLine(row);
    }


void DisplayScrollDown(void)
  {
    memmove(display.buff, display.buff[1], (DISP_ROWS-1)*DISP_COLS);
    memset(display.buff, ' ', DISP_COLS);
    memmove(display.fOn, &display.fOn[1], sizeof(BOOLEAN)*(DISP_ROWS-1));
    display.fOn[0] = FALSE;
    memmove(display.sOn, &display.sOn[1], sizeof(BOOLEAN)*(DISP_ROWS-1));
    display.sOn[0] = FALSE;
    memmove(display.sPos, &display.sPos[1], sizeof(short)*(DISP_ROWS-1));
    memmove(display.sBuff, &display.sBuff[1], sizeof(char *)*(DISP_ROWS-1));
    display.sBuff[0] = NULL;
    memmove(display.sLen, &display.sLen[1], sizeof(short)*(DISP_ROWS-1));
    DisplayRedraw();
    }


void DisplayScrollUp(void)
  {
    memmove(display.buff[1], display.buff, (DISP_ROWS-1)*DISP_COLS);
    memset(display.buff[DISP_ROWS-1], ' ', DISP_COLS);
    memmove(&display.fOn[1], display.fOn, sizeof(BOOLEAN)*(DISP_ROWS-1));
    display.fOn[DISP_ROWS-1] = FALSE;
    memmove(&display.sOn[1], display.sOn, sizeof(BOOLEAN)*(DISP_ROWS-1));
    display.sOn[DISP_ROWS-1] = FALSE;
    memmove(&display.sPos[1], display.sPos, sizeof(short)*(DISP_ROWS-1));
    memmove(&display.sBuff[1], display.sBuff, sizeof(char *)*(DISP_ROWS-1));
    display.sBuff[DISP_ROWS-1] = NULL;
    memmove(&display.sLen[1], display.sLen, sizeof(short)*(DISP_ROWS-1));
    DisplayRedraw();
    }


void DisplaySetChar(short row, short col, char ch)
  {
    GrafPort *old;
    short    top, lef;

    if (row<0 || row>=DISP_ROWS || col<0 || col>=DISP_COLS)
      return;
    display.buff[row][col] = ch;
    if (display.fOn[row] && display.fPhase)
      return;
    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    top = DISP_POS_TOP(row);
    lef = DISP_POS_LEF(col);
    mouseHideCursor();
    bar(lef, top, lef+CHAR_WIDTH-1, top+CHAR_HEIGHT-1);
    DisplayDrawChar(top, lef, ch);
    mouseShowCursor();
    GraphicsSetPort(old);
    }


void DisplaySetLine(short row, short col, char *text)
  {
    short    len, top, lef;
    GrafPort *old;

    if (col < 0) {
      text -= col;
      col = 0;
      }
    len = strlen(text);
    if (col+len > DISP_COLS)
      len = DISP_COLS - col;
    if (len<1 || row<0 || row>=DISP_ROWS || col<0 || col>=DISP_COLS)
      return;
    if (display.sOn[row])
      DisplayClearLine(row);
    display.fOn[row] = FALSE;
    memmove(display.buff[row]+col, text, len);
    if (display.fOn[row] && display.fPhase)
      return;
    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    top = DISP_POS_TOP(row);
    lef = DISP_POS_LEF(col);
    mouseHideCursor();
    bar(lef, top, lef+(len*CHAR_WIDTH)-1, top+CHAR_HEIGHT-1);
    for ( ; col<DISP_COLS && len; len--) {
      DisplayDrawChar(top, lef, *text++);
      lef += CHAR_WIDTH;
      }
    mouseShowCursor();
    GraphicsSetPort(old);
    }


void DisplayShowMessage(short line, char *text, JIFF delay)
  {
    JIFF timeOut;

    DisplaySetLine(line, 0, text);
    timeOut = EventJiffy() + delay;
    while (!EventElapsed(timeOut)) {
      HardwareCommunicate();
      LEDUpdate();
      }
    }


void DisplayTestSegments(void)
  {
    short       top, lef, bot, rig, col, ch, curTop, row;
    char        buff[2], temp[DISP_ROWS*CHAR_HEIGHT][(DISP_COLS-1)*CHAR_WIDTH];
    GrafPort    *old;
    EventRecord event;
    BOOLEAN     ok;

    top = display.pos.v;
    lef = display.pos.h;
    bot = top + (DISP_ROWS * CHAR_HEIGHT) - 1;
    rig = lef + (DISP_COLS * CHAR_WIDTH) - 1;
    col = DISP_POS_LEF(DISP_COLS-1);
    buff[1] = '\0';
    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    mouseHideCursor();
    DisplayPush();
    DisplayClear();
    ok = TRUE;
    for (ch=33; ok && ch<128+DISP_COLS; ch++) {
      getimage(lef+CHAR_WIDTH, top, rig, bot, temp);
      putimage(lef, top, temp, COPY_PUT);
      bar(col, top, rig, bot);
      curTop = top;
      buff[0] = ch;
      for (row=0; row<DISP_ROWS; row++) {
        outtextxy(col, curTop, buff);
        curTop += CHAR_HEIGHT;
        }
      if (EventGetNext(EVT_KEY | EVT_MOUSE_DOWN, &event))
        ok = FALSE;
      }
    DisplayPop();
    mouseShowCursor();
    GraphicsSetPort(old);
    }


void DisplayUpdate(void)
  {
    if (EventElapsed(display.fTimer)) {
      display.fTimer = EventJiffy() + DISP_FLASH_TIME;
      display.fPhase = !display.fPhase;
      DisplayUpdateFlash();
      }
    if (EventElapsed(display.sTimer)) {
      display.sTimer = EventJiffy() + DISP_SCROLL_TIME;
      DisplayUpdateScroll();
      }
    CursorUpdate();
    }


static void DisplayUpdateFlash(void)
  {
    GrafPort *old;
    short    row, col, top, lef, bot, rig;
    char     *s;

    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    for (row=0; row<DISP_ROWS; row++)
      if (display.fOn[row]) {
        top = DISP_POS_TOP(row);
        lef = display.pos.h;
        mouseHideCursor();
        if (display.fPhase) {
          bot = top + CHAR_HEIGHT - 1;
          rig = lef + (DISP_COLS*CHAR_WIDTH) - 1;
          bar(lef, top, rig, bot);
          }
        else {
          s = display.buff[row];
          for (col=0; col<DISP_COLS; col++) {
            DisplayDrawChar(top, lef, *s++);
            lef += CHAR_WIDTH;
            }
          }
        mouseShowCursor();
        }
    GraphicsSetPort(old);
    }


static void DisplayUpdateScroll(void)
  {
    GrafPort *old;
    short    row, top, lef, bot, rig, col;
    char     ch, buff[CHAR_HEIGHT][(DISP_COLS-1)*CHAR_WIDTH];

    old = GraphicsGetPort();
    GraphicsSetPort(&displayPort);
    for (row=0; row<DISP_ROWS; row++)
      if (display.sOn[row]) {
        ch = display.sBuff[row][display.sPos[row]++];
        if (display.sPos[row] == display.sLen[row])
          display.sPos[row] = 0;
        memmove(display.buff[row], display.buff[row]+1, DISP_COLS-1);
        display.buff[row][DISP_COLS-1] = ch;
        if (!display.fOn[row] || !display.fPhase) {
          top = DISP_POS_TOP(row);
          lef = display.pos.h;
          bot = top + CHAR_HEIGHT - 1;
          rig = lef + (DISP_COLS * CHAR_WIDTH) - 1;
          mouseHideCursor();
          getimage(lef+CHAR_WIDTH, top, rig, bot, buff);
          col = DISP_POS_LEF(DISP_COLS-1);
          bar(col, top, rig, bot);
          putimage(lef, top, buff, COPY_PUT);
          DisplayDrawChar(top, col, ch);
          mouseShowCursor();
          }
        }
    GraphicsSetPort(old);
    }
