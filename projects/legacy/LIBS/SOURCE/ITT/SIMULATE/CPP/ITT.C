/* itt.c -- March 8, 1990 */

#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>
#include <..\seachest\graphics.h>
#include <..\seachest\menu.h>
#include <..\seachest\mouse.h>

#include "about.h"
#include "alarm.h"
#include "display.h"
#include "led.h"
#include "button.h"
#include "dialog.h"
#include "hardware.h"
#include "panel.h"
#include "state.h"
#include "test.h"

#define MENU_ABOUT 0
#define MENU_RESET 1
#define MENU_QUIT  2

#define DEFAULT_NUM 6

typedef enum { M_NONE,
               M_ALT,
               M_BYPASS,
               M_MANUAL,
               M_SETUP,
               M_START_UP
               } M_STATE;

typedef struct {
  WORD activeSP;
  WORD activePV;
  WORD activeZone;
  WORD speedVal;
  WORD gpm;
  WORD kw;
  WORD altSeq;
  } OldRecord;

static short     defaultState=DEFAULT_NUM;
static JIFF      defaultTimer=0L;
static HS_STATE  xmittingState;
static M_STATE   mState=M_NONE;
static OldRecord oldVals;

static void    CleanUp(void);
static void    drawScreen(void);
static void    powerUp(void);
static void    TaskUpdate(void);
static void    UpdateDefaultMessages(void);
static BOOLEAN UpdateDefault0(short line, BOOLEAN force);
static BOOLEAN UpdateDefault1(short line, BOOLEAN force);
static BOOLEAN UpdateDefault2(short line, BOOLEAN force);
static BOOLEAN UpdateDefault3(short line, BOOLEAN force);
static BOOLEAN UpdateDefault4(short line, BOOLEAN force);
static BOOLEAN UpdateDefault5(short line, BOOLEAN force);
static void    UpdateXmittingState(void);


int main(int argc, char **argv)
  {
    BOOLEAN     done;
    EventRecord theEvent;
    short       i, lastButton;

    GraphicsInit();
    EventInit(HardwareCommunicate);
    i = (argc == 2 && *argv[1] == '2') ? 2 : 1;
    HardwareInit(i);
    LEDInit(BLACK, RED, GREEN);
    drawScreen();
    powerUp();
    mouseShowCursor();
    done = FALSE;
    while (!done) {
      TaskUpdate();
      if (EventGetNext(EVT_EVERY, &theEvent)) {
        switch (theEvent.what) {
          case EVT_KEY:
            switch (theEvent.message) {
              case K_ESC:
                done = TRUE;
                break;
              default:
                DialogDo(theEvent.message);
                break;
              }
            break;
          case EVT_MOUSE_DOWN:
            if (theEvent.message == MSG_MOUSE_LEFT) {
              if ((lastButton = ButtonFind(&theEvent.where)) != BUTTON_BAD)
                ButtonDown(lastButton);
              }
            else
              switch (MenuDo(&theEvent.where)) {
                case MENU_ABOUT:
                  DoAbout();
                  break;
                case MENU_RESET:
                  ShiftStateReset();
                  DisplayReset();
                  break;
                case MENU_QUIT:
                  done = TRUE;
                  break;
                }
            break;
          case EVT_MOUSE_UP:
            if (theEvent.message == MSG_MOUSE_LEFT) {
              ButtonUp();
              PanelButtonUp();
              }
            break;
          }
        }
      else if (ButtonReleased(&theEvent.where)) {
        ButtonUp();
        PanelButtonUp();
        }
      }
    CleanUp();
    return (EXIT_SUCCESS);
    }


static void CleanUp(void)
  {
    HardwareCleanUp();
    EventCleanUp();
    GraphicsCleanUp();
    }


static void drawScreen(void)
  {
    if (!PanelDraw()) {
      CleanUp();
      exit(EXIT_FAILURE);
      }
    MenuInit(YELLOW, BLUE);
    MenuAddItem("About ITT...");
    MenuAddItem("Reset State");
    MenuAddItem("Quit");
    }


static void powerUp(void)
  {
    EventRecord event;
    JIFF        timer;

    DoAbout();
    DisplaySetLine(DISP_LINE1, 0, " ITT Bell & Gossett ");
    DisplaySetLine(DISP_LINE2, 0, "  Powersav System   ");
    timer = EventJiffy() + 200L;
    while (!EventElapsed(timer) && !EventAvail(EVT_EVERY, &event))
      ;
    DisplaySetLine(DISP_LINE1, 0, "    Technologic     ");
    DisplaySetLine(DISP_LINE2, 0, "  Pump Controller   ");
    timer = EventJiffy() + 200L;
    while (!EventElapsed(timer) && !EventAvail(EVT_EVERY, &event))
      ;
    DisplaySetLine(DISP_LINE1, 0, "   Copyright 1989   ");
    DisplaySetLine(DISP_LINE2, 0, "All Rights Reserved ");
    timer = EventJiffy() + 200L;
    while (!EventElapsed(timer) && !EventAvail(EVT_EVERY, &event))
      ;
    if (!EventGetNext(EVT_EVERY, &event)) {
      TestLEDs();
      DisplayClear();
      DisplayTestSegments();
      }
    DisplayClear();
    xmittingState = HS_CONNECT;
    }


