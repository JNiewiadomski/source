/* setup.c -- February 5, 1990 */

/* Setup Library */

#include <stdlib.h>
#include <string.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>

#include "led.h"
#include "button.h"
#include "datalink.h"
#include "dialog.h"
#include "display.h"
#include "hardware.h"
#include "network.h"
#include "setup.h"
#include "state.h"

static char    *baudRateList[] = { "????", "9600", "1200" };
static char    *yesNoList[] = { "???", "No", "Yes" };
static char    *pvUnitsList[] = { "????", "PSIÇ", "PSI", "ÉF", "ÇÉF", "ÉC", "ÇÉC" };
static WORD    newUAC;


static void DoChangeSetup(short ch);
static void DoChangeUAC(short ch);
static void DoVerifyUAC(short ch);
static void DoChangeNodeNumber(short ch);
static void DoChangeBaudRate(short ch);
static void DoChangeEOCMaxFlow(short ch);
static void DoChangeEOCDStage(short ch);
static void DoChangeDStageVal01(short ch);
static void DoChangeDStageTim01(short ch);
static void DoChangeDStageVal02(short ch);
static void DoChangeDStageTim02(short ch);
static void DoChangeDStageVal03(short ch);
static void DoChangeDStageTim03(short ch);
static void DoChangeDStageVal04(short ch);
static void DoChangeDStageTim04(short ch);
static void DoChangeDStageVal05(short ch);
static void DoChangeDStageTim05(short ch);
static void DoChangeDefaultAccept(short ch);
static void DoChangePumpNumber(short ch);
static void DoChange1stStageTimer(short ch);
static void DoChange2ndStageTimer(short ch);
static void DoChange3rdStageTimer(short ch);
static void DoChange4thStageTimer(short ch);
static void DoChange5thStageTimer(short ch);
static void DoChangeAutoAltTime(short ch);
static void DoChangePIDLoopNum(short ch);
static void DoChangeProportional(short ch);
static void DoChangeIntegral(short ch);
static void DoChangeDerivative(short ch);
static void DoChangePVUnits(short ch);
static void DoChangePVTransSpan(short ch);
static void DoChangeFlowTransSpan(short ch);
static void DoChangeKWTransSpan(short ch);
static void DoChangeNumZones(short ch);
static void DoChangeBypassEnabled(short ch);

static void SetupVerifyUAC(void);
static void SetupNodeNumber(void);
static void SetupBaudRate(void);
static void SetupEOCMaxFlow(void);
static void SetupEOCDStage(void);
static void SetupDStageVal01(void);
static void SetupDStageTim01(void);
static void SetupDStageVal02(void);
static void SetupDStageTim02(void);
static void SetupDStageVal03(void);
static void SetupDStageTim03(void);
static void SetupDStageVal04(void);
static void SetupDStageTim04(void);
static void SetupDStageVal05(void);
static void SetupDStageTim05(void);
static void SetupDefaultAccept(void);
static void SetupPumpNumber(void);
static void Setup1stStageTimer(void);
static void Setup2ndStageTimer(void);
static void Setup3rdStageTimer(void);
static void Setup4thStageTimer(void);
static void Setup5thStageTimer(void);
static void SetupAutoAltTime(void);
static void SetupPIDLoopNum(void);
static void SetupProportional(void);
static void SetupIntegral(void);
static void SetupDerivative(void);
static void SetupPVUnits(void);
static void SetupPVTransSpan(void);
static void SetupFlowTransSpan(void);
static void SetupKWTransSpan(void);
static void SetupNumZones(void);
static void SetupBypassEnabled(void);

static BOOLEAN PLCAcceptMAC(void);


void SetupDo(void)
  {
    DisplayPush();
    ShiftStatePush(S_KEEP_STATE);
    DisplaySetLine(DISP_LINE1, 0, "Enter Access Code   ");
    DisplaySetLine(DISP_LINE2, 0, "      _____         ");
    DialogStartCode(DISP_LINE2, 6, 5, DoChangeSetup, 0, 65535U);
    }


