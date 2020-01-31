/* Network Layer for Serial Communication */

#include <mem.h>

#include <seashell\seashell.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "datalink.h"
#include "hardware.h"
#include "network.h"

#define NET_COMMAND 0
#define NET_LEN     1
#define NET_ADR_HI  2
#define NET_ADR_LO  3
#define NET_DATA    4

static char  netBuf[DLINK_BUF_LEN];
static int   netPort;


void NetworkCleanUp(void)
  {
    DataLinkCleanUp();
    }


void NetworkInit(short port)
  {
    int  baud;
    WORD node;

    baud = HardwareReadRAM(R_BAUD_RATE, 0) == 1 ? 9600 : 1200;
    node = HardwareReadRAM(R_NODE_NUMBER, 0);
    netPort = port;
    DataLinkInit(netPort==1 ? COM1 : COM2, baud, node);
    }


NET_CMD NetworkReceive(NetworkRecord *packet)
  {
    NET_CMD cmd;
    char    temp[2];

    if (!packet || DataLinkReceive(netBuf)<=DLINK_NONE)
      return (NET_NONE);
    cmd = *(netBuf + NET_COMMAND);
    if (cmd!=NET_WRITE && cmd!=NET_READ)
      return (NET_NONE);
    packet->len = *(netBuf+NET_LEN);
    *(temp+1) = *(netBuf + NET_ADR_HI);
    *temp = *(netBuf + NET_ADR_LO);
    memmove(&(packet->adr), temp, 2);
    if (cmd == NET_WRITE)
      memmove(packet->data, netBuf+NET_DATA, packet->len);
    return (cmd);
    }


void NetworkReOpen(void)
  {
    NetworkCleanUp();
    NetworkInit(netPort);
    }


void NetworkRespond(NET_CMD cmd, NetworkRecord *packet)
  {
    switch (cmd) {
      case NET_WRITE:
        DataLinkAck();
	break;
      case NET_READ:
        DataLinkTransmit(packet->len, packet->data);
	break;
      }
    }
