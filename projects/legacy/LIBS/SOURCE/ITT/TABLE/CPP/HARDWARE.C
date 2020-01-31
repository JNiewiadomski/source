#include <ctype.h>
#include <dir.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <seashell\seashell.h>
#include <seashell\dialog.h>
#include <seashell\event.h>
#include <seashell\menu.h>
#include <seashell\message.h>
#include <seashell\video.h>
#include <seashell\window.h>

#include <common\misc.h>

#include "table.sym"
#include "table.h"
#include "datalink.h"
#include "hardware.h"
#include "network.h"

#define ComITEM      1
#define BaudITEM     2
#define AddITEM      4
#define ModifyITEM   5
#define DeleteITEM   6
#define BinaryITEM   8
#define DecimalITEM  9
#define HexITEM     10
#define DosITEM     12

#define REG_EXT  ".REG"

#define RAM_MAX    (8 * 1024)     /* 8K of memory */
#define RAM_OFFSET 1300

#define MAX_REGS  13

typedef enum { R_BINARY, R_DECIMAL, R_HEX } R_BASE;

static char          ram[RAM_MAX];
static NetworkRecord packet;
static JIFF          lastXmit=0L, errorTimeout=0L;

static short   vsRegs, vsComm, numRegs=0, curReg=0, nextReg, regInvHigh;
static WORD    regs[MAX_REGS];
static char    *regComments[MAX_REGS];
static R_BASE  baseReg;
static BOOLEAN dirty=FALSE;

static void   HardwareDosExecutive(void);
static short  HardwareGetRAM(WORD adr, BYTE len, char *data);
static char  *HardwareGetRegisterComment(WORD adr);
static void   HardwareRegsDisplay(void);
static void   HardwareRegsRead(void);
static void   HardwareRegsWrite(void);
static short  HardwareSetRAM(WORD adr, BYTE len, char *data);


void HardwareCleanUp(void)
  {
    HardwareRegsWrite();
    VirtualDispose(vsComm);
    VirtualDispose(vsRegs);
    NetworkCleanUp();
    }


void HardwareCommunicate(void)
  {
    switch (NetworkReceive(&packet)) {
      case NET_WRITE:
        if (HardwareSetRAM(packet.adr, packet.len, packet.data)) {
	  NetworkRespond(NET_WRITE, NULL);
	  errorTimeout = EventJiffy() + 800;
	  }
	lastXmit = EventJiffy() + 800;
        HardwareRegsDisplay();
	break;
      case NET_READ:
        if (HardwareGetRAM(packet.adr, packet.len, packet.data)) {
	  NetworkRespond(NET_READ, &packet);
	  errorTimeout = EventJiffy() + 800;
	  }
	lastXmit = EventJiffy() + 800;
        break;
      }
    HardwareSerLink();
    }


void HardwareDoMenu(short item)
  {
    switch (item) {
      case ComITEM:
        break;
      case BaudITEM:
        break;
      case AddITEM:
        HardwareRegsAdd();
        break;
      case ModifyITEM:
        HardwareRegsModify(-1);
        break;
      case DeleteITEM:
        HardwareRegsDelete();
        break;
      case BinaryITEM:
        baseReg = R_BINARY;
        break;
      case DecimalITEM:
        baseReg = R_DECIMAL;
        break;
      case HexITEM:
        baseReg = R_HEX;
        break;
      case DosITEM:
        HardwareDosExecutive();
        break;
      }
    }


static void HardwareDosExecutive(void)
  {
    Rect  r;
    short vsnum;

    RASG(&r, 0, 0, SCREEN_HEIGHT-1, SCREEN_WIDTH-1);
    vsnum = VirtualNew(SCREEN_HEIGHT, SCREEN_WIDTH, WNONE, &r, NULL, WHT);
    VirtualFill(vsnum, &GIANT_RECT, ' ', WHT);
    ScreenSynch();
    printf("Type EXIT to return to %s.", progName);
    VirtualShowCursor(vsnum, TRUE);
    system("\\command.com");
    VirtualDispose(vsnum);
    ScreenSynch();
    }


