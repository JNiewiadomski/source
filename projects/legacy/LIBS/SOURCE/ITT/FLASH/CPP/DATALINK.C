/* Data Link Layer for Flash reprogramming */

#include <dos.h>
#include <mem.h>

#include <common\common.h>
#include <common\asynch.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "datalink.h"

#define DLINK_MODE	(ASINOUT | BINARY | NORMALRX)

#define DLINK_INDEX_BAD		0x00	/* illegal index value */
#define DLINK_INDEX_MIN		0x40	/* minimum index value */
#define DLINK_INDEX_INIT	0x41	/* initial index value */
#define DLINK_INDEX_MAX		0x7F	/* maximum index value */

#define DLINK_RESET		'R'
#define DLINK_BROADCAST		0x10
#define DLINK_MASTER		0x80

#define MIN_PACKET	5

#define ESC		0x1B

#define DEF_BAUD	9600
#define DEF_NODE	17

typedef struct {
	int	port;			/* communications port in use */
	Byte	node;			/* node number of O/I panel */
	Byte	oldIndex;		/* index of previous packet */
	Byte	curIndex;		/* index of current packet */
	Byte	len;			/* current length of packet received */
	Byte	buff[DLINK_BUF_LEN];	/* packet buffer */
	Word	getting	: 1;		/* are we currently receiving data ? */
	Word	stx	: 1;		/* have we seen initial STX ? */
	Word	on	: 1;		/* is com port turned on ? */
	} DataLinkRecord;

static DataLinkRecord	dlink;


static Boolean	DataLinkAvailable(void);
static Byte	DataLinkCheckSum(short len, Byte *buff);
static void	DataLinkSend(short len, Byte *data);


static Boolean DataLinkAvailable(void) {
	int	ch;

	if (!dlink.getting) {
		if (!dlink.stx) {
			if (getrxcnt(dlink.port) < 1)
				return (false);
			dlink.buff[0] = asigetc(dlink.port);
			switch (dlink.buff[0]) {
				case ACK:
					dlink.len = 1;
					return (true);
				case STX:
					dlink.len = 1;
					dlink.stx = true;
					break;
				default:
					return (false);
				}
			}
		if (getrxcnt(dlink.port) < 1)
			return (false);
		if ((dlink.buff[PACK_LEN] = asigetc(dlink.port)) < MIN_PACKET) {
			dlink.stx = false;
			return (false);
			}
		dlink.len = 2;
		dlink.getting = true;
		dlink.stx = false;
		}
	while (dlink.getting) {
		if (dlink.len == dlink.buff[PACK_LEN])
			return (true);
		if (!getrxcnt(dlink.port))
			return (false);
		if ((dlink.buff[dlink.len++] = asigetc(dlink.port)) == STX) {
			while (!getrxcnt(dlink.port))
				;
			ch = asigetc(dlink.port);
			if (ch >= MIN_PACKET) {
				dlink.buff[PACK_LEN] = ch;
				dlink.len = 2;
				}
			else if (ch)
				dlink.getting = false;
			}
		}
	return (false);
	}


static Byte DataLinkCheckSum(short len, Byte *buff) {
	register Word	chk, i;

	chk = 0;
	for (i=len; i; i--) {
		chk <<= 1;
		chk = (chk & 0x0100) ? ((Byte)chk) + 1 : chk;
		chk += *buff++;
		chk = (chk & 0x0100) ? ((Byte)chk) + 1 : chk;
		}
	return (chk == STX ? 0xFD : chk);
	}


void DataLinkCleanUp(void) {
	if (dlink.on) {
		asiquit(dlink.port);
		dlink.on = false;
		}
	}


Boolean DataLinkInit(short port) {
	dlink.port = port;
	dlink.node = DEF_NODE;
	dlink.oldIndex = DLINK_INDEX_BAD;
	dlink.curIndex = DLINK_INDEX_INIT;
	dlink.getting = false;
	dlink.stx = false;
	dlink.on = asiopen(port, DLINK_MODE, DLINK_BUF_LEN, DLINK_BUF_LEN, DEF_BAUD, P_NONE, 1, 8, OFF, OFF) == ASSUCCESS;
	return (dlink.on);
	}


short DataLinkReceive(char *data) {
	if (!DataLinkAvailable())
		return (DLINK_NONE);
	dlink.getting = false;
	if (dlink.len == 1 && *(dlink.buff) == ACK)
		return (DLINK_ACK);
	if (dlink.buff[PACK_NODE] != DLINK_BROADCAST)
		return (DLINK_ERROR);
	if (dlink.buff[dlink.len-1] != DataLinkCheckSum(dlink.len-1, dlink.buff))
		return (DLINK_ERROR);
	if (dlink.buff[PACK_DATA] != DLINK_MASTER)
		return (DLINK_ERROR);
	memmove(data, dlink.buff, dlink.len);
	return (dlink.len);
	}


void DataLinkReset(void) {
	Byte	*s;

	s = dlink.buff;
	*s++ = STX;
	*s++ = 7;
	*s++ = dlink.node;
	*s++ = dlink.curIndex;
	*s++ = ESC;
	*s++ = DLINK_RESET;
	*s = DataLinkCheckSum(6, dlink.buff);
	DataLinkSend(7, dlink.buff);
	dlink.oldIndex = DLINK_INDEX_BAD;
	dlink.curIndex = DLINK_INDEX_INIT;
	}


static void DataLinkSend(short len, Byte *data) {
	short	i;
	Byte	ch;

	asrts(dlink.port, ON);
	ch = *data++;
	asiputc(dlink.port, ch);
	for (i=0; i<len; i++) {
		ch = *data++;
		asiputc(dlink.port, ch);
		if (ch == STX)
			asiputc(dlink.port, 0x0000);
		}
	AsynchTillTXEmpty(dlink.port);
	asrts(dlink.port, OFF);
	asiclear(dlink.port, ASIN);
	}


void DataLinkTransmit(short len, char *data) {
	Byte	*s;
	short	i;

	s = dlink.buff;
	*s++ = STX;
	*s++ = len + 5;
	*s++ = dlink.node;
	*s++ = dlink.curIndex;
	for (i=len; i; i--)
		*s++ = *data++;
	*s = DataLinkCheckSum(len+4, dlink.buff);
	DataLinkSend(len+5, dlink.buff);
	dlink.oldIndex = dlink.curIndex;
	dlink.curIndex = dlink.curIndex==DLINK_INDEX_MAX ? DLINK_INDEX_MIN : dlink.curIndex+1;
	}
