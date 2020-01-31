/* alarm.c -- February 5, 1990 */

/* Alarm Module */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>

#include "display.h"
#include "hardware.h"
#include "state.h"

#define ALARM_HELP_MAX 99

typedef enum { HELP_SHORT, HELP_LONG } HELP_LEVEL;

static char       *alarmShortHelp[ALARM_HELP_MAX+1], *alarmLongHelp[ALARM_HELP_MAX+1];
static short      numAlarm, posAlarm;
static HELP_LEVEL levelAlarm;


static void AlarmHelpShort(void);


void AlarmActivate(void)
  {
    ShiftStatePush(S_ALARM);
    DisplaySetLine(DISP_LINE2, 0, "* * *  ALARM  * * * ");
    DisplayFlashLine(DISP_LINE2, TRUE);
    }


void AlarmDeactivate(void)
  {
    WORD val;
    char buff[DISP_COLS+1];

    val = HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x000C ? 1 : 2;
    if (val & 0x0004)
      DisplaySetLine(DISP_LINE2, 0, "System in BYPASS    ");
    else if (val & 0x0008) {
      sprintf(buff, "Manual Speed = %3u %%", 0x0000);
      DisplaySetLine(DISP_LINE2, 0, buff);
      }
    else
      DisplaySetLine(DISP_LINE2, 0, "                    ");
    ShiftStatePop();
    }