static short HardwareGetRAM(WORD adr, BYTE len, char *data)
  {
    if (adr<RAM_OFFSET || adr+len>=RAM_OFFSET+RAM_MAX)
      return (0);
    memmove(data, ram+adr-RAM_OFFSET, len);
    return (len);
    }


static char *HardwareGetRegisterComment(WORD adr)
  {
    switch (adr) {
      case R_NODE_NUMBER:
        return ("(R) Node Number");
      case R_BAUD_RATE:
        return ("(R) Baud Rate");
      case R_MASTER_CODE_R:
        return ("(R) Master Access Code");
      case R_EOC_MAX_FLOW:
        return ("(R) EOC Maximum Flow");
      case R_EOC_DSTAGE:
        return ("(R) EOC De-Stage %Value");
      case R_DSTAGE_VAL_01:
        return ("(R) 1st De-Stage Value");
      case R_DSTAGE_TIM_01:
        return ("(R) 1st De-Stage Pr. Timer");
      case R_DSTAGE_VAL_02:
        return ("(R) 2nd De-Stage Value");
      case R_DSTAGE_TIM_02:
        return ("(R) 2nd De-Stage Pr. Timer");
      case R_DSTAGE_VAL_03:
        return ("(R) 3rd De-Stage Value");
      case R_DSTAGE_TIM_03:
        return ("(R) 3rd De-Stage Pr. Timer");
      case R_DSTAGE_VAL_04:
        return ("(R) 4th De-Stage Value");
      case R_DSTAGE_TIM_04:
        return ("(R) 4th De-Stage Pr. Timer");
      case R_DSTAGE_VAL_05:
        return ("(R) 5th De-Stage Value");
      case R_DSTAGE_TIM_05:
        return ("(R) 5th De-Stage Pr. Timer");
      case R_PID_LOOP_NUM:
        return ("(R) PID Loop Number");
      case R_USER_CODE_R:
        return ("(R) User Access Code");
      case R_DEFAULT_ACCEP_R:
        return ("(R) Default Acceptance");
      case R_NUM_PUMPS:
        return ("(R) Number of Pumps to Stage");
      case R_1ST_STAGE_TIM:
        return ("(R) 1st Stage Stabilization Timer");
      case R_2ND_STAGE_TIM:
        return ("(R) 2nd Stage Stabilization Timer");
      case R_3RD_STAGE_TIM:
        return ("(R) 3rd Stage Stabilization Timer");
      case R_4TH_STAGE_TIM:
        return ("(R) 4th Stage Stabilization Timer");
      case R_5TH_STAGE_TIM:
        return ("(R) 5th Stage Stabilization Timer");
      case R_AUTO_ALT_TIM:
        return ("(R) Automatic Alternation Period");
      case R_PROPORTIONAL:
        return ("(R) Proportional");
      case R_INTEGRAL:
        return ("(R) Integral");
      case R_DERIVATIVE:
        return ("(R) Derivative");
      case R_PV_UNITS:
        return ("(R) PV Units");
      case R_PV_TRANS_SPAN:
        return ("(R) PV Transmitter Span");
      case R_FLOW_TRANS_SPAN:
        return ("(R) Flow Transmitter Span");
      case R_KW_TRANS_SPAN:
        return ("(R) kW Transmitter Span");
      case R_NUM_ZONES:
        return ("(R) Number of Zones");
      case R_BYPASS_ENABLED:
        return ("(R) Bypass Enabled");
      case R_ZONE_01_SP:
        return ("(R) Zone  1 SP");
      case 1542U:
        return ("(R) Zone  2 SP");
      case 1544U:
        return ("(R) Zone  3 SP");
      case 1546U:
        return ("(R) Zone  4 SP");
      case 1548U:
        return ("(R) Zone  5 SP");
      case 1550U:
        return ("(R) Zone  6 SP");
      case 1552U:
        return ("(R) Zone  7 SP");
      case 1554U:
        return ("(R) Zone  8 SP");
      case 1556U:
        return ("(R) Zone  9 SP");
      case 1558U:
        return ("(R) Zone 10 SP");
      case 1560U:
        return ("(R) Zone 11 SP");
      case 1562U:
        return ("(R) Zone 12 SP");
      case 1564U:
        return ("(R) Zone 13 SP");
      case 1566U:
        return ("(R) Zone 14 SP");
      case 1568U:
        return ("(R) Zone 15 SP");
      case 1570U:
        return ("(R) Zone 16 SP");
      case R_MANUAL_SPEED:
        return ("(R) Manual Speed Value");
      case R_FKEY_STATUS_R:
        return ("(R) Function Key Status");
      case R_KPAD_FLAG_R:
        return ("(R) Keypad Flag Register");
      case R_PV_01:
        return ("(W) PV  1");
      case 1640U:
        return ("(W) PV  2");
      case 1642U:
        return ("(W) PV  3");
      case 1644U:
        return ("(W) PV  4");
      case 1646U:
        return ("(W) PV  5");
      case 1648U:
        return ("(W) PV  6");
      case 1650U:
        return ("(W) PV  7");
      case 1652U:
        return ("(W) PV  8");
      case 1654U:
        return ("(W) PV  9");
      case 1656U:
        return ("(W) PV 10");
      case 1658U:
        return ("(W) PV 11");
      case 1660U:
        return ("(W) PV 12");
      case 1662U:
        return ("(W) PV 13");
      case 1664U:
        return ("(W) PV 14");
      case 1666U:
        return ("(W) PV 15");
      case 1668U:
        return ("(W) PV 16");
      case R_GPM_01_VAL:
        return ("(W) GPM1 Value");
      case 1672U:
        return ("(W) GPM2 Value");
      case 1674U:
        return ("(W) GPM3 Value");
      case 1676U:
        return ("(W) GPM4 Value");
      case R_KW_01_VAL:
        return ("(W) KW1 Value");
      case 1680U:
        return ("(W) KW2 Value");
      case 1682U:
        return ("(W) KW3 Value");
      case 1684U:
        return ("(W) KW4 Value");
      case 1686U:
        return ("(W) KW5 Value");
      case 1688U:
        return ("(W) KW6 Value");
      case R_SPEED_VALUE:
        return ("(W) Speed Value");
      case R_MASTER_CODE_W:
        return ("(W) Master Access Control");
      case R_ACTIVE_ZONE:
        return ("(W) Active Zone");
      case R_USER_CODE_W:
        return ("(W) User Access Code");
      case R_ACTIVE_SP:
        return ("(W) Active Setpoint");
      case R_ACTIVE_PV:
        return ("(W) Active Process Variable");
      case R_DEFAULT_ACCEP_W:
        return ("(W) Default Acceptance");
      case R_FKEY_STATUS_W:
        return ("(W) Function Key Status");
      case R_KPAD_FLAG_W:
        return ("(W) Keypad Flag Register");
      case R_ALT_SEQ_W:
        return ("(W) Alternation Sequence");
      case R_LEDA_STATUS:
        return ("(W) LED 1 - LED 4 Status");
      case R_LEDB_STATUS:
        return ("(W) LED 5 - LED 8 Status");
      case R_LEDC_STATUS:
        return ("(W) LED 9 - LED12 Status");
      case R_LEDD_STATUS:
        return ("(W) LED13 - LED16 Status");
      case R_PUMP_FAILURE:
        return ("(W) Pump1-4 Alarms");
      case 1746U:
        return ("(W) Pump5-6 Alarms,Remt,Pwr Sply");
      case R_ZONE_FAILURE:
        return ("(W) Zone Failures");
      case R_XMIT_CALIBRATE:
        return ("(W) Xmitters Calibrate");
      case R_FLOW_KW_FAILURE:
        return ("(W) 6KW,4GPM Failures,Battery Fail");
      default:
        return ("");
      }
    }