static void TaskUpdate(void)
  {
    S_STATE state;

    DisplayUpdate();
    LEDUpdate();
    PanelUpdate();
    UpdateXmittingState();
    if (xmittingState != HS_CONNECT) {
      mState = M_NONE;
      return;
      }
    AlarmStatus();
    state = ShiftStateGet();
    if (state != S_OFF && state != S_ALARM) {
      mState = M_NONE;
      return;
      }
    if (!(HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0400)) {
      if (state==S_OFF && mState!=M_SETUP) {
        DisplaySetLine(DISP_LINE1, 0, " Please Do 'Setup'  ");
        DisplaySetLine(DISP_LINE2, 0, "    No User Data    ");
        mState = M_SETUP;
        }
      return;
      }
    UpdateDefaultMessages();
    }


static void UpdateDefaultMessages(void)
  {
    BOOLEAN ok, force;
    short   numLine, defaultNumLines, curState;
    WORD    val;
    char    buff[DISP_COLS+1];

    if (ShiftStateGet() == S_ALARM) {
      defaultNumLines = 1;
      mState = M_NONE;
      }
    else {
      val = HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x080D;
      if (val) {
        defaultNumLines = 1;
        if (val & 0x0001) {
          if (mState != M_ALT)
            DisplaySetLine(DISP_LINE2, 0, "Alt Engaged-Stand By");
          mState = M_ALT;
          }
        else if (val & 0x0004) {
          if (mState != M_BYPASS)
            DisplaySetLine(DISP_LINE2, 0, "System in BYPASS    ");
          mState = M_BYPASS;
          }
        else if (val & 0x0008) {
          if (mState != M_MANUAL) {
            sprintf(buff, "Manual Speed = %3u %%", HardwareReadRAM(R_SPEED_VALUE, 0));
            DisplaySetLine(DISP_LINE2, 0, buff);
            }
          mState = M_MANUAL;
          }
        else {
          if (mState != M_START_UP)
            DisplaySetLine(DISP_LINE2, 0, "Start Up - Stand By ");
          mState = M_START_UP;
          }
        }
      else {
        defaultNumLines = 2;
        if (defaultState % 2 == 1)
          defaultState = 0;
        mState = M_NONE;
        }
      }
    force = FALSE;
    if (EventElapsed(defaultTimer)) {
      defaultTimer = EventJiffy() + 500;
      defaultState += defaultNumLines;
      if (defaultState > 5)
        defaultState = 0;
      force = TRUE;
      }
    else if (!hwUpdate.write)
      return;
    ok = FALSE;
    while (!ok) {
      curState = defaultState;
      for (numLine=DISP_LINE1; numLine<defaultNumLines; numLine++) {
        switch (curState) {
          case 0:
            ok |= UpdateDefault0(numLine, force);
            break;
          case 1:
            ok |= UpdateDefault1(numLine, force);
            break;
          case 2:
            ok |= UpdateDefault2(numLine, force);
            break;
          case 3:
            ok |= UpdateDefault3(numLine, force);
            break;
          case 4:
            ok |= UpdateDefault4(numLine, force);
            break;
          case 5:
            ok |= UpdateDefault5(numLine, force);
            break;
          }
        curState++;
        if (curState >= DEFAULT_NUM)
          curState = 0;
        }
      if (!ok)
        defaultState = curState;
      }
    hwUpdate.write = FALSE;
    }


static BOOLEAN UpdateDefault0(short line, BOOLEAN force)
  {
    WORD activeSP;
    char buff[DISP_COLS+1], *s;

    activeSP = HardwareReadRAM(R_ACTIVE_SP, 0);
    if (!force && activeSP==oldVals.activeSP)
      return (TRUE);
    oldVals.activeSP = activeSP;
    sprintf(buff, " Setpoint = %3u     ", activeSP);
    s = buff + 16;
    switch (HardwareReadRAM(R_PV_UNITS, 0)) {
      case 1:
        sprintf(s, "PSI‚");
        break;
      case 2:
        sprintf(s, "PSI");
        break;
      case 3:
        sprintf(s, "ƒF");
        break;
      case 4:
        sprintf(s, "‚ƒF");
        break;
      case 5:
        sprintf(s, "ƒC");
        break;
      case 6:
        sprintf(s, "‚ƒC");
        break;
      default:
        sprintf(s, "????");
        break;
      }
    DisplaySetLine(line, 0, buff);
    return (TRUE);
    }


