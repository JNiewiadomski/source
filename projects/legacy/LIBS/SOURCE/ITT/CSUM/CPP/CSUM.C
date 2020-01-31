/* datalink.c -- February 5, 1990 */

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <mem.h>

#include <..\seachest\seachest.h>
#include <..\seashell\keyboard.h>

#include "..\datalink.h"

#define STX 0x02

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

static BYTE    DataLinkCheckSum(short len, BYTE *buff);
static BYTE    DataLinkTransmitError(BYTE err);


main()
  {
    BOOLEAN done;
    BYTE ch, buff[512], *ptr;
    short i, len;

    done = FALSE;
    while (!done) {
      printf("Enter data following STX: ");
      ptr = buff;
      *ptr++ = STX;
      printf("%2Xh ", STX);
      *ptr++ = ch = KeyboardRead();
      printf("%2Xh ", ch);
      len = ch;
      i = len-3;
      while (i--) {
        *ptr++ = ch = KeyboardRead();
        printf("%2Xh ", ch);
        }
      printf("\n");
      ch = DataLinkCheckSum(len-1, buff);
      printf("Checksum = %2Xh.\n\n", ch);
      }
    return (0);
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


static BYTE DataLinkTransmitError(BYTE err)
  {
    BYTE *s, ch;

    s = dlink.buff;
    *s++ = STX;
    *s++ = 6;
    *s++ = DLINK_BROADCAST;
    *s++ = err;
    *s++ = DLINK_MASTER;
    *s = ch = DataLinkCheckSum(5, dlink.buff);
    return (ch);
    }