void HardwareInit(short port)
  {
    short height, width, attr;

    memset(ram, '\0', RAM_MAX);
    HardwareWriteRAM(R_NODE_NUMBER, 0, 17U);
    HardwareWriteRAM(R_BAUD_RATE, 0, 1U);
    HardwareWriteRAM(R_MASTER_CODE_W, 0, 54321U);
    NetworkInit(port);
    vsRegs = VirtualGetNew(RegsWINDOW, &height, &width, &attr);
    vsComm = VirtualGetNew(CommWINDOW, &height, &width, &attr);
    if (video.colorFlag)
      regInvHigh = ((CYN << 4) & REVERSE) | BWHT;
    else
      regInvHigh = 0x78;
    HardwareRegsRead();
    }


static void HardwareModifiedRAM(WORD adr, BYTE len)
  {
    }


void HardwareProcessChar(int ch)
  {
    if (numRegs < 1)
      return;
    switch (ch) {
      case K_SPACE:
        nextReg = (curReg + 1) % numRegs;
        break;
      case K_UP:
        if (curReg > 0)
          nextReg = curReg - 1;
        break;
      case K_DOWN:
        if (curReg < numRegs-1)
          nextReg = curReg + 1;
        break;
      case K_HOME:
        nextReg = 0;
        break;
      case K_END:
        nextReg = numRegs - 1;
        break;
      case K_INS:
        HardwareRegsAdd();
        break;
      case K_RETURN:
        HardwareRegsModify(-1);
        break;
      case K_DEL:
        HardwareRegsDelete();
        break;
      default:
        ch = toupper(ch);
        if (ch >= '0' && ch <= '9')
          HardwareRegsModify(ch);
        break;
      }
    HardwareRegsDisplay();
    }


