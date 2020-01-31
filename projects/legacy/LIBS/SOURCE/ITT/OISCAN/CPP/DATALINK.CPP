// Data Link Layer for Serial Communication

#include <dos.h>
#include <mem.h>
#include <stdarg.h>
#include <stdio.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\asynch.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "datalink.h"
#include "oiscan.h"

#define DLINK_MODE		(ASINOUT | BINARY | NORMALRX)

#define DLINK_INDEX_BAD		0x00     // illegal index value
#define DLINK_INDEX_MIN		0x40     // minimum index value
#define DLINK_INDEX_INIT	0x41     // initial index value
#define DLINK_INDEX_MAX		0x7F     // maximum index value

#define DLINK_RESET		'R'
#define DLINK_BROADCAST		0x10
#define DLINK_PANEL		0x11
#define DLINK_MASTER		0x80

#define ERR_RESET		0xC0     // reset done
#define ERR_INDEX		0xC1     // index error
#define ERR_DONE		0xC8     // instruction done

#define PACK_STX		0x00
#define PACK_LEN		0x01
#define PACK_NODE		0x02
#define PACK_IDX		0x03
#define PACK_DATA		0x04

#define MIN_PACKET		5

typedef struct {
	int	port;			// communications port in use
	Byte	node;			// node number of O/I panel
	Byte	oldIndex;		// index of previous packet
	Byte	curIndex;		// index of current packet
	Byte	len;			// current length of packet received
	Byte	buff[DLINK_BUF_LEN];	// packet buffer
	Word	getting	: 1;		// are we currently receiving data ?
	Word	on	: 1;		// is com port turned on ?
	} DataLinkRecord;

static DataLinkRecord	dlink;
static short		errCSum=0, errReset=0, errIndex=0, errDone=0;
static long		far *ram_timer;

static Boolean DataLinkAvailable(void);
static Byte    DataLinkCheckSum(short len, Byte *buff);
static void	DataLinkReport(char *format, ...);


static Boolean DataLinkAvailable(void) {
	int	ch;

	if (!dlink.getting) {
		if (getrxcnt(dlink.port) > 0 && asipeek(dlink.port) == ACK) {
			dlink.buff[0] = asigetc(dlink.port);
			dlink.len = 1;
			return (true);
			}
		if (getrxcnt(dlink.port) < 2)
			return (false);
		if ((dlink.buff[PACK_STX] = asigetc(dlink.port)) != STX)
			return (false);
		if ((dlink.buff[PACK_LEN] = asigetc(dlink.port)) < MIN_PACKET)
			return (false);
		dlink.len = 2;
		dlink.getting = true;
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


Boolean DataLinkInit(int port, int baud, Byte node) {
	ram_timer = (long *) MK_FP(0x0000, 0x046C);
	dlink.port = port;
	dlink.node = node;
	dlink.oldIndex = DLINK_INDEX_BAD;
	dlink.curIndex = DLINK_INDEX_INIT;
	dlink.getting = false;
	dlink.on = asiopen(port, DLINK_MODE, DLINK_BUF_LEN*16, DLINK_BUF_LEN, baud, P_NONE, 1, 8, OFF, OFF) == ASSUCCESS;
	return ((Boolean)dlink.on);
	}


short DataLinkReceive(void) {
	short	i;
	char	*node;

	if (!DataLinkAvailable())
		return (DLINK_NONE);
	dlink.getting = false;
	DataLinkReport("\nPacket: [%u]\n", (int)*ram_timer);
	for (i=0; i<dlink.len; i++)
		DataLinkReport("%02Xh ", dlink.buff[i]);
	DataLinkReport("\n");
	if (dlink.len == 1 && dlink.buff[0] == ACK)
		DataLinkReport("Acknowledge\n");
	else {
		switch (dlink.buff[PACK_NODE]) {
			case DLINK_BROADCAST:
				node = "From O/I Panel";
				break;
			case DLINK_PANEL:
				node = "From PLC";
				break;
			default:
				node = "From Unknown";
				break;
			}
		if (dlink.buff[PACK_NODE] != dlink.node) {
			DataLinkReport("%s: Packet ignored.\n", node);
			return (DLINK_NONE);
			}
		if (dlink.buff[dlink.len-1] != DataLinkCheckSum(dlink.len-1, dlink.buff)) {
			DataLinkReport("Error: Check Sum (%d)\n", ++errCSum);
			return (DLINK_ERROR);
			}
		if (dlink.buff[PACK_DATA]==A_ESC && dlink.buff[PACK_DATA+1]==DLINK_RESET) {
			dlink.oldIndex = DLINK_INDEX_BAD;
			dlink.curIndex = DLINK_INDEX_INIT;
			DataLinkReport("%s: Reset Done (%d)\n", node, ++errReset);
			return (DLINK_ERROR);
			}
		if (dlink.buff[PACK_IDX] == dlink.oldIndex) {
			DataLinkReport("%s: Index error - %02Xh already used (%d)\n", node, dlink.oldIndex, ++errDone);
			return (DLINK_ERROR);
			}
		if (dlink.buff[PACK_IDX] != dlink.curIndex) {
			DataLinkReport("%s: Instruction done - expecting index %02Xh (%d)\n", node, dlink.curIndex, ++errIndex);
			return (DLINK_ERROR);
			}
		switch (dlink.buff[PACK_DATA]) {
			case 0:
				DataLinkReport("%s: Write request.\n", node);
				break;
			case 1:
				DataLinkReport("%s: Read request.\n", node);
				break;
			case DLINK_MASTER:
				DataLinkReport("%s: Master broadcast.\n", node);
			default:
				DataLinkReport("%s: Unknown request.\n", node);
				break;
			}
		}
	dlink.oldIndex = dlink.curIndex;
	dlink.curIndex = dlink.curIndex==DLINK_INDEX_MAX ? DLINK_INDEX_MIN : dlink.curIndex+1;
	}


static void DataLinkReport(char *format, ...) {
	va_list	args;

	va_start(args, format);
	vprintf(format, args);
	if (fptr)
		vfprintf(fptr, format, args);
	va_end(args);
	}
