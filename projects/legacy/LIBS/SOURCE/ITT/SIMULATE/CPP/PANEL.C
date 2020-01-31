/* panel.c -- February 5, 1990 */

/* Panel Simulator */

#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>
#include <..\seachest\graphics.h>

#include "led.h"
#include "alarm.h"
#include "button.h"
#include "dialog.h"
#include "display.h"
#include "hardware.h"
#include "panel.h"
#include "setpoint.h"
#include "setup.h"
#include "state.h"
#include "test.h"
#include "text.h"

#define TOP 137
#define LEF   3
#define BOT 341
#define RIG 636

static BOOLEAN    delayOn=FALSE;
static JIFF       delayTimer, bumplesTimer=0L, helpTimeout, autoRepeat;
static buttonFunc delayFunc;
static short      changeUAC=0, procVarPos;
static WORD       fkeyStatus=0x0000, kpadFlag=0x0000;

static short bStartStop, bAuto, bReset, bHand, bPump1, bFunc1, bPump2;
static short bFunc2, bPump3, bFunc3, bPump4, bFunc4, bPump5, bFunc5, bPump6;
static short bFunc6, bProcVar, bAlt, bBypass, bSetPoint, bTest, bManual;
static short bUp, bSetup, bDown, bClear, bHelp, bEnter;


static void    DelayClear(void);
static void    DelaySet(JIFF timer, buttonFunc func);
static BOOLEAN DelayMatch(buttonFunc func);
static void    DisplayProcVar(void);

static void    doStartStop(void);
static void    doAuto(void);
static void    doReset(void);
static void    doHand(void);
static void    doPump1(void);
static void    doFunc1(void);
static void    doPump2(void);
static void    doFunc2(void);
static void    doPump3(void);
static void    doFunc3(void);
static void    doPump4(void);
static void    doFunc4(void);
static void    doPump5(void);
static void    doFunc5(void);
static void    doPump6(void);
static void    doFunc6(void);
static void    doProcVar(void);
static void    doAlt(void);
static void    doBypass(void);
static void    doSetPoint(void);
static void    doTest(void);
static void    doManual(void);
static void    doUp(void);
static void    doSetup(void);
static void    doDown(void);
static void    doClear(void);
static void    doHelp(void);
static void    doEnter(void);


static void DelayClear(void)
  {
    delayOn = FALSE;
    ShiftStatePop();
    }


static void DelaySet(JIFF timer, buttonFunc func)
  {
    delayTimer = EventJiffy() + timer;
    delayFunc = func;
    if (!delayOn) {
      delayOn = TRUE;
      ShiftStatePush(S_DELAY);
      }
    }


static BOOLEAN DelayMatch(buttonFunc func)
  {
    return (delayFunc == func);
    }


static void DisplayProcVar(void)
  {
    WORD sp, pv, pos;
    char buff[DISP_COLS+1];

    pos = (procVarPos - 1) * 2;
    sp = HardwareReadRAM(R_ZONE_01_SP + pos, 0);
    pv = HardwareReadRAM(R_PV_01 + pos, 0);
    sprintf(buff, "Zone%2d:SP=%3u PV=%3u", procVarPos, sp, pv);
    DisplaySetLine(DISP_LINE1, 0, buff);
    if (procVarPos+1 > HardwareReadRAM(R_NUM_ZONES, 0))
      DisplayClearLine(DISP_LINE2);
    else {
      pos = procVarPos * 2;
      sp = HardwareReadRAM(R_ZONE_01_SP + pos, 0);
      pv = HardwareReadRAM(R_PV_01 + pos, 0);
      sprintf(buff, "Zone%2d:SP=%3u PV=%3u", procVarPos+1, sp, pv);
      DisplaySetLine(DISP_LINE2, 0, buff);
      }
    }


static void HelpOnKeyBegin(void)
  {
    DisplayPush();
    ShiftStatePush(S_HELP);
    DisplaySetLine(DISP_LINE1, 0, "HELP: Press a Key...");
    DisplayClearLine(DISP_LINE2);
    }


static void HelpOnKeyEnd(void)
  {
    ShiftStatePop();
    DisplayPop();
    }


