/* setpoint.c -- February 5, 1990 */

/* Set Point Library */

#include <stdio.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>

#include "dialog.h"
#include "display.h"
#include "hardware.h"
#include "setpoint.h"
#include "state.h"


#define PREV_ZONE(x) ((x)==1 ? numZones : ((x)-1))
#define NEXT_ZONE(x) ((x)==numZones ? 1 : ((x)+1))

static WORD numZones, curZone;


static void DoSetPointDisplay(void);
static void DoSetPointChange(short ch);


void SetPointChange(void)
  {
    if ((numZones = HardwareReadRAM(R_NUM_ZONES, 0)) < 1)
      return;
    curZone = 1;
    DisplayPush();
    ShiftStatePush(S_KEEP_STATE);
    DoSetPointDisplay();
    }


static void DoSetPointDisplay(void)
  {
    WORD zone, sp, max;
    char buff[DISP_COLS+1];

    zone = PREV_ZONE(curZone);
    if (zone != curZone) {
      sp = HardwareReadRAM(R_ZONE_01_SP, zone-1);
      sprintf(buff, "Zone%2u:    SP = %3u ", zone, sp);
      DisplaySetLine(DISP_LINE1, 0, buff);
      }
    else
      DisplayClearLine(DISP_LINE1);
    sp = HardwareReadRAM(R_ZONE_01_SP, curZone-1);
    sprintf(buff, "Zone%2u:    SP = %3u ", curZone, sp);
    DisplaySetLine(DISP_LINE2, 0, buff);
    max = HardwareReadRAM(R_PV_TRANS_SPAN, 0);
    DialogStartNum(DISP_LINE2, 16, 3, DoSetPointChange, sp, 0U, max);
    }


static void DoSetPointChange(short ch)
  {
    WORD val;

    switch (ch) {
      case K_UP:
        curZone = PREV_ZONE(curZone);
        DoSetPointDisplay();
        break;
      case K_DOWN:
        curZone = NEXT_ZONE(curZone);
        DoSetPointDisplay();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_ZONE_01_SP, curZone-1, val);
        curZone = NEXT_ZONE(curZone);
        DoSetPointDisplay();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }
