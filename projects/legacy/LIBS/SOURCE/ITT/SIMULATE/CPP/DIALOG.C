/* dialog.c -- February 5, 1990 */

/* Dialog Library */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>

#include "dialog.h"
#include "display.h"
#include "state.h"

typedef enum { D_NONE, D_NUM, D_CODE, D_LIST } D_STATE;

typedef struct {
  D_STATE    state;                     /* dialog state */
  short      row;                       /* row to echo data entered */
  short      col;                       /* cursor position */
  short      len;                       /* length of input expected */
  short      num;                       /* number of characters entered */
  short      pos;                       /* current position in alarm */
  char       val[DISP_COLS+1];          /* value entered by user */
  dialogFunc func;                      /* function to call when done */
  WORD       inEntry : 1;               /* ENTER pressed to enter data ? */
  WORD       dirty   : 1;               /* have any values been entered ? */
  WORD       dflt;                      /* default data */
  WORD       loLimit;                   /* minimum value required */
  WORD       hiLimit;                   /* maximum value required */
  char       **choice;                  /* list of choices available */
  } DialogRecord;

static DialogRecord theDialog = { D_NONE, 0, 0, 0, 0, { '\0' }, NULL };

static void DialogClearCode(void);
static void DialogClearList(void);
static void DialogClearNum(void);
static void DialogDoCode(short ch);
static void DialogDoList(short ch);
static void DialogDoNum(short ch);


void DialogClear(void)
  {
    switch (theDialog.state) {
      case D_CODE:
        DialogClearCode();
        break;
      case D_LIST:
        DialogClearList();
        break;
      case D_NUM:
        DialogClearNum();
        break;
      }
    }


static void DialogClearCode(void)
  {
    short i;

    if (theDialog.state == D_NONE || !theDialog.inEntry)
      return;
    CursorEnable(FALSE);
    theDialog.col -= theDialog.num;
    theDialog.num = 0;
    theDialog.val[0] = '\0';
    for (i=0; i<theDialog.len; i++)
      DisplaySetChar(theDialog.row, theDialog.col+i, '_');
    CursorSet(theDialog.row, theDialog.col, 1);
    CursorEnable(TRUE);
    }


static void DialogClearNum(void)
  {
    short i;

    if (theDialog.state == D_NONE)
      return;
    if (!theDialog.inEntry) {
      if (theDialog.func)
        (*theDialog.func)(K_CLEAR);
      return;
      }
    CursorEnable(FALSE);
    if (theDialog.dirty) {
      sprintf(theDialog.val, "%u", theDialog.dflt);
      theDialog.num = strlen(theDialog.val);
      CursorSet(theDialog.row, theDialog.col+theDialog.len-theDialog.num, theDialog.num);
      theDialog.dirty = FALSE;
      }
    else {
      theDialog.num = 0;
      theDialog.val[0] = '\0';
      CursorSet(theDialog.row, theDialog.col+theDialog.len-1, 1);
      theDialog.dirty = TRUE;
      }
    for (i=0; i<theDialog.len; i++)
      DisplaySetChar(theDialog.row, theDialog.col+i, '_');
    DisplaySetLine(theDialog.row, theDialog.col+theDialog.len-theDialog.num, theDialog.val);
    CursorEnable(TRUE);
    }


static void DialogClearList(void)
  {
    WORD val;
    char buff[DISP_COLS+1];

    if (theDialog.state == D_NONE)
      return;
    if (!theDialog.inEntry) {
      if (theDialog.func)
        (*theDialog.func)(K_CLEAR);
      return;
      }
    if (theDialog.dirty) {
      val = theDialog.dflt;
      theDialog.dirty = FALSE;
      }
    else {
      val = 1;
      theDialog.dirty = TRUE;
      }
    sprintf(theDialog.val, "%u", val);
    CursorEnable(FALSE);
    sprintf(buff, "%*s", theDialog.len, theDialog.choice[val]);
    DisplaySetLine(theDialog.row, theDialog.col, buff);
    CursorEnable(TRUE);
    }