static void HelpOnKeySet(char *line1, char *line2)
  {
    DisplaySetLine(DISP_LINE1, 0, line1);
    if (strlen(line2) > DISP_COLS)
      DisplayScrollLineOn(DISP_LINE2, line2);
    else
      DisplaySetLine(DISP_LINE2, 0, line2);
    ShiftStateSet(S_HELP_CLEAR);
    helpTimeout = EventJiffy() + 6000;
    }


void PanelButtonUp(void)
  {
    fkeyStatus = 0x0000;
    kpadFlag = 0x0000;
    }


BOOLEAN PanelDraw(void)
  {
    Point pLED, pText, pDisplay;
    Rect  r;

    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, getmaxx(), getmaxy());
    setfillstyle(SOLID_FILL, RED);
    bar(LEF, TOP, RIG, TOP+23);
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(LEF, TOP+24, RIG, BOT);
    setfillstyle(SOLID_FILL, BLACK);
    bar(LEF+65, TOP+24, LEF+426, TOP+116);
    setcolor(LIGHTGRAY);
    line(LEF+65, TOP+42, LEF+426, TOP+42);
    setcolor(BLACK);
    rectangle(LEF-1, TOP-1, RIG+1, BOT+1);
    setcolor(WHITE);
    settextstyle(G_FONT, HORIZ_DIR, 5);
    outtextxy(LEF+70, TOP+5, "Technologic Pump Controller");
    settextstyle(G_FONT, HORIZ_DIR, 4);
    outtextxy(LEF+70, TOP+28, "ITT Bell & Gossett");
    PASG(&pDisplay, TOP+48, LEF+66);
    if (!DisplayInit(&pDisplay, LIGHTCYAN, BLACK))
      return (FALSE);

    PASG(&pLED, 6, 6);

    PASG(&pText, 8, 1);
    RASG(&r, 262, 68, 290, 105);
    bStartStop = ButtonCreate(&r, RED, LED_BAD, TEXT_BAD, doStartStop);
    ButtonAddLED(bStartStop, &pLED, 4, L_OFF);
    ButtonAddText(bStartStop, &pText, WHITE, 4, "Start\rStop");

    PASG(&pText, 17, 1);
    RASG(&r, 262, 114, 290, 151);
    bReset = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doReset);
    ButtonAddLED(bReset, &pLED, 4, L_OFF);
    ButtonAddText(bReset, &pText, WHITE, 4, "Reset");

    RASG(&r, 262, 160, 290, 197);
    bPump1 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doPump1);
    ButtonAddLED(bPump1, &pLED, 4, L_OFF);
    ButtonAddText(bPump1, &pText, WHITE, 4, "Pump 1");

    RASG(&r, 262, 206, 290, 243);
    bPump2 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doPump2);
    ButtonAddLED(bPump2, &pLED, 4, L_OFF);
    ButtonAddText(bPump2, &pText, WHITE, 4, "Pump 2");

    RASG(&r, 262, 252, 290, 289);
    bPump3 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doPump3);
    ButtonAddLED(bPump3, &pLED, 4, L_OFF);
    ButtonAddText(bPump3, &pText, WHITE, 4, "Pump 3");

    RASG(&r, 262, 298, 290, 335);
    bPump4 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doPump4);
    ButtonAddLED(bPump4, &pLED, 4, L_OFF);
    ButtonAddText(bPump4, &pText, WHITE, 4, "Pump 4");

    RASG(&r, 262, 344, 290, 381);
    bPump5 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doPump5);
    ButtonAddLED(bPump5, &pLED, 4, L_OFF);
    ButtonAddText(bPump5, &pText, WHITE, 4, "Pump 5");

    RASG(&r, 262, 390, 290, 427);
    bPump6 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doPump6);
    ButtonAddLED(bPump6, &pLED, 4, L_OFF);
    ButtonAddText(bPump6, &pText, WHITE, 4, "Pump 6");

    RASG(&r, 300, 68, 327, 105);
    bAuto = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doAuto);
    ButtonAddLED(bAuto, &pLED, 4, L_OFF);
    ButtonAddText(bAuto, &pText, WHITE, 4, "Auto");

    RASG(&r, 300, 114, 327, 151);
    bHand = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doHand);
    ButtonAddLED(bHand, &pLED, 4, L_OFF);
    ButtonAddText(bHand, &pText, WHITE, 4, "Hand");

    RASG(&r, 300, 160, 327, 197);
    bFunc1 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doFunc1);
    ButtonAddLED(bFunc1, &pLED, 4, L_OFF);

    RASG(&r, 300, 206, 327, 243);
    bFunc2 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doFunc2);
    ButtonAddLED(bFunc2, &pLED, 4, L_OFF);

    RASG(&r, 300, 252, 327, 289);
    bFunc3 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doFunc3);
    ButtonAddLED(bFunc3, &pLED, 4, L_OFF);

    RASG(&r, 300, 298, 327, 335);
    bFunc4 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doFunc4);
    ButtonAddLED(bFunc4, &pLED, 4, L_OFF);

    RASG(&r, 300, 344, 327, 381);
    bFunc5 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doFunc5);
    ButtonAddLED(bFunc5, &pLED, 4, L_OFF);

    RASG(&r, 300, 390, 327, 427);
    bFunc6 = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doFunc6);
    ButtonAddLED(bFunc6, &pLED, 4, L_OFF);

    PASG(&pText, 0, 1);
    RASG(&r, 225, 445, 253, 473);
    bProcVar = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doProcVar);
    ButtonAddText(bProcVar, &pText, WHITE, 4, "Proc\rVar\r1");

    RASG(&r, 262, 445, 290, 473);
    bAlt = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doAlt);
    ButtonAddText(bAlt, &pText, WHITE, 4, "Alt\r\r4");

    RASG(&r, 300, 445, 327, 473);
    bBypass = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doBypass);
    ButtonAddText(bBypass, &pText, WHITE, 4, "Byp\r\r7");

    RASG(&r, 225, 478, 253, 506);
    bSetPoint = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doSetPoint);
    ButtonAddText(bSetPoint, &pText, WHITE, 4, "Set\rPoint\r2");

    RASG(&r, 262, 478, 290, 506);
    bTest = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doTest);
    ButtonAddText(bTest, &pText, WHITE, 4, "Test\r\r5");

    RASG(&r, 300, 478, 327, 506);
    bManual = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doManual);
    ButtonAddText(bManual, &pText, WHITE, 4, "Man\r\r8");

    RASG(&r, 225, 511, 253, 539);
    bUp = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doUp);
    ButtonAddText(bUp, &pText, WHITE, 4, "Up\r\r3");

    RASG(&r, 262, 511, 290, 539);
    bSetup = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doSetup);
    ButtonAddText(bSetup, &pText, WHITE, 4, "Set\rUp\r6");

    RASG(&r, 300, 511, 327, 539);
    bDown = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doDown);
    ButtonAddText(bDown, &pText, WHITE, 4, "Down\r\r9");

    RASG(&r, 225, 544, 253, 572);
    bClear = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doClear);
    ButtonAddText(bClear, &pText, WHITE, 4, "Clear");

    RASG(&r, 262, 544, 290, 572);
    bHelp = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doHelp);
    ButtonAddText(bHelp, &pText, WHITE, 4, "Help\r\r0");

    RASG(&r, 300, 544, 327, 572);
    bEnter = ButtonCreate(&r, BLACK, LED_BAD, TEXT_BAD, doEnter);
    ButtonAddText(bEnter, &pText, WHITE, 4, "Enter");

    ButtonDrawAll();

    return (TRUE);
    }


