/* datalink.c -- February 5, 1990 */

#define DEBUG(err) ErrorStatus(err);

/* Data Link Layer for Serial Communication */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   10-JAN-1990
 * ----------------------------------------------------------------------- */

#include <dos.h>
#include <mem.h>

#include <..\seachest\seachest.h>
#include <..\seachest\serial.h>

#include <..\comm\gf.h>
#include <..\comm\asiports.h>

#include "datalink.h"

#define DLINK_MODE        (ASINOUT | BINARY | NORMALRX)

#define DLINK_INDEX_BAD   0x00     /* illegal index value */
#define DLINK_INDEX_MIN   0x40     /* minimum index value */
#define DLINK_INDEX_INIT  0x41     /* initial index value */
#define DLINK_INDEX_MAX   0x7F     /* maximum index value */

#define DLINK_RESET       'R'
#define DLINK_BROADCAST   0x10
#define DLINK_MASTER      0x80

#define ERR_RESET         0xC0     /* reset done */
#define ERR_INDEX         0xC1     /* index error */
#define ERR_DONE          0xC8     /* instruction done */

#define PACK_STX          0x00
#define PACK_LEN          0x01
#define PACK_NODE         0x02
#define PACK_IDX          0x03
#define PACK_DATA         0x04

#define MIN_PACKET        5

#define ESC               0x1B

typedef struct {
  int  port;                   /* communications port in use */
  BYTE node;                   /* node number of O/I panel */
  BYTE oldIndex;               /* index of previous packet */
  BYTE curIndex;               /* index of current packet */
  BYTE len;                    /* current length of packet received */
  BYTE buff[DLINK_BUF_LEN];    /* packet buffer */
  WORD getting : 1;            /* are we currently receiving data ? */
  WORD on      : 1;            /* is com port turned on ? */
  } DataLinkRecord;

static DataLinkRecord dlink;


static BOOLEAN DataLinkAvailable(void);
static BYTE    DataLinkCheckSum(short len, BYTE *buff);
static void    DataLinkSend(short len, BYTE *data);
static void    DataLinkTransmitError(BYTE err);


#ifdef DEBUG
  #include <graphics.h>
  #include <stdio.h>

  #include <..\seachest\graphics.h>
  #include <..\seachest\mouse.h>

  #define ERR_CHKSUM        0x00

  static short errCount=0;

  static void ErrorStatus(BYTE err);

  static void ErrorStatus(BYTE err)
    {
      GrafPort *old, new;
      char     buff[50];

      old = GraphicsGetPort();
      GraphicsNewPort(&new);
      GraphicsForeColor(WHITE);
      GraphicsFillColor(BLACK);
      GraphicsTextSize(5);
      switch (err) {
        case ERR_RESET:
          sprintf(buff, "Reset (%d)", ++errCount);
          break;
        case ERR_INDEX:
          sprintf(buff, "Invalid Index (%d)", ++errCount);
          break;
        case ERR_DONE:
          sprintf(buff, "Instruction Done (%d)", ++errCount);
          break;
        case ERR_CHKSUM:
          sprintf(buff, "Checksum (%d)", ++errCount);
          break;
        default:
          sprintf(buff, "Unknown - 0x%02X (%d)", err, ++errCount);
          break;
        }
      mouseHideCursor();
      bar(0, 0, 200, 20);
      outtextxy(0, 2, buff);
      mouseShowCursor();
      GraphicsSetPort(old);
      }
#endif


void DataLinkAck(void)
  {
    asrts(dlink.port, ON);
    asiputc(dlink.port, ACK);
    s_tillTXEmpty(dlink.port);
    asrts(dlink.port, OFF);
    asiclear(dlink.port, ASIN);
    }


static BOOLEAN DataLinkAvailable(void)
  {
    int ch;

    if (!dlink.getting) {
      if (getrxcnt(dlink.port) < 2)
        return (FALSE);
      if ((dlink.buff[PACK_STX] = asigetc(dlink.port)) != STX)
        return (FALSE);
      if ((dlink.buff[PACK_LEN] = asigetc(dlink.port)) < MIN_PACKET)
        return (FALSE);
      dlink.len = 2;
      dlink.getting = TRUE;
      }
    while (dlink.getting) {
      if (dlink.len == dlink.buff[PACK_LEN])
        return (TRUE);
      if (!getrxcnt(dlink.port))
        return (FALSE);
      if ((dlink.buff[dlink.len++] = asigetc(dlink.port)) == STX) {
        while (!getrxcnt(dlink.port))
          ;
        ch = asigetc(dlink.port);
        if (ch >= MIN_PACKET) {
          dlink.buff[PACK_LEN] = ch;
          dlink.len = 2;
          }
        else if (ch)
          dlink.getting = FALSE;
        }
      }
    return (FALSE);
    }