static void DoChangeSetup(short ch)
  {
    char    *val;
    WORD    code;
    BOOLEAN ok;

    switch (ch) {
      case K_RETURN:
        ok = strlen(val = DialogGetVal()) > 0;
        code = atoi(val);
        if (ok && code==HardwareReadRAM(R_USER_CODE_W, 0))
          SetupDefaultAccept();
        else if (ok && code==65432U)   /* SAC - System Access Code */
          SetupNodeNumber();
        else if (ok && code == HardwareReadRAM(R_MASTER_CODE_W, 0)) {
          HardwareWriteRAM(R_MASTER_CODE_R, 0, code);
          if (HardwareSerLink() == HS_CONNECT) {
            if (PLCAcceptMAC())
              SetupEOCMaxFlow();
            else {
              ShiftStatePop();
              DisplayPop();
              }
            }
          else
            SetupEOCMaxFlow();
          }
        else {
          ShiftStatePop();
          DisplayPop();
          }
        break;
      }
    }


static BOOLEAN PLCAcceptMAC(void)
  {
    JIFF    timeOut;

    HardwareWriteRAM(R_KPAD_FLAG_R, 0, HardwareReadRAM(R_KPAD_FLAG_R, 0) | 0x0020);
    DisplaySetLine(DISP_LINE2, 0, "      Stand By      ");
    timeOut = EventJiffy() + 2000;
    while ((!EventElapsed(timeOut)) && (!(HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0040))) {
      HardwareCommunicate();
      LEDUpdate();
      }
    HardwareWriteRAM(R_KPAD_FLAG_R, 0, HardwareReadRAM(R_KPAD_FLAG_R, 0) & ~0x0020);
    if (EventElapsed(timeOut)) {
      DisplayShowMessage(DISP_LINE2, " No Acknowledgement ", 250L);
      return (FALSE);
      }
    DisplayShowMessage(DISP_LINE2, " Confirming Code... ", 250L);
    timeOut = EventJiffy() + 2000;
    while ((!EventElapsed(timeOut)) && (HardwareReadRAM(R_KPAD_FLAG_W, 0) & 0x0040)) {
      HardwareCommunicate();
      LEDUpdate();
      }
    return (!EventElapsed(timeOut));
    }


void SetupChangeUAC(void)
  {
    DisplaySetLine(DISP_LINE1, 0, "New User Code       ");
    DisplaySetLine(DISP_LINE2, 0, "   'ENTER' if OK    ");
    DialogStartNum(DISP_LINE1, 14, 5, DoChangeUAC, 0U, 0U, 65535U);
    }