void PanelUpdate(void)
  {
    WORD    val, valRead, valWrite;
    S_STATE state;

    if (hwUpdate.fkey) {
      if (HardwareReadRAM(R_FKEY_STATUS_W, 0) == HardwareReadRAM(R_FKEY_STATUS_R, 0))
        HardwareWriteRAM(R_FKEY_STATUS_R, 0, fkeyStatus);
      hwUpdate.fkey = FALSE;
      }
    if (hwUpdate.kpad) {
      val = HardwareReadRAM(R_KPAD_FLAG_R, 0);
      valRead = val & 0x0180;
      valWrite = HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0180;
      if (valRead == valWrite)
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, (val & ~0x0180) | kpadFlag);
      hwUpdate.kpad = FALSE;
      }
    state = ShiftStateGet();
    if (hwUpdate.pv) {
      if (state==S_PROC_VAR && HardwareSerLink()==HS_CONNECT)
        DisplayProcVar();
      hwUpdate.pv = FALSE;
      }
    if (delayOn && EventElapsed(delayTimer))
      (*delayFunc)();
    if (state==S_BUMPLES && EventElapsed(bumplesTimer)) {
      DisplayPop();
      ShiftStatePop();
      }
    if (state==S_HELP_CLEAR && EventElapsed(helpTimeout))
      HelpOnKeyEnd();
    }


