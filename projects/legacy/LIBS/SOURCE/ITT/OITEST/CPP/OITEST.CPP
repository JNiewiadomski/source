// OITest - polls O/I Panel's to verify that communication is established.
//
// Syntax:	OITest [-2] [-b] [-h] [file]
//
// Description:	polls O/I Panel's to verify that communication hasn't failed.
//		O/I Panel and the PLC.
//
// Output:	Writes to stdout the results of communication.
//
// Status:	OIScan may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//		2	I/O error.
//
// Options:	-2	Selects COM2 for serial communication.
//		-b	Select new Baud rate (default is 9600)
//		-h	Display help screen.
//		-n	Number of nodes to poll.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0ë1  9103.07  JN      Original program.

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\asynch.h>
#include <common\getargs.h>
#include <common\jiffy.h>
#include <common\keyboard.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#define DLINK_BUF_LEN	255

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

#define REPLY_TIME		20

#define DLINK_FIRST_NODE	0x11
#define DLINK_LAST_NODE		0x1F
#define DLINK_MAX_NODES		(DLINK_LAST_NODE - DLINK_FIRST_NODE + 1)

#define DLINK_MODE		(ASINOUT | BINARY | NORMALRX)

#define NET_WRITE		0
#define NET_READ		1

enum { EXIT_OK, EXIT_SYNTAX, EXIT_IO };

typedef struct {
	Byte	oldIndex;		// index of previous packet
	Byte	curIndex;		// index of current packet
	Byte	len;			// current length of packet received
	Byte	buff[DLINK_BUF_LEN];	// packet buffer
	Word	getting	: 1;		// are we currently receiving data ?
	} DataLinkRecord;

static DataLinkRecord	dlink;
static FILE		*fptr=NULL;
static int		port=COM1, baud=9600, numNodes=15;
static Byte		oiReset[] = { A_ESC, DLINK_RESET };
static Byte		redLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 };
static Byte		greenLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 };
static Byte		orangeLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66 };
static Byte		flRedLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 };
static Byte		flGreenLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };
static Byte		flOrangeLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99 };
static Byte		flRedOrgLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
static Byte		flGreenOrgLED[]	= { NET_WRITE, 14, 6, 194, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
static Byte		*ledList[] = {	redLED,
					greenLED,
					orangeLED,
					flRedLED,
					flGreenLED,
					flOrangeLED,
					flRedOrgLED,
					flGreenOrgLED,
					NULL
					};

static void	HelpOptions(void);
static short	Poll(void);
static void	DataLinkReset(void);
static void	DataLinkTransmit(Byte node, short len, Byte *data);
static void	DataLinkSend(short len, Byte *data);
static Byte	DataLinkCheckSum(short len, Byte *buff);


main() {
	char	*progName;
	int	opt, status;

	progName = getname("OITEST");
	fprintf(stderr, COPYRIGHT_FORMAT, progName, "1.0ë1", "1991");
	while ((opt = getopt("2b:n:")) != OPT_DONE)
		switch (opt) {
			case '2':
				port = COM2;
				break;
			case 'b':
				baud = atoi(optarg);
				break;
			case 'n':
				numNodes = atoi(optarg);
				if (numNodes < 1 || numNodes > DLINK_MAX_NODES)
					exit(EXIT_SYNTAX);
				break;
			default:
				HelpOptions();
				break;
			}
	_argc -= optind;
	_argv += optind;
	if (_argc > 0)
		fptr = fopen(_argv[0], "wt");
	status = Poll();
	fclose(fptr);
	return(status);
	}


// HelpOptions explains what the program does and displays the syntax line.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nPolls the O/I Panels with test data.\n\n",
				"Syntax:  OITest [-2] [-h] [file]\n\n",
				"Status:  OITest may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Syntax error.\n",
				"          2  I/O error.\n\n",
				"Options: -2  Selects COM2 for serial communication.\n",
				"         -b  Select new Baud rate (default is 9600).\n",
				"         -h  Display help screen.\n"
				"         -n  Number of nodes to poll.\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}


static short Poll(void) {
	Boolean	got;
	Byte	node, *s;
	Jiff	timeLeft;
	short	len, ledIndex;

	dlink.oldIndex = DLINK_INDEX_BAD;
	dlink.curIndex = DLINK_INDEX_INIT;
	dlink.getting = false;
	if (asiopen(port, DLINK_MODE, DLINK_BUF_LEN*16, DLINK_BUF_LEN, baud, P_NONE, 1, 8, OFF, OFF) != ASSUCCESS)
		return (EXIT_IO);
	DataLinkReset();
	len = sizeof(redLED);
	ledIndex = 0;
	s = ledList[ledIndex];
	while (!KeyboardAvail() || KeyboardRead() != K_ESC) {
		for (node=DLINK_FIRST_NODE; node<=DLINK_LAST_NODE; node++) {
			DataLinkTransmit(node, len, s);
			for (timeLeft=JiffyTime()+REPLY_TIME, got=false; !JiffyElapsed(timeLeft) && !got; )
				if (getrxcnt(port) > 0 && asigetc(port) == ACK)
					got = true;
			if (!got && node <= DLINK_FIRST_NODE+numNodes-1)
				printf("Node %d failed to acknowledge write.\n", node);
			}
		dlink.oldIndex = dlink.curIndex;
		dlink.curIndex = dlink.curIndex==DLINK_INDEX_MAX ? DLINK_INDEX_MIN : dlink.curIndex+1;
		s = ledList[++ledIndex];
		if (!s) {
			ledIndex = 0;
			s = ledList[ledIndex];
			}
		}
	asiquit(port);
	}


static void DataLinkReset(void) {
	Boolean	got;
	Byte	node;
	Jiff	timeLeft;

	for (node=DLINK_FIRST_NODE; node<=DLINK_LAST_NODE; node++) {
		DataLinkTransmit(node, sizeof(oiReset), oiReset);
		for (timeLeft=JiffyTime()+REPLY_TIME, got=false; !JiffyElapsed(timeLeft) && !got; ) {
			if (getrxcnt(port) < 6)
				continue;
			if (asigetc(port) != STX)
				continue;
			if (asigetc(port) != 6)
				continue;
			if (asigetc(port) != DLINK_BROADCAST)
				continue;
			if (asigetc(port) != ERR_RESET)
				continue;
			if (asigetc(port) != DLINK_MASTER)
				continue;
			asigetc(port);
			got = true;
			}
		if (!got && node <= DLINK_FIRST_NODE+numNodes-1)
			printf("Node %d failed to confirm reset.\n", node);
		}
	}


static void DataLinkTransmit(Byte node, short len, Byte *data) {
	Byte	*s, buff[DLINK_BUF_LEN];
	short	i;

	s = buff;
	*s++ = STX;
	*s++ = len + 5;
	*s++ = node;
	*s++ = dlink.curIndex;
	for (i=len; i; i--)
		*s++ = *data++;
	*s = DataLinkCheckSum(len+4, buff);
	DataLinkSend(len+5, buff);
	}


static void DataLinkSend(short len, Byte *data) {
	short	i;
	Byte	ch;

	asrts(port, ON);
	ch = *data++;
	len--;
	asiputc(port, ch);
	for (i=0; i<len; i++) {
		ch = *data++;
		asiputc(port, ch);
		if (ch == STX)
			asiputc(port, 0x0000);
		}
	AsynchTillTXEmpty(port);
	asrts(port, OFF);
	asiclear(port, ASIN);
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