static void DoChangeUAC(short ch)
  {
    switch (ch) {
      case K_RETURN:
        newUAC = atoi(DialogGetVal());
        SetupVerifyUAC();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupVerifyUAC(void)
  {
    DisplaySetLine(DISP_LINE1, 0, "New User Code       ");
    DisplaySetLine(DISP_LINE2, 0, " 'ENTER' to Verify  ");
    DialogStartNum(DISP_LINE1, 14, 5, DoVerifyUAC, 0U, 0U, 65535U);
    EventPost(EVT_KEY, K_RETURN);
    }


static void DoVerifyUAC(short ch)
  {
    switch (ch) {
      case K_RETURN:
        if (newUAC == atoi(DialogGetVal())) {
          HardwareWriteRAM(R_USER_CODE_R, 0, newUAC);
          SetupDefaultAccept();
          }
        else {
          DisplaySetLine(DISP_LINE2, 0, "Verification Failed ");
          EventDelay(100L);
          SetupChangeUAC();
          }
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupNodeNumber(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Node Number         ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Comm address of operator panel               ");
    val = HardwareReadRAM(R_NODE_NUMBER, 0);
    DialogStartNum(DISP_LINE1, 12, 2, DoChangeNodeNumber, val, 17U, 31U);
    }


static void DoChangeNodeNumber(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
      case K_DOWN:
        SetupBaudRate();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_NODE_NUMBER, 0, val);
        NetworkReOpen();
        SetupBaudRate();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupBaudRate(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Baud Rate           ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Use arrow keys to make a selection               ");
    val = HardwareReadRAM(R_BAUD_RATE, 0);
    DialogStartList(DISP_LINE1, 10, DoChangeBaudRate, val, 2, baudRateList);
    }


static void DoChangeBaudRate(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
      case K_DOWN:
        SetupNodeNumber();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_BAUD_RATE, 0, val);
        NetworkReOpen();
        SetupNodeNumber();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupEOCMaxFlow(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "EOC Max Flow        ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Maximum allowed flow of a pump at full speed               ");
    val = HardwareReadRAM(R_EOC_MAX_FLOW, 0);
    DialogStartNum(DISP_LINE1, 13, 5, DoChangeEOCMaxFlow, val, 0U, 10000U);
    }


static void DoChangeEOCMaxFlow(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupPIDLoopNum();
        break;
      case K_DOWN:
        SetupEOCDStage();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_EOC_MAX_FLOW, 0, val);
        SetupEOCDStage();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupEOCDStage(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "EOC D-Stg %Val      ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "% of flow at speed when D-Stage is OK               ");
    val = HardwareReadRAM(R_EOC_DSTAGE, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeEOCDStage, val, 655U, 65535U);
    }


static void DoChangeEOCDStage(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupEOCMaxFlow();
        break;
      case K_DOWN:
        SetupDStageVal01();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_EOC_DSTAGE, 0, val);
        SetupDStageVal01();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageVal01(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "1st D-Stg Val       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Stable % value to be reached to D-Stage               ");
    val = HardwareReadRAM(R_DSTAGE_VAL_01, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeDStageVal01, val, 655U, 65535U);
    }


static void DoChangeDStageVal01(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupEOCDStage();
        break;
      case K_DOWN:
        SetupDStageTim01();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_VAL_01, 0, val);
        SetupDStageTim01();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageTim01(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "1st D-Stg Pr.Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "After this time pump will D-Stage if at 1st D-Stage value               ");
    val = HardwareReadRAM(R_DSTAGE_TIM_01, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChangeDStageTim01, val, 30U, 1800U);
    }


static void DoChangeDStageTim01(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageVal01();
        break;
      case K_DOWN:
        SetupDStageVal02();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_TIM_01, 0, val);
        SetupDStageVal02();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageVal02(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "2nd D-Stg Val       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Stable % value to be reached to D-Stage               ");
    val = HardwareReadRAM(R_DSTAGE_VAL_02, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeDStageVal02, val, 655U, 65535U);
    }


static void DoChangeDStageVal02(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageTim01();
        break;
      case K_DOWN:
        SetupDStageTim02();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_VAL_02, 0, val);
        SetupDStageTim02();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageTim02(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "2nd D-Stg Pr.Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "After this time pump will D-Stage if at 2nd D-Stage value               ");
    val = HardwareReadRAM(R_DSTAGE_TIM_02, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChangeDStageTim02, val, 30U, 1800U);
    }


static void DoChangeDStageTim02(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageVal02();
        break;
      case K_DOWN:
        SetupDStageVal03();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_TIM_02, 0, val);
        SetupDStageVal03();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageVal03(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "3rd D-Stg Val       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Stable % value to be reached to D-Stage               ");
    val = HardwareReadRAM(R_DSTAGE_VAL_03, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeDStageVal03, val, 655U, 65535U);
    }


static void DoChangeDStageVal03(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageTim02();
        break;
      case K_DOWN:
        SetupDStageTim03();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_VAL_03, 0, val);
        SetupDStageTim03();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageTim03(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "3rd D-Stg Pr.Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "After this time pump will D-Stage if at 3rd D-Stage value               ");
    val = HardwareReadRAM(R_DSTAGE_TIM_03, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChangeDStageTim03, val, 30U, 1800U);
    }


static void DoChangeDStageTim03(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageVal03();
        break;
      case K_DOWN:
        SetupDStageVal04();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_TIM_03, 0, val);
        SetupDStageVal04();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageVal04(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "4th D-Stg Val       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Stable % value to be reached to D-Stage               ");
    val = HardwareReadRAM(R_DSTAGE_VAL_04, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeDStageVal04, val, 655U, 65535U);
    }


static void DoChangeDStageVal04(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageTim03();
        break;
      case K_DOWN:
        SetupDStageTim04();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_VAL_04, 0, val);
        SetupDStageTim04();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageTim04(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "4th D-Stg Pr.Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "After this time pump will D-Stage if at 4th D-Stage value               ");
    val = HardwareReadRAM(R_DSTAGE_TIM_04, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChangeDStageTim04, val, 30U, 1800U);
    }