static void doStartStop(void)
  {
    WORD val;

    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        fkeyStatus = 0x0001;
        HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0001);
        val  = HardwareReadRAM(R_LEDA_STATUS, 0) & 0xFF00;
        val |= HardwareReadRAM(R_LEDB_STATUS, 0);
        if (!val) {
          DisplayPush();
          DisplaySetLine(DISP_LINE2, 0, "Please Enable A Pump");
          DelaySet(250L, doStartStop);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doStartStop)) {
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Start/Stop'", "Used to enable and disable outputs of the controller; Green=Start, Red=Stop               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doAuto(void)
  {
    WORD val;

    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (!EventElapsed(bumplesTimer)) {
          if (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0008) {
            DisplayPush();
            DisplaySetLine(DISP_LINE1, 0, " Auto Speed Control ");
            DisplaySetLine(DISP_LINE2, 0, "Accept With 'ENTER' ");
            ShiftStatePush(S_BUMPLES);
            bumplesTimer = EventJiffy() + 500;
            }
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Auto'      ", "Used to engage the automatic control of the system; Green=Auto Control On               ");
        break;
      case S_HAND:
        fkeyStatus = 0x0100;
        HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0100);
        val = HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x000C;
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, val);
        ShiftStatePop();
        DisplayPop();
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doReset(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        fkeyStatus = 0x0002;
        HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0002);
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Reset'     ", "Used to reset alarm after its correction; Red=Reset Required               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doHand(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if ((HardwareSerLink() != HS_CONNECT) || (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x000C))
          return;
        if (EventElapsed(bumplesTimer)) {
          if (!(HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0200)) {   /* stop */
            fkeyStatus = 0x0200;
            HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0200);
            DisplayPush();
            ShiftStatePush(S_HAND);
            DisplaySetLine(DISP_LINE1, 0, "System in 'HAND'    ");
            DisplayScrollLineOn(DISP_LINE2, "Press: 'BYPASS', 'MANUAL', or 'AUTO'               ");
            }
          }
        else {
          DisplayPush();
          DisplaySetLine(DISP_LINE1, 0, "Manual Speed Control");
          DisplaySetLine(DISP_LINE2, 0, "Accept With 'ENTER' ");
          ShiftStatePush(S_BUMPLES);
          bumplesTimer = EventJiffy() + 500;
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Hand'      ", "Allows user to force bypass and manually set speed; Green=Hand Control On               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doPump1(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_NUM_PUMPS, 0) >= 1) {
          fkeyStatus = 0x0004;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0004);
          }
        else {
          DisplayPush();
          ButtonSetLED(bPump1, L_RED);
          DisplaySetLine(DISP_LINE2, 0, "Pump not available. ");
          DelaySet(250L, doPump1);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doPump1)) {
          ButtonSetLED(bPump1, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Pump 1'    ", "Used to start/stop a pump; Flashing Green=Enable, Green=Run, Red=Bypass, Flashing Red=Failure               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doFunc1(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        DisplayPush();
        ButtonSetLED(bFunc1, L_RED);
        DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
        DelaySet(250L, doFunc1);
        break;
      case S_DELAY:
        if (DelayMatch(doFunc1)) {
          ButtonSetLED(bFunc1, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on Function Key", "Key not used.       ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doPump2(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_NUM_PUMPS, 0) >= 2) {
          fkeyStatus = 0x0008;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0008);
          }
        else {
          DisplayPush();
          ButtonSetLED(bPump2, L_RED);
          DisplaySetLine(DISP_LINE2, 0, "Pump not available. ");
          DelaySet(250L, doPump2);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doPump2)) {
          ButtonSetLED(bPump2, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Pump 2'    ", "Used to start/stop a pump; Flashing Green=Enable, Green=Run, Red=Bypass, Flashing Red=Failure               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doFunc2(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        DisplayPush();
        ButtonSetLED(bFunc2, L_RED);
        DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
        DelaySet(250L, doFunc2);
        break;
      case S_DELAY:
        if (DelayMatch(doFunc2)) {
          ButtonSetLED(bFunc2, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on Function Key", "Key not used.       ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doPump3(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_NUM_PUMPS, 0) >= 3) {
          fkeyStatus = 0x0010;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0010);
          }
        else {
          DisplayPush();
          ButtonSetLED(bPump3, L_RED);
          DisplaySetLine(DISP_LINE2, 0, "Pump not available. ");
          DelaySet(250L, doPump3);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doPump3)) {
          ButtonSetLED(bPump3, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Pump 3'    ", "Used to start/stop a pump; Flashing Green=Enable, Green=Run, Red=Bypass, Flashing Red=Failure               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doFunc3(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        DisplayPush();
        ButtonSetLED(bFunc3, L_RED);
        DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
        DelaySet(250L, doFunc3);
        break;
      case S_DELAY:
        if (DelayMatch(doFunc3)) {
          ButtonSetLED(bFunc3, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on Function Key", "Key not used.       ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doPump4(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_NUM_PUMPS, 0) >= 4) {
          fkeyStatus = 0x0020;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0020);
          }
        else {
          DisplayPush();
          ButtonSetLED(bPump4, L_RED);
          DisplaySetLine(DISP_LINE2, 0, "Pump not available. ");
          DelaySet(250L, doPump4);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doPump4)) {
          ButtonSetLED(bPump4, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Pump 4'    ", "Used to start/stop a pump; Flashing Green=Enable, Green=Run, Red=Bypass, Flashing Red=Failure               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doFunc4(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        DisplayPush();
        ButtonSetLED(bFunc4, L_RED);
        DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
        DelaySet(250L, doFunc4);
        break;
      case S_DELAY:
        if (DelayMatch(doFunc4)) {
          ButtonSetLED(bFunc4, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on Function Key", "Key not used.       ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doPump5(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_NUM_PUMPS, 0) >= 5) {
          fkeyStatus = 0x0040;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0040);
          }
        else {
          DisplayPush();
          ButtonSetLED(bPump5, L_RED);
          DisplaySetLine(DISP_LINE2, 0, "Pump not available. ");
          DelaySet(250L, doPump5);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doPump5)) {
          ButtonSetLED(bPump5, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Pump 5'    ", "Used to start/stop a pump; Flashing Green=Enable, Green=Run, Red=Bypass, Flashing Red=Failure               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doFunc5(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        DisplayPush();
        ButtonSetLED(bFunc5, L_RED);
        DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
        DelaySet(250L, doFunc5);
        break;
      case S_DELAY:
        if (DelayMatch(doFunc5)) {
          ButtonSetLED(bFunc5, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on Function Key", "Key not used.       ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doPump6(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_NUM_PUMPS, 0) >= 6) {
          fkeyStatus = 0x0080;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0080);
          }
        else {
          DisplayPush();
          ButtonSetLED(bPump6, L_RED);
          DisplaySetLine(DISP_LINE2, 0, "Pump not available. ");
          DelaySet(250L, doPump6);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doPump6)) {
          ButtonSetLED(bPump6, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Pump 6'    ", "Used to start/stop a pump; Flashing Green=Enable, Green=Run, Red=Bypass, Flashing Red=Failure               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doFunc6(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        DisplayPush();
        ButtonSetLED(bFunc6, L_RED);
        DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
        DelaySet(250L, doFunc6);
        break;
      case S_DELAY:
        if (DelayMatch(doFunc6)) {
          ButtonSetLED(bFunc6, L_OFF);
          DisplayPop();
          DelayClear();
          }
        break;
      case S_HELP:
        HelpOnKeySet("Help on Function Key", "Key not used.       ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doProcVar(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()==HS_CONNECT && HardwareReadRAM(R_NUM_ZONES, 0) > 0L) {
          procVarPos = 1;
          DisplayPush();
          DisplayProcVar();
          ShiftStatePush(S_PROC_VAR);
          }
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '1');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Proc Var'  ", "Used to view process variables; use up and down arrow keys to scroll               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doAlt(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareSerLink()!=HS_CONNECT)
          return;
        if (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0002) {   /* if 1, not allowed */
          DisplayPush();
          DisplaySetLine(DISP_LINE2, 0, "Function Not Allowed");
          DelaySet(250L, doAlt);
          }
        else {   /* if 0, allowed */
          DisplayPush();
          DisplaySetLine(DISP_LINE1, 0, "    Alternate ?     ");
          DisplaySetLine(DISP_LINE2, 0, "   'ENTER' if OK    ");
          ShiftStatePush(S_ALT);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doAlt)) {
          DisplayPop();
          DelayClear();
          }
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '4');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Alt'       ", "Used to alternate pump sequence               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doBypass(void)
  {
    switch (ShiftStateGet()) {
      case S_NUMBER:
        EventPost(EVT_KEY, '7');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Bypass'    ", "Used in 'Hand' to enable constant speed operation               ");
        break;
      case S_HAND:
        if (HardwareReadRAM(R_BYPASS_ENABLED, 0) == 0x0001) {
          DisplaySetLine(DISP_LINE1, 0, "  Hand - Bypass ?   ");
          DisplaySetLine(DISP_LINE2, 0, "   'ENTER' if OK    ");
          ShiftStateSet(S_BYPASS);
          }
        else {
          DisplayPush();
          DisplaySetLine(DISP_LINE2, 0, "Function not used.  ");
          DelaySet(250L, doBypass);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doBypass)) {
          DisplayPop();
          DelayClear();
          }
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doSetPoint(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (HardwareReadRAM(R_NUM_ZONES, 0) > 0L)
          SetPointChange();
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '2');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Set Point' ", "Used to enter set points; use up and down arrow keys to select zones and enter data               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doTest(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        TestMenu();
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '5');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Test'      ", "Used to test LED's, display and transmitters               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doManual(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        bumplesTimer = EventJiffy() + 150;
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '8');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Manual'    ", "Used in 'Hand' to manually set speed; use up and down arrow keys               ");
        break;
      case S_HAND:
        DisplaySetLine(DISP_LINE1, 0, "  Hand - Manual ?   ");
        DisplaySetLine(DISP_LINE2, 0, "   'ENTER' if OK    ");
        ShiftStateSet(S_MANUAL);
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      }
    }


static void doUp(void)
  {
    WORD    val;
    char    buff[DISP_COLS+1];
    S_STATE state;

    state = ShiftStateGet();
    switch (state) {
      case S_OFF:
      case S_ALARM:
        if (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0008) {
          val = HardwareReadRAM(R_MANUAL_SPEED, 0);
          if (val < 100)
            val++;
          HardwareWriteRAM(R_MANUAL_SPEED, 0, val);
          if (state == S_ALARM)
            DisplayPush();
          sprintf(buff, "Manual Speed = %3u %%", val);
          DisplaySetLine(DISP_LINE2, 0, buff);
          autoRepeat = 50L;
          DelaySet(autoRepeat, doUp);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doUp)) {
          if (ButtonStillDown(bUp)) {
            val = HardwareReadRAM(R_MANUAL_SPEED, 0);
            if (val < 100)
              val++;
            HardwareWriteRAM(R_MANUAL_SPEED, 0, val);
            sprintf(buff, "%3u", val);
            DisplaySetLine(DISP_LINE2, 15, buff);
            if (autoRepeat > 10)
              autoRepeat -= 10;
            DelaySet(autoRepeat, doUp);
            }
          else {
            DelayClear();
            if (ShiftStateGet() == S_ALARM)
              DisplayPop();
            }
          }
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '3');
        break;
      case S_SELECT:
        changeUAC = (changeUAC == 2) ? 3 : 0;
        EventPost(EVT_KEY, K_UP);
        break;
      case S_ALARM_HELP:
        AlarmHelpUp();
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Up Arrow'  ", "Used for selection and manual speed set               ");
        break;
      case S_PROC_VAR:
        if (procVarPos == 1) {
          procVarPos = HardwareReadRAM(R_NUM_ZONES, 0);
          if (!(procVarPos % 2))
            procVarPos--;
          }
        else
          procVarPos -= 2;
        DisplayProcVar();
        break;
      case S_TEST:
        TestUp();
        break;
      }
    }


