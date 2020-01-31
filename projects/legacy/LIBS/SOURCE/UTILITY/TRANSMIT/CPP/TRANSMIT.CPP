// Transmit - sends serial data over serial port.
//
// Syntax:	Transmit [-2] [-b] [-d] [-h] [-o] [-p] [-r] [-s] [filename...]
//
// Description:	Sends data over the serial port.  If no file name list is
//		specified, data is input from stdin.  If no options are
//		specified to change the serial data format, the default
//		data format, which can be set via DOS's MODE command, is
//		used.  In the default cooked mode, control codes are
//		introduced with the '~' character.  For example, ~M is
//		replaced with a carriage return.
//
//		~@ = NUL   ~H = BS    ~P = DLE   ~X = CAN   ~~ = ~
//		~A = SOH   ~I = HT    ~Q = DC1   ~Y = EM    ~` = DEL
//		~B = STX   ~J = LF    ~R = DC2   ~Z = SUB
//		~C = ETX   ~K = VT    ~S = DC3   ~[ = ESC
//		~D = EOT   ~L = FF    ~T = DC4   ~\ = FS
//		~E = ENQ   ~M = CR    ~U = NAK   ~] = GS
//		~F = ACK   ~N = SO    ~V = SYN   ~^ = RS
//		~G = BEL   ~O = SI    ~W = ETB   ~_ = US
//
// Input:	Reads standard input for the data if no parameters are
//		specified.
//
// Status:	Transmit may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//		2	I/O error.
//
// Options:	-2	COM2 (default COM1).
//		-b	Baud rate (default 9600).
//		-d	Data bits (defualt 8).
//		-h	Display help screen.
//		-o	Override DOS MODE settings with: 9600, None, 8, 1.
//		-p	Parity (default None).
//		-r	Raw mode (explicit control codes).\n",
//		-s	Stop bits (defualt 1).
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.1    9109.18  JN      Introduce control code syntax; cook/raw mode.
// 1.0    9106.06  JN      Original program.

#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <common\common.h>
#include <common\ascii.h>
#include <common\asynch.h>
#include <common\getargs.h>
#include <common\keyboard.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#define TX_BUF_SIZE	65535U

enum {
	EXIT_OK,
	EXIT_SYNTAX,
	EXIT_IO
	};


static void	HelpOptions(void);
static void	Send(int port, FILE *fp, Boolean raw, Boolean text);


main(int argc, char **argv) {
	int	opt, port=COM1, baud=9600, data=8, parity=P_NONE, stop=1;
	int	result;
	Boolean	change=false, raw=false;
	FILE	*fp;
	char	parityChar='N';

	getinfo("TRANSMIT");
	printf(COPYRIGHT_FORMAT, progName, "1.1", "1991");
	while ((opt = getopt("2b:B:d:D:hHoOp:P:rRs:S:")) != OPT_DONE)
		switch (opt) {
			case '2':
				port = COM2;
				change = true;
				break;
			case 'b':
			case 'B':
				baud = atoi(optarg);
				change = true;
				break;
			case 'd':
			case 'D':
				data = atoi(optarg);
				change = true;
				break;
			case 'o':
			case 'O':
				change = true;
				break;
			case 'p':
			case 'P':
				parityChar = toupper(*optarg);
				switch (parityChar) {
					case 'N':
						parity = P_NONE;
						break;
					case 'O':
						parity = P_ODD;
						break;
					case 'E':
						parity = P_EVEN;
						break;
					case 'S':
						parity = P_S_STICK;
						break;
					case 'M':
						parity = P_M_STICK;
						break;
					}
				change = true;
				break;
			case 'r':
			case 'R':
				raw = true;
				break;
			case 's':
			case 'S':
				stop = atoi(optarg);
				change = true;
				break;
			case 'h':
			case 'H':
			case OPT_ERR:
				HelpOptions();
				break;
			}
	argc -= optind;
	argv += optind;
	if (change) {
		result = asiopen(port, ASOUT, MIN_BUF_SIZE, TX_BUF_SIZE, baud, parity, stop, data, OFF, OFF);
		printf("COM%d:%d,%c,%d,%d\n", port+1, baud, parityChar, data, stop);
		}
	else
		result = asifirst(port, ASOUT, MIN_BUF_SIZE, TX_BUF_SIZE);
	if (result != ASSUCCESS) {
		printf("Unable to open COM%d.\n", port+1);
		return (EXIT_IO);
		}
	if (argc == 0)
		Send(port, stdin, raw, true);
	else
		for ( ; argc > 0; argc--, argv++)
			if ((fp = fopen(*argv, "rb")) == NULL)
				printf("Unable to open file %s.\n", *argv);
			else {
				printf("%s:\n", *argv);
				Send(port, fp, raw, false);
				fclose(fp);
				}
	AsynchTillTXEmpty(port);
	asiquit(port);
	return (EXIT_OK);
	}


// Displays a brief description of the programs purpose and the command line
// used to invoke it.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nTransmits data over the serial port.  If no file name list is specified,\n",
				"data is input from stdin.  If no options are specified to change the serial\n",
				"data format, the current data format is used.  The current serial data format\n",
				"can be set via DOS's MODE command.  In the default cooked mode, control codes\n",
				"are introduced with the '~' character.  For example, ~M is replaced with a\n",
				"carriage return.\n\n",
				"Syntax:  TRANSMIT [-2] [-b] [-d] [-h] [-p] [-r] [-s] [filename...]\n\n",
				"Status:  Transmit may return the following status codes:\n",
				"          0  No errors.\n",
				"          1  Syntax error.\n",
				"          2  I/O error.\n\n",
				"Options: -2  COM2 (default COM1).\n",
				"         -b  Baud rate (default 9600).\n",
				"         -d  Data bits (default 8).\n",
				"         -h  Displays this help text.\n",
				"         -o  Override DOS MODE settings with: 9600, None, 8, 1.\n",
				"         -p  Parity (None, Even, Odd, Mark, Space; default None).\n",
				"         -r  Raw mode (explicit control codes).\n",
				"         -s  Stop bits (default 1).\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}


static void Send(int port, FILE *fp, Boolean raw, Boolean text) {
	int	ch;

	while (!feof(fp) && !(!text && KeyboardAvail() && KeyboardRead()==K_ESC)) {
		if ((ch = fgetc(fp)) == -1)
			continue;
		if (!raw) {
			if (iscntrl(ch))
				continue;
			if (ch == '~' && !feof(fp)) {
				ch = fgetc(fp);
				if (ch == '`')
					ch = A_DEL;
				else if (ch >= '@' && ch <= '_')
					ch -= '@';
				}
			}
		while (istxfull(port))
			;
		if (raw && text && ch == '\n') {
			asiputc(port, A_CR);
			while (istxfull(port))
				;
			asiputc(port, A_LF);
			}
		else
			asiputc(port, ch);
		}
	}