void DialogDo(short ch)
  {
    switch (theDialog.state) {
      case D_CODE:
        DialogDoCode(ch);
        break;
      case D_LIST:
        DialogDoList(ch);
        break;
      case D_NUM:
        DialogDoNum(ch);
        break;
      }
    }


static void DialogDoCode(short ch)
  {
    if (!theDialog.inEntry) {
      if (ch == K_RETURN) {
        CursorSet(theDialog.row, theDialog.col, 1);
        CursorEnable(TRUE);
        theDialog.inEntry = TRUE;
        ShiftStateSet(S_NUMBER);
        }
      else if (theDialog.func)
        (*theDialog.func)(ch);
      return;
      }
    if (ch == K_RETURN) {
      DialogStop();
      CursorEnable(FALSE);
      if (theDialog.func)
        (*theDialog.func)(ch);
      }
    else if (theDialog.num < theDialog.len) {
      CursorEnable(FALSE);
      theDialog.val[theDialog.num++] = ch;
      theDialog.val[theDialog.num] = '\0';
      DisplaySetChar(theDialog.row, theDialog.col++, '*');
      if (theDialog.num < theDialog.len) {
        CursorSet(theDialog.row, theDialog.col, 1);
        CursorEnable(TRUE);
        }
      }
    }


static void DialogDoNum(short ch)
  {
    DWORD val;
    short i;
    char  buff[DISP_COLS+1];

    if (!theDialog.inEntry) {
      if (ch == K_RETURN) {
        sprintf(theDialog.val, "%u", theDialog.dflt);
        theDialog.num = strlen(theDialog.val);
        CursorSet(theDialog.row, theDialog.col+theDialog.len-theDialog.num, theDialog.num);
        CursorEnable(TRUE);
        theDialog.inEntry = TRUE;
        ShiftStateSet(S_NUMBER);
        }
      else if (theDialog.func)
        (*theDialog.func)(ch);
      return;
      }
    if (ch == K_RETURN) {
      val = (DWORD)atol(theDialog.val);
      if (theDialog.dirty && (val < theDialog.loLimit || val > theDialog.hiLimit)) {
        CursorEnable(FALSE);
        DisplayPush();
        DisplaySetLine(DISP_LINE1, 0, "Value Out Of Range  ");
        sprintf(buff, "%u<=value<=%u", theDialog.loLimit, theDialog.hiLimit);
        DisplayClearLine(DISP_LINE2);
        DisplaySetLine(DISP_LINE2, 0, buff);
        EventDelay(400);
        DisplayPop();
        for (i=0; i<theDialog.len; i++)
          DisplaySetChar(theDialog.row, theDialog.col+i, '_');
        sprintf(theDialog.val, "%u", theDialog.dflt);
        DisplaySetLine(theDialog.row, theDialog.col+theDialog.len-strlen(theDialog.val), theDialog.val);
        theDialog.dirty = FALSE;
        CursorEnable(TRUE);
        }
      else {
        DialogStop();
        CursorEnable(FALSE);
        sprintf(theDialog.val, "%u", theDialog.dirty ? (WORD)val : theDialog.dflt);
        if (theDialog.func)
          (*theDialog.func)(ch);
        }
      }
    else if (ch >= '0' && ch <= '9') {
      CursorEnable(FALSE);
      if (theDialog.dirty) {
        if (theDialog.num == theDialog.len) {
          memmove(theDialog.val, theDialog.val+1, theDialog.len-1);
          theDialog.num--;
          }
        }
      else {
        CursorSet(theDialog.row, theDialog.col+theDialog.len-1, 1);
        for (i=0; i<theDialog.len; i++)
          DisplaySetChar(theDialog.row, theDialog.col+i, '_');
        theDialog.val[0] = '\0';
        theDialog.num = 0;
        theDialog.dirty = TRUE;
        }
      theDialog.val[theDialog.num++] = ch;
      theDialog.val[theDialog.num] = '\0';
      DisplaySetLine(theDialog.row, theDialog.col+theDialog.len-theDialog.num, theDialog.val);
      CursorEnable(TRUE);
      }
    }