static BOOLEAN UpdateDefault1(short line, BOOLEAN force)
  {
    WORD activePV;
    char buff[DISP_COLS+1];

    activePV = HardwareReadRAM(R_ACTIVE_PV, 0);
    if (!force && activePV==oldVals.activePV)
      return (TRUE);
    oldVals.activePV = activePV;
    sprintf(buff, "Process Var.  =  %3u", activePV);
    DisplaySetLine(line, 0, buff);
    return (TRUE);
    }


static BOOLEAN UpdateDefault2(short line, BOOLEAN force)
  {
    WORD activeZone;
    char buff[DISP_COLS+1];

    activeZone = HardwareReadRAM(R_ACTIVE_ZONE, 0);
    if (!force && activeZone==oldVals.activeZone)
      return (TRUE);
    oldVals.activeZone = activeZone;
    sprintf(buff, " Active Zone  =   %2u", activeZone);
    DisplaySetLine(line, 0, buff);
    return (TRUE);
    }


static BOOLEAN UpdateDefault3(short line, BOOLEAN force)
  {
    WORD speedVal;
    char buff[DISP_COLS+1];

    speedVal = HardwareReadRAM(R_SPEED_VALUE, 0);
    if (!force && speedVal==oldVals.speedVal)
      return (TRUE);
    oldVals.speedVal = speedVal;
    sprintf(buff, "Speed Percent =  %3u", speedVal);
    DisplaySetLine(line, 0, buff);
    return (TRUE);
    }


static BOOLEAN UpdateDefault4(short line, BOOLEAN force)
  {
    WORD    gpm, kw, flowTransSpan, kwTransSpan;
    BOOLEAN ok;
    char    buff[DISP_COLS+1];

    gpm = HardwareReadRAM(R_GPM_01_VAL, 0);
    kw = HardwareReadRAM(R_KW_01_VAL, 0);
    flowTransSpan = HardwareReadRAM(R_FLOW_TRANS_SPAN, 0);
    kwTransSpan = HardwareReadRAM(R_KW_TRANS_SPAN, 0);
    ok = flowTransSpan>0 || kwTransSpan>0;
    if (!force && gpm==oldVals.gpm && kw==oldVals.kw)
      return (ok);
    oldVals.gpm = gpm;
    oldVals.kw = kw;
    if (flowTransSpan > 0) {
      sprintf(buff, "GPM =%5u", gpm);
      DisplaySetLine(line, 0, buff);
      }
    if (kwTransSpan > 0) {
      sprintf(buff, "KW =%5u", kw);
      DisplaySetLine(line, 11, buff);
      }
    return (ok);
    }


static BOOLEAN UpdateDefault5(short line, BOOLEAN force)
  {
    WORD  altSeq, val, max;
    short i;
    char  buff[DISP_COLS+1], *s;

    max = HardwareReadRAM(R_NUM_PUMPS, 0);
    altSeq = HardwareReadRAM(R_ALT_SEQ_W, 0);
    if (max < 2)
      return (FALSE);
    if (!force && altSeq==oldVals.altSeq)
      return (TRUE);
    oldVals.altSeq = altSeq;
    i = altSeq;
    for (val=1; val<max && !(i & 0x0001); val++)
      i >>= 1;
    sprintf(buff, "Sequence=           ");
    s = buff+9;
    for (i=max; i; i--) {
      *s++ = '0' + val;
      if (i > 1)
        *s++ = '-';
      val = (val == max) ? 1 : val+1;
      }
    DisplaySetLine(line, 0, buff);
    return (TRUE);
    }


static void UpdateXmittingState(void)
  {
    HS_STATE hwState;
    S_STATE  state;

    hwState = HardwareSerLink();
    if (xmittingState != hwState) {
      xmittingState = hwState;
      state = ShiftStateGet();
      if (state == S_OFF || state == S_ALARM) {
        switch (xmittingState) {
          case HS_CONNECT:
            DisplayClear();
            defaultState=DEFAULT_NUM;
            defaultTimer=0L;
            break;
          case HS_OFFLINE:
            if (state == S_ALARM)
              AlarmDeactivate();
            DisplaySetLine(DISP_LINE1, 0, "Communication Failed");
            DisplayScrollLineOn(DISP_LINE2, "O/I interface not communicating with P.L.C.; call factory for help               ");
            break;
          case HS_ERROR:
            if (state == S_ALARM)
              AlarmDeactivate();
            DisplaySetLine(DISP_LINE1, 0, "Communication Error ");
            DisplayScrollLineOn(DISP_LINE2, "O/I interface receiving invalid commands from P.L.C.; call factory for help               ");
            break;
          }
        }
      }
    }
