/* hardware.c -- February 5, 1989 */

/* Hardware Simulator */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   28-DEC-1989
 * ----------------------------------------------------------------------- */

#include <mem.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>

#include "datalink.h"
#include "hardware.h"
#include "network.h"

#define RAM_MAX    (8 * 1024)     /* 8K of memory */
#define RAM_OFFSET 1300

HWRecord hwUpdate;

static char          ram[RAM_MAX];
static NetworkRecord packet;
static JIFF          lastXmit=0L, errorTimeout=0L;


static short HardwareGetRAM(WORD adr, BYTE len, char *data);
static void  HardwareModifiedRAM(WORD adr, BYTE len);
static short HardwareSetRAM(WORD adr, BYTE len, char *data);


void HardwareCleanUp(void)
  {
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
        break;
      case NET_READ:
        if (HardwareGetRAM(packet.adr, packet.len, packet.data)) {
          NetworkRespond(NET_READ, &packet);
          errorTimeout = EventJiffy() + 800;
          }
        lastXmit = EventJiffy() + 800;
        break;
      }
    }


static short HardwareGetRAM(WORD adr, BYTE len, char *data)
  {
    if (adr<RAM_OFFSET || adr+len>=RAM_OFFSET+RAM_MAX)
      return (0);
    memmove(data, ram+adr-RAM_OFFSET, len);
    return (len);
    }


void HardwareInit(short port)
  {
    memset(ram, '\0', RAM_MAX);
    HardwareWriteRAM(R_NODE_NUMBER, 0, 17U);
    HardwareWriteRAM(R_BAUD_RATE, 0, 1U);
    HardwareWriteRAM(R_MASTER_CODE_W, 0, 54321U);
    NetworkInit(port);
    hwUpdate.led = FALSE;
    hwUpdate.fkey = FALSE;
    hwUpdate.kpad = FALSE;
    hwUpdate.pv = FALSE;
    hwUpdate.write = FALSE;
    }


static void HardwareModifiedRAM(WORD adr, BYTE len)
  {
    WORD adrHi, val;

    adrHi = adr + len;
    if (adr <= R_LEDC_STATUS && adrHi >= R_LEDA_STATUS)
      hwUpdate.led = TRUE;
    if (adr <= R_FKEY_STATUS_W && adrHi >= R_FKEY_STATUS_W)
      hwUpdate.fkey = TRUE;
    if (adr <= R_KPAD_FLAG_W && adrHi >= R_KPAD_FLAG_W)
      hwUpdate.kpad = TRUE;
    val = R_PV_01 + ((HardwareReadRAM(R_NUM_ZONES, 0)*2)-1);
    if (adr <= val && adrHi >= R_PV_01)
      hwUpdate.pv = TRUE;
    hwUpdate.write = TRUE;
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


HS_STATE HardwareSerLink(void)
  {
    if (EventElapsed(lastXmit))
      return (HS_OFFLINE);
    if (EventElapsed(errorTimeout))
      return (HS_ERROR);
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