static void DoChangeDStageTim04(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageVal04();
        break;
      case K_DOWN:
        SetupDStageVal05();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_TIM_04, 0, val);
        SetupDStageVal05();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageVal05(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "5th D-Stg Val       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Stable % value to be reached to D-Stage               ");
    val = HardwareReadRAM(R_DSTAGE_VAL_05, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeDStageVal05, val, 655U, 65535U);
    }


static void DoChangeDStageVal05(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageTim04();
        break;
      case K_DOWN:
        SetupDStageTim05();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_VAL_05, 0, val);
        SetupDStageTim05();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDStageTim05(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "5th D-Stg Pr.Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "After this time pump will D-Stage if at 5th D-Stage value               ");
    val = HardwareReadRAM(R_DSTAGE_TIM_05, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChangeDStageTim05, val, 30U, 1800U);
    }


static void DoChangeDStageTim05(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupDStageVal05();
        break;
      case K_DOWN:
        SetupPIDLoopNum();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_DSTAGE_TIM_05, 0, val);
        SetupPIDLoopNum();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupPIDLoopNum(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "PID Loop Number     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Enter value between 1 and 8               ");
    val = HardwareReadRAM(R_PID_LOOP_NUM, 0);
    DialogStartNum(DISP_LINE1, 16, 1, DoChangePIDLoopNum, val, 1U, 8U);
    }


static void DoChangePIDLoopNum(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupDStageTim05();
        break;
      case K_DOWN:
        SetupEOCMaxFlow();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_PID_LOOP_NUM, 0, atoi(DialogGetVal()));
        SetupEOCMaxFlow();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDefaultAccept(void)
  {
    DisplaySetLine(DISP_LINE1, 0, "Default Accept      ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Enter 'Yes' to download standard values as data for set up menu; use Ñ/Ö keys to select 'Yes' or 'No'               ");
    DialogStartList(DISP_LINE1, 15, DoChangeDefaultAccept, 1U, 2U, yesNoList);
    }