static BYTE DataLinkCheckSum(short len, BYTE *buff)
  {
    register WORD chk, i;

    chk = 0;
    for (i=len; i; i--) {
      chk <<= 1;
      chk = (chk & 0x0100) ? ((BYTE)chk) + 1 : chk;
      chk += *buff++;
      chk = (chk & 0x0100) ? ((BYTE)chk) + 1 : chk;
      }
    return (chk == STX ? 0xFD : chk);
    }


void DataLinkCleanUp(void)
  {
    if (dlink.on) {
      asiquit(dlink.port);
      dlink.on = FALSE;
      }
    }


BOOLEAN DataLinkInit(int port, int baud, BYTE node)
  {
    s_initBase();
    dlink.port = port;
    dlink.node = node;
    dlink.oldIndex = DLINK_INDEX_BAD;
    dlink.curIndex = DLINK_INDEX_INIT;
    dlink.getting = FALSE;
    dlink.on = asiopen(port, DLINK_MODE, DLINK_BUF_LEN, DLINK_BUF_LEN, baud, P_NONE, 1, 8, OFF, OFF) == ASSUCCESS;
    return (dlink.on);
    }


short DataLinkReceive(char *data)
  {
    if (!DataLinkAvailable())
      return (DLINK_NONE);
    dlink.getting = FALSE;
    if (dlink.buff[PACK_NODE] != dlink.node)
      return (DLINK_NONE);
    if (dlink.buff[dlink.len-1] != DataLinkCheckSum(dlink.len-1, dlink.buff)) {
DEBUG(ERR_CHKSUM)
      return (DLINK_ERROR);
      }
    if (dlink.buff[PACK_DATA]==ESC && dlink.buff[PACK_DATA+1]==DLINK_RESET) {
      DataLinkTransmitError(ERR_RESET);
      dlink.oldIndex = DLINK_INDEX_BAD;
      dlink.curIndex = DLINK_INDEX_INIT;
      return (DLINK_ERROR);
      }
    if (dlink.buff[PACK_IDX] == dlink.oldIndex) {
      DataLinkTransmitError(ERR_DONE);
      return (DLINK_ERROR);
      }
    if (dlink.buff[PACK_IDX] != dlink.curIndex) {
      DataLinkTransmitError(ERR_INDEX);
      return (DLINK_ERROR);
      }
    dlink.oldIndex = dlink.curIndex;
    dlink.curIndex = dlink.curIndex==DLINK_INDEX_MAX ? DLINK_INDEX_MIN : dlink.curIndex+1;
    dlink.len -= PACK_DATA + 1;   /* disclude checksum too */
    memmove(data, dlink.buff+PACK_DATA, dlink.len);
    return (dlink.len);
    }


static void DataLinkSend(short len, BYTE *data)
  {
    short i;
    BYTE  ch;

    asrts(dlink.port, ON);
    ch = *data++;
    asiputc(dlink.port, ch);
    for (i=0; i<len; i++) {
      ch = *data++;
      asiputc(dlink.port, ch);
      if (ch == STX)
        asiputc(dlink.port, 0x0000);
      }
    s_tillTXEmpty(dlink.port);
    asrts(dlink.port, OFF);
    asiclear(dlink.port, ASIN);
    }


void DataLinkTransmit(short len, char *data)
  {
    BYTE  *s;
    short i;

    s = dlink.buff;
    *s++ = STX;
    *s++ = len + 6;
    *s++ = DLINK_BROADCAST;
    *s++ = dlink.curIndex;
    *s++ = DLINK_MASTER;
    for (i=len; i; i--)
      *s++ = *data++;
    *s = DataLinkCheckSum(len+5, dlink.buff);
    DataLinkCheckSum(len+5, dlink.buff);
    DataLinkSend(len+6, dlink.buff);
    }


static void DataLinkTransmitError(BYTE err)
  {
    BYTE *s;

    sound(1500);
    s = dlink.buff;
    *s++ = STX;
    *s++ = 6;
    *s++ = DLINK_BROADCAST;
    *s++ = err;
    *s++ = DLINK_MASTER;
    *s = DataLinkCheckSum(5, dlink.buff);
    DataLinkSend(6, dlink.buff);
    nosound();
DEBUG(err)
    }