static void doSetup(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        kpadFlag = 0x0100;
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, HardwareReadRAM(R_KPAD_FLAG_R, 0) | 0x0100);
        changeUAC = 1;
        SetupDo();
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '6');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Set Up'    ", "Used to define system parameters - security               ");
        break;
      }
    }


static void doDown(void)
  {
    WORD    val;
    char    buff[DISP_COLS+1];
    S_STATE state;

    state = ShiftStateGet();
    switch (state) {
      case S_OFF:
      case S_ALARM:
        if (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0008) {
          val = HardwareReadRAM(R_MANUAL_SPEED, 0);
          if (val > 0)
            val--;
          HardwareWriteRAM(R_MANUAL_SPEED, 0, val);
          if (state == S_ALARM)
            DisplayPush();
          sprintf(buff, "Manual Speed = %3u %%", val);
          DisplaySetLine(DISP_LINE2, 0, buff);
          autoRepeat = 50L;
          DelaySet(autoRepeat, doDown);
          }
        break;
      case S_DELAY:
        if (DelayMatch(doDown)) {
          if (ButtonStillDown(bDown)) {
            val = HardwareReadRAM(R_MANUAL_SPEED, 0);
            if (val > 0)
              val--;
            HardwareWriteRAM(R_MANUAL_SPEED, 0, val);
            sprintf(buff, "%3u", val);
            DisplaySetLine(DISP_LINE2, 15, buff);
            if (autoRepeat > 10)
              autoRepeat -= 10;
            DelaySet(autoRepeat, doDown);
            }
          else {
            DelayClear();
            if (ShiftStateGet() == S_ALARM)
              DisplayPop();
            }
          }
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '9');
        break;
      case S_SELECT:
        if (changeUAC == 3)
          SetupChangeUAC();
        changeUAC = 0;
        EventPost(EVT_KEY, K_DOWN);
        break;
      case S_ALARM_HELP:
        AlarmHelpDown();
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Down Arrow'", "Used for selection and manual speed set               ");
        break;
      case S_PROC_VAR:
        procVarPos += 2;
        if (procVarPos > HardwareReadRAM(R_NUM_ZONES, 0))
          procVarPos = 1;
        DisplayProcVar();
        break;
      case S_TEST:
        TestDown();
        break;
      }
    }