static void DoChangeDefaultAccept(short ch)
  {
    WORD  val;
    JIFF  timeOut;

    switch (ch) {
      case K_UP:
        SetupBypassEnabled();
        break;
      case K_DOWN:
        SetupPumpNumber();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal()) - 1;
        if (val == 1) {
          if (HardwareSerLink() == HS_CONNECT) {
            ButtonUp();
            HardwareWriteRAM(R_DEFAULT_ACCEP_R, 0, 1U);
            DisplaySetLine(DISP_LINE2, 0, "      Stand By      ");
            timeOut = EventJiffy() + 1000;
            while ((!EventElapsed(timeOut)) && (HardwareReadRAM(R_DEFAULT_ACCEP_W, 0)==0U)) {
              HardwareCommunicate();
              LEDUpdate();
              }
            DisplaySetLine(DISP_LINE2, 0, "Setting defaults... ");
            HardwareWriteRAM(R_DEFAULT_ACCEP_R, 0, 0U);
            timeOut = EventJiffy() + 1000;
            while ((!EventElapsed(timeOut)) && (HardwareReadRAM(R_DEFAULT_ACCEP_W, 0)==1U)) {
              HardwareCommunicate();
              LEDUpdate();
              }
            if (EventElapsed(timeOut)) {
              DisplayPush();
              DisplayShowMessage(DISP_LINE2, " No Acknowledgement ", 250L);
              DisplayPop();
              }
            }
          HardwareWriteRAM(R_EOC_MAX_FLOW, 0, 0U);
          HardwareWriteRAM(R_EOC_DSTAGE, 0, 39323U);
          HardwareWriteRAM(R_DSTAGE_VAL_01, 0, 55707U);
          HardwareWriteRAM(R_DSTAGE_TIM_01, 0, 300U);
          HardwareWriteRAM(R_DSTAGE_VAL_02, 0, 55707U);
          HardwareWriteRAM(R_DSTAGE_TIM_02, 0, 300U);
          HardwareWriteRAM(R_DSTAGE_VAL_03, 0, 55707U);
          HardwareWriteRAM(R_DSTAGE_TIM_03, 0, 300U);
          HardwareWriteRAM(R_DSTAGE_VAL_04, 0, 55707U);
          HardwareWriteRAM(R_DSTAGE_TIM_04, 0, 300U);
          HardwareWriteRAM(R_DSTAGE_VAL_05, 0, 55707U);
          HardwareWriteRAM(R_DSTAGE_TIM_05, 0, 300U);
          HardwareWriteRAM(R_PID_LOOP_NUM, 0, 1U);
          HardwareWriteRAM(R_NUM_PUMPS, 0, 0x0001U);
          HardwareWriteRAM(R_1ST_STAGE_TIM, 0, 600U);
          HardwareWriteRAM(R_2ND_STAGE_TIM, 0, 600U);
          HardwareWriteRAM(R_3RD_STAGE_TIM, 0, 600U);
          HardwareWriteRAM(R_4TH_STAGE_TIM, 0, 600U);
          HardwareWriteRAM(R_5TH_STAGE_TIM, 0, 600U);
          HardwareWriteRAM(R_AUTO_ALT_TIM, 0, 0U);
          HardwareWriteRAM(R_PROPORTIONAL, 0, 15U);
          HardwareWriteRAM(R_INTEGRAL, 0, 7U);
          HardwareWriteRAM(R_DERIVATIVE, 0, 3U);
          HardwareWriteRAM(R_PV_UNITS, 0, 1U);
          HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 40U);
          HardwareWriteRAM(R_FLOW_TRANS_SPAN, 0, 0U);
          HardwareWriteRAM(R_KW_TRANS_SPAN, 0, 0U);
          HardwareWriteRAM(R_NUM_ZONES, 0, 1U);
          HardwareWriteRAM(R_BYPASS_ENABLED, 0, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  0, 15U);
          HardwareWriteRAM(R_ZONE_01_SP,  1, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  2, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  3, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  4, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  5, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  6, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  7, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  8, 0U);
          HardwareWriteRAM(R_ZONE_01_SP,  9, 0U);
          HardwareWriteRAM(R_ZONE_01_SP, 10, 0U);
          HardwareWriteRAM(R_ZONE_01_SP, 11, 0U);
          HardwareWriteRAM(R_ZONE_01_SP, 12, 0U);
          HardwareWriteRAM(R_ZONE_01_SP, 13, 0U);
          HardwareWriteRAM(R_ZONE_01_SP, 14, 0U);
          HardwareWriteRAM(R_ZONE_01_SP, 15, 0U);
          }
        SetupPumpNumber();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupPumpNumber(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Number of Pumps     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Number of pumps to run together (1-6)               ");
    val = HardwareReadRAM(R_NUM_PUMPS, 0);
    DialogStartNum(DISP_LINE1, 16, 1, DoChangePumpNumber, val, 1U, 6U);
    }


static void DoChangePumpNumber(short ch)
  {
    WORD  val;
    short i;

    switch (ch) {
      case K_UP:
        SetupDefaultAccept();
        break;
      case K_DOWN:
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 1)
          Setup1stStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_NUM_PUMPS, 0, val);
        for (i=val+1; i<=6; i++)
          LEDSet(i, L_OFF);
        LEDDrawAll();
        HardwareWriteRAM(R_LEDA_STATUS, 0, 0);
        HardwareWriteRAM(R_LEDB_STATUS, 0, 0);
        if (val > 1)
          Setup1stStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void Setup1stStageTimer(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "1st Stg Stab Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "(Second*10) for staged pumps to stabilize               ");
    val = HardwareReadRAM(R_1ST_STAGE_TIM, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChange1stStageTimer, val, 60U, 6000U);
    }


static void DoChange1stStageTimer(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        SetupPumpNumber();
        break;
      case K_DOWN:
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 2)
          Setup2ndStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_1ST_STAGE_TIM, 0, val);
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 2)
          Setup2ndStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void Setup2ndStageTimer(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "2nd Stg Stab Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "(Second*10) for staged pumps to stabilize               ");
    val = HardwareReadRAM(R_2ND_STAGE_TIM, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChange2ndStageTimer, val, 60U, 6000U);
    }