WORD HardwareReadRAM(WORD adr, WORD offset)
  {
    char data[2];
    WORD val;

    if (offset)
      adr += offset * 2;
    if (adr < RAM_OFFSET || adr+1 >= RAM_OFFSET+RAM_MAX)
      return (0);
    adr -= RAM_OFFSET;
    *(data+1) = ram[adr++];
    *data = ram[adr];
    memmove(&val, data, 2);
    return (val);
    }


void HardwareRegsAdd(void)
  {
    short   vsAdd, height, width, attr, i;
    char    buff[5], *comment, *tempComment;
    BOOLEAN done;
    WORD    val, tempVal;

    if (numRegs >= MAX_REGS)
      return;
    vsAdd = VirtualGetNew(AddWINDOW, &height, &width, &attr);
    if (numRegs < 1)
      sprintf(buff, "%u", RAM_OFFSET);
    else {
      val = regs[curReg] + 2;
      for (i=curReg; i<numRegs && val>=regs[i]; i++)
        if (val == regs[i])
          val += 2;
      sprintf(buff, "%u", val);
      }
    done = FALSE;
    while (!done) {
      switch (DialogStringEditor(vsAdd, 0, 19, buff, 4)) {
        case K_ESC:
          done = TRUE;
          break;
        case K_RETURN:
          val = atoi(buff);
          if (val >= RAM_OFFSET && val+1 <= RAM_OFFSET+RAM_MAX && val%2==0) {
            comment = HardwareGetRegisterComment(val);
            for (i=0; i<numRegs && regs[i]<val; i++)
              ;
            if (i==numRegs || regs[i] != val) {
              nextReg = i;
              for ( ; i<numRegs; i++) {
                tempVal = regs[i];
                regs[i] = val;
                val = tempVal;
                tempComment = regComments[i];
                regComments[i] = comment;
                comment = tempComment;
                }
              regs[numRegs] = val;
              regComments[numRegs++] = comment;
              dirty = TRUE;
              }
            }
          done = TRUE;
          break;
        }
      }
    VirtualDispose(vsAdd);
    }


void HardwareRegsDelete(void)
  {
    short   vsDelete, height, width, attr, i;
    char    buff[5];
    BOOLEAN done;
    Rect    r;
    WORD    val;

    if (numRegs < 1)
      return;
    vsDelete = VirtualGetNew(DeleteWINDOW, &height, &width, &attr);
    sprintf(buff, "%u", regs[curReg]);
    done = FALSE;
    while (!done) {
      switch (DialogStringEditor(vsDelete, 0, 19, buff, 4)) {
        case K_ESC:
          done = TRUE;
          break;
        case K_RETURN:
          val = atoi(buff);
          if (val >= RAM_OFFSET && val+1 <= RAM_OFFSET+RAM_MAX && val%2==0) {
            for (i=0; i<numRegs && regs[i]!=val; i++)
              ;
            if (i < numRegs) {
              numRegs--;
              if (curReg >= numRegs)
                nextReg = numRegs - 1;
              for ( ; i<numRegs; i++) {
                regs[i] = regs[i+1];
                regComments[i] = regComments[i+1];
                }
              RASG(&r, 2+numRegs, 0, GIANT, GIANT);
              VirtualFill(vsRegs, &r, ' ', BLK);
              dirty = TRUE;
              }
            }
          done = TRUE;
          break;
        }
      }
    VirtualDispose(vsDelete);
    }