static void doClear(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
      case S_ALARM:
        if (!(HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0200)) {   /* stop */
          kpadFlag = 0x0080;
          HardwareWriteRAM(R_KPAD_FLAG_R, 0, HardwareReadRAM(R_KPAD_FLAG_R, 0) | 0x0080);
          if (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x000C) {
            DisplayPush();
            DisplaySetLine(DISP_LINE1, 0, "System in 'HAND'    ");
            DisplayScrollLineOn(DISP_LINE2, "Press: 'BYPASS', 'MANUAL', or 'AUTO'               ");
            ShiftStatePush(S_HAND);
            }
          }
        break;
      case S_SELECT:
      case S_NUMBER:
        changeUAC = (changeUAC == 1) ? 2 : 0;
        DialogClear();
        break;
      case S_ALARM_HELP:
        AlarmHelpClear();
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Clear'     ", "Clears latest entry (operation and data)               ");
        break;
      case S_HELP_CLEAR:
        HelpOnKeyEnd();
        break;
      case S_BYPASS:
      case S_MANUAL:
        DisplaySetLine(DISP_LINE1, 0, "System in 'HAND'    ");
        DisplayScrollLineOn(DISP_LINE2, "Press: 'BYPASS', 'MANUAL', or 'AUTO'               ");
        ShiftStateSet(S_HAND);
        break;
      case S_ALT:
        DisplayPop();
        ShiftStatePop();
        break;
      case S_PROC_VAR:
        DisplayPop();
        ShiftStatePop();
        break;
      case S_TEST:
        TestClear();
        break;
      }
    }