static void DialogDoList(short ch)
  {
    WORD val;
    char buff[DISP_COLS+1];

    if (!theDialog.inEntry) {
      if (ch == K_RETURN) {
        sprintf(theDialog.val, "%u", theDialog.dflt);
        CursorSet(theDialog.row, theDialog.col, theDialog.len);
        CursorEnable(TRUE);
        ShiftStateSet(S_NUMBER);
        theDialog.inEntry = TRUE;
        }
      else if (theDialog.func)
        (*theDialog.func)(ch);
      return;
      }
    switch (ch) {
      case K_RETURN:
        DialogStop();
        CursorEnable(FALSE);
        if (theDialog.func)
          (*theDialog.func)(ch);
        break;
      case '3':   /* up arrow */
        val = atoi(theDialog.val) + 1;
        if (val > theDialog.num)
          val = 1;
        sprintf(theDialog.val, "%u", val);
        sprintf(buff, "%*s", theDialog.len, theDialog.choice[val]);
        CursorEnable(FALSE);
        DisplaySetLine(theDialog.row, theDialog.col, buff);
        CursorEnable(TRUE);
        theDialog.dirty = TRUE;
        break;
      case '9':   /* down arrow */
        val = atoi(theDialog.val) - 1;
        if ((short)val < 1)
          val = theDialog.num;
        sprintf(theDialog.val, "%u", val);
        sprintf(buff, "%*s", theDialog.len, theDialog.choice[val]);
        CursorEnable(FALSE);
        DisplaySetLine(theDialog.row, theDialog.col, buff);
        CursorEnable(TRUE);
        theDialog.dirty = TRUE;
        break;
      }
    }


char *DialogGetVal(void)
  {
    return (theDialog.val);
    }


void DialogStartCode(short row, short col, short len, dialogFunc func, WORD lo, WORD hi)
  {
    short i;

    theDialog.state = D_CODE;
    theDialog.row = row;
    theDialog.col = col;
    theDialog.len = len;
    theDialog.num = 0;
    theDialog.val[0] = '\0';
    theDialog.func = func;
    theDialog.inEntry = FALSE;
    theDialog.loLimit = lo;
    theDialog.hiLimit = hi;
    for (i=0; i<theDialog.len; i++)
      DisplaySetChar(theDialog.row, theDialog.col+i, '_');
    ShiftStateSet(S_SELECT);
    }


void DialogStartNum(short row, short col, short len, dialogFunc func, WORD dflt, WORD lo, WORD hi)
  {
    short i;

    theDialog.state = D_NUM;
    theDialog.row = row;
    theDialog.col = col;
    theDialog.len = len;
    theDialog.num = 0;
    theDialog.val[0] = '\0';
    theDialog.func = func;
    theDialog.inEntry = FALSE;
    theDialog.dirty = FALSE;
    theDialog.dflt = dflt;
    theDialog.loLimit = lo;
    theDialog.hiLimit = hi;
    ShiftStateSet(S_SELECT);
    for (i=0; i<theDialog.len; i++)
      DisplaySetChar(theDialog.row, theDialog.col+i, '_');
    sprintf(theDialog.val, "%u", dflt);
    DisplaySetLine(theDialog.row, col+len-strlen(theDialog.val), theDialog.val);
    }


void DialogStartList(short row, short col, dialogFunc func, WORD dflt, short num, char **list)
  {
    short i;
    char  buff[DISP_COLS+1];

    theDialog.state = D_LIST;
    theDialog.row = row;
    theDialog.col = col;
    theDialog.num = num;
    sprintf(theDialog.val, "%u", dflt);
    theDialog.func = func;
    theDialog.dirty = FALSE;
    theDialog.dflt = dflt;
    theDialog.inEntry = FALSE;
    theDialog.choice = list;
    theDialog.len = 0;
    for (i=1; i<=num; i++)
      theDialog.len = max(theDialog.len, strlen(theDialog.choice[i]));
    sprintf(buff, "%*s", theDialog.len, theDialog.choice[dflt]);
    DisplaySetLine(theDialog.row, col, buff);
    ShiftStateSet(S_SELECT);
    }


void DialogStop(void)
  {
    theDialog.state = D_NONE;
    }
