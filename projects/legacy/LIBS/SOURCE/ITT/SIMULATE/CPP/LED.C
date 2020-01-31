/* led.c -- February 5, 1990 */

/* LED Object Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   05-JAN-1990
 * ----------------------------------------------------------------------- */

#include <graphics.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>
#include <..\seachest\mouse.h>

#include "led.h"
#include "hardware.h"

#define MAX_LED  50

#define LED_RATE 75

#define LED_FLASH(x) (((x)==L_FL_RED) || ((x)==L_FL_GREEN))

typedef struct {
  Point   pos;
  short   radius;
  L_STATE state;
  } LED_OBJECT;

static short       offLED, on1LED, on2LED, updateLED[MAX_LED];
static short       numLED=0, numUpdateLED=0;
static LED_OBJECT *listLED[MAX_LED];

static JIFF        flashTimer=0L;
static BOOLEAN     flashLED=FALSE;


static L_STATE LEDHardwareState(WORD val, short pos);


short LEDCreate(Point *pos, short radius, L_STATE state)
  {
    LED_OBJECT *new;

    if (numLED >= MAX_LED)
      return (LED_BAD);
    if ((new = (LED_OBJECT *) malloc(sizeof(LED_OBJECT))) == NULL)
      return (LED_BAD);
    PCPY(&(new->pos), pos);
    new->radius = radius;
    new->state = state;
    listLED[numLED] = new;
    if (LED_FLASH(state))
      updateLED[numUpdateLED] = numLED;
    return (numLED++);
    }


void LEDDraw(short index)
  {
    LED_OBJECT *led;

    if (index < 0 || index >= numLED)
      return;
    led = listLED[index];
    switch (led->state) {
      case L_RED:
        setfillstyle(SOLID_FILL, on1LED);
        break;
      case L_GREEN:
        setfillstyle(SOLID_FILL, on2LED);
        break;
      case L_FL_RED:
        setfillstyle(SOLID_FILL, flashLED ? offLED : on1LED);
        break;
      case L_FL_GREEN:
        setfillstyle(SOLID_FILL, flashLED ? offLED : on2LED);
        break;
      default: L_OFF:
        setfillstyle(SOLID_FILL, offLED);
        break;
      }
    setcolor(BLACK);
    fillellipse(led->pos.h, led->pos.v, led->radius, led->radius);
    }


void LEDDrawAll(void)
  {
    short      index;
    LED_OBJECT *led;

    mouseHideCursor();
    for (index=0; index<numLED; index++) {
      led = listLED[index];
      switch (led->state) {
        case L_RED:
          setfillstyle(SOLID_FILL, on1LED);
          break;
        case L_GREEN:
          setfillstyle(SOLID_FILL, on2LED);
          break;
        case L_FL_RED:
          setfillstyle(SOLID_FILL, flashLED ? offLED : on1LED);
          break;
        case L_FL_GREEN:
          setfillstyle(SOLID_FILL, flashLED ? offLED : on2LED);
          break;
        default: L_OFF:
          setfillstyle(SOLID_FILL, offLED);
          break;
        }
      setcolor(BLACK);
      fillellipse(led->pos.h, led->pos.v, led->radius, led->radius);
      }
    mouseShowCursor();
    }


static L_STATE LEDHardwareState(WORD val, short pos)
  {
    WORD state;

    state = (val >> ((pos-1)*4)) & 0x0F;
    if (state & 0x0001)
      return (L_FL_GREEN);
    else if (state & 0x0002)
      return (L_GREEN);
    else if (state & 0x0004)
      return (L_RED);
    else if (state & 0x0008)
      return (L_FL_RED);
    return (L_OFF);
    }


void LEDInit(short off, short on1, short on2)
  {
    offLED = off;
    on1LED = on1;
    on2LED = on2;
    }


void LEDSet(short index, L_STATE state)
  {
    LED_OBJECT *led;
    short      i;

    if (index < 0 || index >= numLED)
      return;
    led = listLED[index];
    switch (state) {
      case L_NEXT:
        state = led->state == L_FL_GREEN ? L_OFF : led->state+1;
        break;
      case L_PREV:
        state = led->state == L_OFF ? L_FL_GREEN : led->state+1;
        break;
      }
    if (state == led->state)
      return;
    if (LED_FLASH(led->state)) {
      if (!LED_FLASH(state)) {
        for (i=0; i<numUpdateLED && updateLED[i]!=index; i++)
          ;
        numUpdateLED--;
        if (i < numUpdateLED)
          updateLED[i] = updateLED[numUpdateLED];
        }
      }
    else if (LED_FLASH(state))
      updateLED[numUpdateLED++] = index;
    led->state = state;
    }


void LEDSetAll(L_STATE state)
  {
    LED_OBJECT *led;
    short      index, i;

    for (index=0; index<numLED; index++) {
      led = listLED[index];
      switch (state) {
        case L_NEXT:
          state = led->state == L_FL_GREEN ? L_OFF : led->state+1;
          break;
        case L_PREV:
          state = led->state == L_OFF ? L_FL_GREEN : led->state+1;
          break;
        }
      if (state == led->state)
        return;
      if (LED_FLASH(led->state)) {
        if (!LED_FLASH(state)) {
          for (i=0; i<numUpdateLED && updateLED[i]!=index; i++)
            ;
          numUpdateLED--;
          if (i < numUpdateLED)
            updateLED[i] = updateLED[numUpdateLED];
          }
        }
      else if (LED_FLASH(state))
        updateLED[numUpdateLED++] = index;
      led->state = state;
      }
    }


void LEDUpdate(void)
  {
    short i;
    WORD  val, numPumps;

    if (EventElapsed(flashTimer)) {
      flashLED = !flashLED;
      if (numUpdateLED) {
        mouseHideCursor();
        for (i=0; i<numUpdateLED; i++)
          LEDDraw(updateLED[i]);
        mouseShowCursor();
        }
      flashTimer = EventJiffy() + LED_RATE;
      }
    if (hwUpdate.led) {
      numPumps = HardwareReadRAM(R_NUM_PUMPS, 0);
      val = HardwareReadRAM(R_LEDA_STATUS, 0);
      LEDSet(0, LEDHardwareState(val, 1));
      LEDSet(1, LEDHardwareState(val, 2));
      if (numPumps > 0) {
        LEDSet(2, LEDHardwareState(val, 3));
        if (numPumps > 1)
          LEDSet(3, LEDHardwareState(val, 4));
        }
      if (numPumps > 2) {
        val = HardwareReadRAM(R_LEDB_STATUS, 0);
        LEDSet(4, LEDHardwareState(val, 1));
        if (numPumps > 3) {
          LEDSet(5, LEDHardwareState(val, 2));
          if (numPumps > 4) {
            LEDSet(6, LEDHardwareState(val, 3));
            if (numPumps > 5)
              LEDSet(7, LEDHardwareState(val, 4));
            }
          }
        }
      val = HardwareReadRAM(R_LEDC_STATUS, 0);
      LEDSet(8, LEDHardwareState(val, 1));
      LEDSet(9, LEDHardwareState(val, 2));
      LEDDrawAll();
      hwUpdate.led = FALSE;
      }
    }