static void HardwareRegsDisplay(void)
  {
    short i, j, attr;
    WORD  adr, val;
    char  buff[20], *s;
    Rect  r;

    if (numRegs == 0)
      return;
    if (curReg != nextReg) {
      RASG(&r, 2+curReg, 0, 2+curReg, GIANT);
      VirtualFill(vsRegs, &r, ' ', BLK);
      curReg = nextReg;
      }
    for (i=0; i<numRegs; i++) {
      adr = regs[i];
      val = HardwareReadRAM(adr, 0);
      if (i == curReg) {
        RASG(&r, 2+curReg, 0, 2+curReg, GIANT);
        attr = regInvHigh;
        VirtualFill(vsRegs, &r, ' ', attr);
        }
      else
        attr = BWHT;
      VirtualPrint(vsRegs, i+2, 1, GIANT, attr, "R%04u    0x%04X    %5u", adr, val, val);
      s = buff;
      for (j=0; j<4; j++) {
        *s++ = (val & 0x8000) ? '1' : '0';
        *s++ = (val & 0x4000) ? '1' : '0';
        *s++ = (val & 0x2000) ? '1' : '0';
        *s++ = (val & 0x1000) ? '1' : '0';
        *s++ = ' ';
        val <<= 4;
        }
      *s = '\0';
      VirtualWrite(vsRegs, i+2, 27, GIANT, buff, attr);
      VirtualWrite(vsRegs, i+2, 48, 30, regComments[i], attr);
      }
    }


void HardwareRegsModify(int digit)
  {
    short   vsModify, type, len, height, width, attr, i;
    char    buff[17], *s;
    BOOLEAN done;
    WORD    adr, val;

    if (numRegs < 1)
      return;
    adr = regs[curReg];
    if (digit == -1) {
      val = HardwareReadRAM(adr, 0);
      switch (baseReg) {
        case R_BINARY:
          type = BinaryWINDOW;
          len = 16;
          s = buff;
          for (i=0; i<4; i++) {
            *s++ = (val & 0x8000) ? '1' : '0';
            *s++ = (val & 0x4000) ? '1' : '0';
            *s++ = (val & 0x2000) ? '1' : '0';
            *s++ = (val & 0x1000) ? '1' : '0';
            val <<= 4;
            }
          *s = '\0';
          break;
        case R_DECIMAL:
          type = DecimalWINDOW;
          len = 5;
          sprintf(buff, "%u", val);
          break;
        default:
          type = HexWINDOW;
          len = 4;
          sprintf(buff, "%X", val);
          break;
        }
      }
    else {
      switch (baseReg) {
        case R_BINARY:
          if (digit < '0' || digit > '1')
            return;
          type = BinaryWINDOW;
          len = 16;
          break;
        case R_DECIMAL:
          type = DecimalWINDOW;
          len = 5;
          break;
        default:
          type = HexWINDOW;
          len = 4;
          break;
        }
      *buff = digit;
      *(buff+1) = '\0';
      EventPost(EVT_KEY, K_END);
      }
    vsModify = VirtualGetNew(type, &height, &width, &attr);
    VirtualPrint(vsModify, 0, 10, GIANT, BWHT, "%u", adr);
    done = FALSE;
    while (!done) {
      switch (DialogStringEditor(vsModify, 0, 22, buff, len)) {
        case K_ESC:
          done = TRUE;
          break;
        case K_RETURN:
          switch (baseReg) {
            case R_BINARY:
              val = 0;
              s = buff;
              len = strlen(buff);
              for (i=0; i<len; i++) {
                val *= 2;
                if (*s++ == '1')
                  val += 1;
                }
              break;
            case R_DECIMAL:
              val = atoi(buff);
              break;
            default:
              sscanf(buff, "%X", &val);
              break;
            }
          HardwareWriteRAM(adr, 0, val);
          done = TRUE;
          break;
        }
      }
    VirtualDispose(vsModify);
    }


