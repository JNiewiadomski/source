/* button.c -- February 5, 1989 */

/* Button Object Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   27-NOV-1989
 * ----------------------------------------------------------------------- */

#include <dos.h>
#include <graphics.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\mouse.h>
#include <..\seachest\graphics.h>

#include "led.h"
#include "button.h"
#include "text.h"

#define MAX_BUTTON 50

typedef struct {
  Rect       pos;
  short      color;
  short      led;
  short      text;
  buttonFunc func;
  } BUTTON_OBJECT;


short buttonPressed = BUTTON_NONE;

static BUTTON_OBJECT *listButton[MAX_BUTTON];
static short         numButton=0;


void ButtonAddFunc(short index, buttonFunc func)
  {
    BUTTON_OBJECT *button;

    if (index < 0 || index >= numButton)
      return;
    button = listButton[index];
    button->func = func;
    }


void ButtonAddLED(short index, Point *p, short radius, L_STATE state)
  {
    BUTTON_OBJECT *button;
    Point         newP;

    if (index < 0 || index >= numButton)
      return;
    button = listButton[index];
    newP.v = button->pos.p.top + p->v;
    newP.h = button->pos.p.rig - p->h;
    button->led = LEDCreate(&newP, radius, state);
    }


void ButtonAddText(short index, Point *p, short color, short size, char *text)
  {
    BUTTON_OBJECT *button;
    Point         newP;

    if (index < 0 || index >= numButton)
      return;
    button = listButton[index];
    PCPY(&newP, &(button->pos.w.topLef));
    newP.v += p->v;
    newP.h += p->h;
    button->text = TextCreate(&newP, color, size, text);
    }


short ButtonCreate(Rect *pos, short color, short led, short text, buttonFunc func)
  {
    BUTTON_OBJECT *new;

    if (numButton >= MAX_BUTTON)
      return (BUTTON_BAD);
    if ((new = (BUTTON_OBJECT *) malloc(sizeof(BUTTON_OBJECT))) == NULL)
      return (BUTTON_BAD);
    RCPY(&(new->pos), pos);
    new->color = color;
    new->led = led;
    new->text = text;
    new->func = func;
    listButton[numButton] = new;
    return (numButton++);
    }


void ButtonDown(short index)
  {
    BUTTON_OBJECT *button;
    int           i;

    if (index < 0 || index >= numButton)
      return;
    button = listButton[index];
    mouseHideCursor();
    GraphicsReverse(&(button->pos));
    mouseShowCursor();
    sound(400);
    for (i=0; i<7000; i++)
      ;
    nosound();
    buttonPressed = index;
    if (button->func)
      (button->func)();
    }


void ButtonDraw(short index)
  {
    BUTTON_OBJECT *button;

    if (index < 0 || index >= numButton)
      return;
    button = listButton[index];
    setfillstyle(SOLID_FILL, button->color);
    bar(button->pos.p.lef, button->pos.p.top, button->pos.p.rig, button->pos.p.bot);
    if (button->led != LED_BAD)
      LEDDraw(button->led);
    if (button->text != TEXT_BAD)
      TextDraw(button->text);
    }


void ButtonDrawAll(void)
  {
    BUTTON_OBJECT *button;
    short         i;

    for (i=0; i<numButton; i++) {
      button = listButton[i];
      setfillstyle(SOLID_FILL, button->color);
      bar(button->pos.p.lef, button->pos.p.top, button->pos.p.rig, button->pos.p.bot);
      if (button->led != LED_BAD)
        LEDDraw(button->led);
      if (button->text != TEXT_BAD)
        TextDraw(button->text);
      }
    }


short ButtonFind(Point *p)
  {
    short i;

    for (i=0; i<numButton; i++)
      if (GraphicsInRect(&(listButton[i]->pos), p))
        return (i);
    return (BUTTON_BAD);
    }


BOOLEAN ButtonReleased(Point *p)
  {
    if (buttonPressed == BUTTON_NONE)
      return (FALSE);
    return (!GraphicsInRect(&(listButton[buttonPressed]->pos), p));
    }


void ButtonSetLED(short index, L_STATE state)
  {
    BUTTON_OBJECT *button;

    if (index < 0 || index >= numButton)
      return;
    button = listButton[index];
    LEDSet(button->led, state);
    mouseHideCursor();
    LEDDraw(button->led);
    mouseShowCursor();
    }


BOOLEAN ButtonStillDown(short index)
  {
    return (buttonPressed == index);
    }


void ButtonUp(void)
  {
    short i;

    if (buttonPressed == BUTTON_NONE)
      return;
    sound(300);
    for (i=0; i<7000; i++)
      ;
    nosound();
    mouseHideCursor();
    ButtonDraw(buttonPressed);
    mouseShowCursor();
    buttonPressed = BUTTON_NONE;
    }