static void DoChange2ndStageTimer(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        Setup1stStageTimer();
        break;
      case K_DOWN:
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 3)
          Setup3rdStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_2ND_STAGE_TIM, 0, val);
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 3)
          Setup3rdStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void Setup3rdStageTimer(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "3rd Stg Stab Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "(Second*10) for staged pumps to stabilize               ");
    val = HardwareReadRAM(R_3RD_STAGE_TIM, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChange3rdStageTimer, val, 60U, 6000U);
    }


static void DoChange3rdStageTimer(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        Setup2ndStageTimer();
        break;
      case K_DOWN:
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 4)
          Setup4thStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_3RD_STAGE_TIM, 0, val);
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 4)
          Setup4thStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void Setup4thStageTimer(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "4th Stg Stab Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "(Second*10) for staged pumps to stabilize               ");
    val = HardwareReadRAM(R_4TH_STAGE_TIM, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChange4thStageTimer, val, 60U, 6000U);
    }


static void DoChange4thStageTimer(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        Setup3rdStageTimer();
        break;
      case K_DOWN:
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 5)
          Setup5thStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_4TH_STAGE_TIM, 0, val);
        if (HardwareReadRAM(R_NUM_PUMPS, 0) > 5)
          Setup5thStageTimer();
        else
          SetupAutoAltTime();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void Setup5thStageTimer(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "5th Stg Stab Tm     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "(Second*10) for staged pumps to stabilize               ");
    val = HardwareReadRAM(R_5TH_STAGE_TIM, 0);
    DialogStartNum(DISP_LINE1, 16, 4, DoChange5thStageTimer, val, 60U, 6000U);
    }


static void DoChange5thStageTimer(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        Setup4thStageTimer();
        break;
      case K_DOWN:
        SetupAutoAltTime();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_5TH_STAGE_TIM, 0, val);
        SetupAutoAltTime();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupAutoAltTime(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Auto Alt. Prd.      ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "0=Manual only, for auto see table in IOM               ");
    val = HardwareReadRAM(R_AUTO_ALT_TIM, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeAutoAltTime, val, 0U, 60480U);
    }


static void DoChangeAutoAltTime(short ch)
  {
    WORD  val;

    switch (ch) {
      case K_UP:
        switch (HardwareReadRAM(R_NUM_PUMPS, 0)) {
          case 0:
          case 1:
            SetupPumpNumber();
            break;
          case 2:
            Setup1stStageTimer();
            break;
          case 3:
            Setup2ndStageTimer();
            break;
          case 4:
            Setup3rdStageTimer();
            break;
          case 5:
            Setup4thStageTimer();
            break;
          case 6:
            Setup5thStageTimer();
            break;
          }
        break;
      case K_DOWN:
        SetupProportional();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_AUTO_ALT_TIM, 0, val);
        SetupProportional();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupProportional(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Proportional        ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Corrective action as F(Error)               ");
    val = HardwareReadRAM(R_PROPORTIONAL, 0);
    DialogStartNum(DISP_LINE1, 13, 3, DoChangeProportional, val, 0U, 100U);
    }


static void DoChangeProportional(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupAutoAltTime();
        break;
      case K_DOWN:
        SetupIntegral();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_PROPORTIONAL, 0, atoi(DialogGetVal()));
        SetupIntegral();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupIntegral(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Integral            ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Corrective action as F(Error sum in time)               ");
    val = HardwareReadRAM(R_INTEGRAL, 0);
    DialogStartNum(DISP_LINE1, 9, 3, DoChangeIntegral, val, 0U, 100U);
    }


static void DoChangeIntegral(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupProportional();
        break;
      case K_DOWN:
        SetupDerivative();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_INTEGRAL, 0, atoi(DialogGetVal()));
        SetupDerivative();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupDerivative(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Derivative          ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Corrective action as F(Rate change of error)               ");
    val = HardwareReadRAM(R_DERIVATIVE, 0);
    DialogStartNum(DISP_LINE1, 11, 3, DoChangeDerivative, val, 0U, 100U);
    }


static void DoChangeDerivative(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupIntegral();
        break;
      case K_DOWN:
        SetupPVUnits();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_DERIVATIVE, 0, atoi(DialogGetVal()));
        SetupPVUnits();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupPVUnits(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "PV Units            ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "PSIÇ, PSI, ÉF, ÇÉF, ÉC, ÇÉC               ");
    val = HardwareReadRAM(R_PV_UNITS, 0);
    DialogStartList(DISP_LINE1, 9, DoChangePVUnits, val, 6, pvUnitsList);
    }


static void DoChangePVUnits(short ch)
  {
    WORD val;

    switch (ch) {
      case K_UP:
        SetupDerivative();
        break;
      case K_DOWN:
        SetupPVTransSpan();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        val = atoi(DialogGetVal());
        HardwareWriteRAM(R_PV_UNITS, 0, val);
        switch (val) {
          case 1:
            HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 40U);
            break;
          case 2:
            HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 150U);
            break;
          case 3:
            HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 100U);
            break;
          case 4:
            HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 50U);
            break;
          case 5:
            HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 38U);
            break;
          case 6:
            HardwareWriteRAM(R_PV_TRANS_SPAN, 0, 10U);
            break;
          }
        SetupPVTransSpan();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupPVTransSpan(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "PV Xmitter Span     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Set by 'UNITS'; must be equal to actual Xmitter calibration               ");
    val = HardwareReadRAM(R_PV_TRANS_SPAN, 0);
    DialogStartNum(DISP_LINE1, 16, 3, DoChangePVTransSpan, val, 0U, 999U);
    }