static void doHelp(void)
  {
    switch (ShiftStateGet()) {
      case S_OFF:
        HelpOnKeyBegin();
        break;
      case S_NUMBER:
        EventPost(EVT_KEY, '0');
        break;
      case S_HELP:
        HelpOnKeySet("Help on 'Help'      ", "User assistance; press: Help + Key/Item               ");
        break;
      case S_SELECT:
        changeUAC = 0;
        break;
      case S_ALARM:
        AlarmHelp();
        break;
      case S_ALARM_HELP:
        AlarmHelpHelp();
        break;
      }
    }


static void doEnter(void)
  {
    WORD val;
    char buff[DISP_COLS+1];
    JIFF timeOut;

    switch (ShiftStateGet()) {
      case S_HELP:
        HelpOnKeySet("Help on 'Enter'     ", "Used to accept data and operation input               ");
        break;
      case S_BYPASS:
        val = HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x0008;
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, val | 0x0004);
        ShiftStatePop();
        DisplayPop();
        break;
      case S_MANUAL:
        val = HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x0004;
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, val | 0x0008);
        HardwareWriteRAM(R_MANUAL_SPEED, 0, 0x0000);
        ShiftStatePop();
        DisplayPop();
        break;
      case S_ALT:
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, HardwareReadRAM(R_KPAD_FLAG_R, 0) | 0x0001);
        timeOut = EventJiffy() + 2000;
        while ((!EventElapsed(timeOut)) && (!(HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0001))) {   /* wait for 1 */
          HardwareCommunicate();
          LEDUpdate();
          }
        ButtonUp();
        DisplaySetLine(DISP_LINE2, 0, "      Stand By      ");
        HardwareWriteRAM(R_KPAD_FLAG_R, 0, HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x0001);
        if (!EventElapsed(timeOut)) {
          DisplaySetLine(DISP_LINE2, 0, "Alt Engaged-Stand By");
          timeOut = EventJiffy() + 2000;
          while ((!EventElapsed(timeOut)) && (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0001)) {   /* wait for 0 */
            HardwareCommunicate();
            LEDUpdate();
            }
          if (EventElapsed(timeOut))
            DisplayShowMessage(DISP_LINE2, " Alt Not Completed  ", 250L);
          else
            DisplayShowMessage(DISP_LINE2, "   Alt Completed    ", 250L);
          }
        else
          DisplayShowMessage(DISP_LINE2, " No Acknowledgement ", 250L);
        DisplayPop();
        ShiftStatePop();
        break;
      case S_BUMPLES:
        val = HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x000C;
        if (val & 0x0008) {   /* manual -> auto */
          fkeyStatus = 0x0100;
          HardwareWriteRAM(R_FKEY_STATUS_R, 0, HardwareReadRAM(R_FKEY_STATUS_R, 0) | 0x0100);
          val = HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x000C;
          HardwareWriteRAM(R_KPAD_FLAG_R, 0, val);
          DisplayPop();
          ShiftStatePop();
          }
        else if (!val) {      /* auto -> manual */
          val = HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x0004;
          HardwareWriteRAM(R_KPAD_FLAG_R, 0, val | 0x0008);
          HardwareWriteRAM(R_MANUAL_SPEED, 0, HardwareReadRAM(R_SPEED_VALUE, 0));
          DisplayPop();
          if (ShiftStatePop() == S_OFF) {
            sprintf(buff, "Manual Speed = %3u %%", HardwareReadRAM(R_SPEED_VALUE, 0));
            DisplaySetLine(DISP_LINE2, 0, buff);
            }
          }
        break;
      case S_TEST:
        TestSelect();
        break;
      default:
        changeUAC = 0;
        EventPost(EVT_KEY, K_RETURN);
        break;
      }
    }