static void HardwareRegsRead(void)
  {
    char   name[MAXPATH];
    FILE  *fptr;
    short  i;

    sprintf(name, "%s%s", progName, REG_EXT);
    if ((fptr = fopen(name, "rb")) == NULL) {
      baseReg = R_HEX;
      numRegs = 0;
      }
    else {
      MessagePush("Reading %s...", name);
      fread(&baseReg, sizeof(baseReg), 1, fptr);
      fread(&numRegs, sizeof(numRegs), 1, fptr);
      if (numRegs > 0) {
        fread(&regs, sizeof(regs[0]), numRegs, fptr);
        for (i=0; i<numRegs; i++)
          regComments[i] = HardwareGetRegisterComment(regs[i]);
        }
      fclose(fptr);
      HardwareRegsDisplay();
      dirty = FALSE;
      MessagePop();
      }
    }


static void HardwareRegsWrite(void)
  {
    char name[MAXPATH];
    FILE *fptr;

    if (!dirty)
      return;
    sprintf(name, "%s%s", progName, REG_EXT);
    if ((fptr = fopen(name, "wb")) == NULL)
      return;
    MessagePush("Writing %s...", name);
    fwrite(&baseReg, sizeof(baseReg), 1, fptr);
    fwrite(&numRegs, sizeof(numRegs), 1, fptr);
    if (numRegs > 0)
      fwrite(&regs, sizeof(regs[0]), numRegs, fptr);
    fclose(fptr);
    dirty = FALSE;
    MessagePop();
    }


HS_STATE HardwareSerLink(void)
  {
    if (EventElapsed(lastXmit)) {
      VirtualPrint(vsComm, 0, 1, GIANT, BWHT, "Offline");
      return (HS_OFFLINE);
      }
    if (EventElapsed(errorTimeout)) {
      VirtualPrint(vsComm, 0, 1, GIANT, BWHT, "Error  ");
      return (HS_ERROR);
      }
    VirtualPrint(vsComm, 0, 1, GIANT, BWHT, "Online ");
    return (HS_CONNECT);
    }


static short HardwareSetRAM(WORD adr, BYTE len, char *data)
  {
    if (adr<RAM_OFFSET || adr+len>=RAM_OFFSET+RAM_MAX)
      return (0);
    memmove(ram+adr-RAM_OFFSET, data, len);
    HardwareModifiedRAM(adr, len);
    return (len);
    }


void HardwareUpdateMenu(void)
  {
    if (numRegs > 0) {
      if (numRegs < MAX_REGS)
        MenuEnableItem(SystemMenuID, AddITEM);
      else
        MenuDisableItem(SystemMenuID, AddITEM);
      MenuEnableItem(SystemMenuID, ModifyITEM);
      MenuEnableItem(SystemMenuID, DeleteITEM);
      }
    else {
      MenuEnableItem(SystemMenuID, AddITEM);
      MenuDisableItem(SystemMenuID, ModifyITEM);
      MenuDisableItem(SystemMenuID, DeleteITEM);
      }
    MenuCheckItem(SystemMenuID, BinaryITEM, baseReg==R_BINARY);
    MenuCheckItem(SystemMenuID, DecimalITEM, baseReg==R_DECIMAL);
    MenuCheckItem(SystemMenuID, HexITEM, baseReg==R_HEX);
    }


short HardwareWriteRAM(WORD adr, WORD offset, WORD val)
  {
    char  data[2];

    if (offset)
      adr += offset * 2;
    if (adr < RAM_OFFSET || adr+1 >= RAM_OFFSET+RAM_MAX)
      return (0);
    memmove(data, &val, 2);
    adr -= RAM_OFFSET;
    ram[adr++] = *(data+1);
    ram[adr] = *data;
    return (2);
    }