static void DoChangePVTransSpan(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupPVUnits();
        break;
      case K_DOWN:
        SetupFlowTransSpan();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_PV_TRANS_SPAN, 0, atoi(DialogGetVal()));
        SetupFlowTransSpan();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupFlowTransSpan(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "GPM Xmitr Span      ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Enter value equal to GPM transmitter calibration               ");
    val = HardwareReadRAM(R_FLOW_TRANS_SPAN, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeFlowTransSpan, val, 0U, 30000U);
    }


static void DoChangeFlowTransSpan(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupPVTransSpan();
        break;
      case K_DOWN:
        SetupKWTransSpan();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_FLOW_TRANS_SPAN, 0, atoi(DialogGetVal()));
        SetupKWTransSpan();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupKWTransSpan(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "kW Xmiter Span       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Enter value equal to KW transmitter calibration               ");
    val = HardwareReadRAM(R_KW_TRANS_SPAN, 0);
    DialogStartNum(DISP_LINE1, 15, 5, DoChangeKWTransSpan, val, 0U, 10000U);
    }


static void DoChangeKWTransSpan(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupFlowTransSpan();
        break;
      case K_DOWN:
        SetupNumZones();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_KW_TRANS_SPAN, 0, atoi(DialogGetVal()));
        SetupNumZones();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupNumZones(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Number of Zones     ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Need number equal to total PV transmitters               ");
    val = HardwareReadRAM(R_NUM_ZONES, 0);
    DialogStartNum(DISP_LINE1, 16, 2, DoChangeNumZones, val, 1U, 16U);
    }


static void DoChangeNumZones(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupKWTransSpan();
        break;
      case K_DOWN:
        SetupBypassEnabled();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_NUM_ZONES, 0, atoi(DialogGetVal()));
        SetupBypassEnabled();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }


static void SetupBypassEnabled(void)
  {
    WORD val;

    DisplaySetLine(DISP_LINE1, 0, "Bypass Enable       ");
    DisplayClearLine(DISP_LINE2);
    DisplayScrollLineOn(DISP_LINE2, "Yes=Bypass allowed, No=Bypass not allowed; use Ñ/Ö keys to select 'Yes' or 'No'               ");
    val = HardwareReadRAM(R_BYPASS_ENABLED, 0) + 1;
    DialogStartList(DISP_LINE1, 14, DoChangeBypassEnabled, val, 2U, yesNoList);
    }


static void DoChangeBypassEnabled(short ch)
  {
    switch (ch) {
      case K_UP:
        SetupNumZones();
        break;
      case K_DOWN:
        SetupDefaultAccept();
        break;
      case K_RETURN:
        DisplayScrollLineOff(DISP_LINE2);
        HardwareWriteRAM(R_BYPASS_ENABLED, 0, atoi(DialogGetVal())-1);
        SetupDefaultAccept();
        break;
      case K_CLEAR:
        DialogStop();
        ShiftStatePop();
        DisplayPop();
        break;
      }
    }