void AlarmHelp(void)
  {
    short i;
    WORD  val;
    char  buff[DISP_COLS+1];

    DisplayPush();
    ShiftStatePush(S_ALARM_HELP);
    levelAlarm = HELP_SHORT;
    posAlarm = 0;
    numAlarm = 0;
    val = HardwareReadRAM(R_ZONE_FAILURE, 0);
    for (i=1; i<=16; i++) {
      if (val & 0x0001) {
        sprintf(buff, "Zone %2d Failure     ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check wiring, piping, polarity, continuity               ";
        numAlarm++;
        }
      val >>= 1;
      }
    val = HardwareReadRAM(R_XMIT_CALIBRATE, 0);
    for (i=1; i<=16; i++) {
      if (val & 0x0001) {
        sprintf(buff, "Xmitter %2d Calibrate", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Transmitter to be re-calibrated               ";
        numAlarm++;
        }
      val >>= 1;
      }
    val = HardwareReadRAM(R_PUMP_FAILURE, 0);
    for (i=1; i<=4; i++) {
      if (val & 0x0001) {
        sprintf(buff, "Overload %d Failure  ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check Amp draw, use manual reset if OK               ";
        numAlarm++;
        }
      if (val & 0x0002) {
        sprintf(buff, "Pump %d Failure      ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check DP switch, impeller, coupler, motor               ";
        numAlarm++;
        }
      if (val & 0x0004) {
        sprintf(buff, "AFD %d Failure       ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check H-O-A switch, wiring & AFD's LED               ";
        numAlarm++;
        }
      if (val & 0x0008) {
        sprintf(buff, "Pump %d Off          ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Need pump in system; if OK-enable pump               ";
        numAlarm++;
        }
      val >>= 4;
      }
    val = HardwareReadRAM(R_PUMP_FAILURE+2, 0);
    for (i=5; i<=6; i++) {
      if (val & 0x0001) {
        sprintf(buff, "Overload %d Failure  ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check Amp draw, use manual reset if OK               ";
        numAlarm++;
        }
      if (val & 0x0002) {
        sprintf(buff, "Pump %d Failure      ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check DP switch, impeller, coupler, motor               ";
        numAlarm++;
        }
      if (val & 0x0004) {
        sprintf(buff, "AFD %d Failure       ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check H-O-A switch, wiring & AFD's LED               ";
        numAlarm++;
        }
      if (val & 0x0008) {
        sprintf(buff, "Pump %d Off          ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Need pump in system; if OK-enable pump               ";
        numAlarm++;
        }
      val >>= 4;
      }
    for (i=1; i<=4; i++) {
      if (val & 0x0001) {
        sprintf(buff, "PWR Supply %d Failure", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check wiring, fuses, supply calibration               ";
        numAlarm++;
        }
      val >>= 1;
      }
    for (i=1; i<=4; i++) {
      if (val & 0x0001) {
        sprintf(buff, "Remote %d in 'STOP'  ", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Wait for 'START' or put in 'LOCAL' & start               ";
        numAlarm++;
        }
      val >>= 1;
      }
    val = HardwareReadRAM(R_FLOW_KW_FAILURE, 0);
    for (i=1; i<=4; i++) {
      if (val & 0x0001) {
        sprintf(buff, "GPM Xmitter %d Failed", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check wiring, piping, polarity, continuity               ";
        numAlarm++;
        }
      val >>= 1;
      }
    for (i=1; i<=6; i++) {
      if (val & 0x0001) {
        sprintf(buff, "KW Xmitter %d Failure", i);
        alarmShortHelp[numAlarm] = strdup(buff);
        alarmLongHelp[numAlarm] = "Check wiring: polarity, continuity               ";
        numAlarm++;
        }
      val >>= 1;
      }
    if (val & 0x0001) {
      sprintf(buff, "Data Lost-Need Input");
      alarmShortHelp[numAlarm] = strdup(buff);
      alarmLongHelp[numAlarm] = "Replace battery now! Do 'SETUP' procedure               ";
      numAlarm++;
      }
    if (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0010) {
      sprintf(buff, "Rem-Loc-Off in 'OFF'");
      alarmShortHelp[numAlarm] = strdup(buff);
      alarmLongHelp[numAlarm] = "Verify that it's OK to turn system 'ON'               ";
      numAlarm++;
      }
    AlarmHelpShort();
    }


void AlarmHelpClear(void)
  {
    short i;

    if (levelAlarm == HELP_SHORT) {
      for (i=0; i<numAlarm; i++) {
        if (alarmShortHelp[i])
          free(alarmShortHelp[i]);
        }
      ShiftStatePop();
      DisplayPop();
      }
    else {
      levelAlarm = HELP_SHORT;
      AlarmHelpShort();
      }
    }


void AlarmHelpDown(void)
  {
    if (levelAlarm == HELP_LONG)
      return;
    posAlarm = posAlarm==0 ? numAlarm-1 : posAlarm-1;
    AlarmHelpShort();
    }


void AlarmHelpHelp(void)
  {
    if (levelAlarm == HELP_LONG)
      return;
    levelAlarm = HELP_LONG;
    DisplaySetLine(DISP_LINE1, 0, alarmShortHelp[posAlarm]);
    DisplayScrollLineOn(DISP_LINE2, alarmLongHelp[posAlarm]);
    }


static void AlarmHelpShort(void)
  {
    char buff[DISP_COLS+1];

    sprintf(buff, "   Alarm %2d of %2d   ", posAlarm+1, numAlarm);
    DisplaySetLine(DISP_LINE1, 0, buff);
    DisplaySetLine(DISP_LINE2, 0, alarmShortHelp[posAlarm]);
    }


void AlarmHelpUp(void)
  {
    if (levelAlarm == HELP_LONG)
      return;
    posAlarm = (posAlarm + 1) % numAlarm;
    AlarmHelpShort();
    }


void AlarmStatus(void)
  {
    S_STATE state;
    WORD    val;

    if (HardwareSerLink() == HS_CONNECT) {
      val  = HardwareReadRAM(R_ZONE_FAILURE, 0);
      val |= HardwareReadRAM(R_XMIT_CALIBRATE, 0);
      val |= HardwareReadRAM(R_PUMP_FAILURE, 0);
      val |= HardwareReadRAM(R_PUMP_FAILURE, 1);
      val |= HardwareReadRAM(R_FLOW_KW_FAILURE, 0);
      val |= HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0010;
      state = ShiftStateGet();
      if (val && state==S_OFF)
        AlarmActivate();
      else if (!val && state==S_ALARM)
        AlarmDeactivate();
      }
    }
