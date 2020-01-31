#include <ctype.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\getargs.h>
#include <common\jiffy.h>

#include <comm\gf.h>
#include <comm\asiports.h>

#include "datalink.h"

#define NET_COMMAND	0
#define NET_LEN		1
#define NET_ADR_HI	2
#define NET_ADR_LO	3
#define NET_DATA	4

#define COMMAND_PROGRAM		'\002'
#define COMMAND_READ		'\003'
#define COMMAND_ERASE		'\004'
#define COMMAND_CHECKSUM	'\005'

#define TIME_OUT	500L		/* seconds to wait till timeout */
#define NUM_TRIES	5		/* number of tries before abort */

enum {
	ERR_FLASH_ERASE,
	ERR_FLASH_ERASE_TIME,
	ERR_FILE_OPEN,
	ERR_FLASH_PROG,
	ERR_FLASH_PROG_TIME,
	ERR_FLASH_CSUM,
	ERR_FLASH_CSUM_TIME
	};

/*
 *  Version Resource defenition.
 *  A new program has Version 1.0.  If there is a minor revision to the
 *  program, it is then labeled Version 1.1.  If there's a bug fix to
 *  Version 1.1, it's designated 1.1.1.  So, if you have a program that had
 *  a second bug fix to a third minor revision, it would be Version 1.3.2.
 *  If there's a major revision to the program, the first number is
 *  incremented.  If the program has gone through a major revision, four
 *  minor revisions, and six bug fixes, the current version is 2.4.6.
 *  There's also a development suffix, which is added to indicate how far
 *  along the product is.  There are four different stages: The earliest is
 *  'd' for "development".  The next level is 'a' for "alpha".  Then 'b' for
 *  "beta".  Finally, a '.' indicates the released version.  If you have a
 *  product labeled 1.3d1, it is the first development version of the third
 *  revision of the first release of the product.
 */

#define ABOUT_FORMAT	"%s  Version %s  Copyright (c) 1990 Vorne Industries Incorporated\n\n"
#define PROGRAM_NAME	"FLASH"
#define VERSION		"1.0.1"

static char	*progName;

static void	FlashAbort(short err);
static void	FlashErase(void);
static void	FlashProgram(char *name);
static char	*get_hex(char *p, Byte *s);
static void	HelpOptions(void);


main(int argc, char **argv) {
	int	port=COM1, opt, err=0;
	Boolean	erase=false, program=false;

	progName = getname(PROGRAM_NAME);
	printf(ABOUT_FORMAT, progName, VERSION);
	while ((opt = getopt(argc, argv, "2eE")) != EOF) {
		switch (opt) {
			case '2':
				port = COM2;
				break;
			case 'e':
			case 'E':
				erase = true;
				break;
			default:
				err++;
				break;
			}
		}
	if (argc >= optind+1)		/* if file specified, program */
		program = true;
	if (err || !(erase || program))
		HelpOptions();
	DataLinkInit(port);
	if (erase)
		FlashErase();
	if (program)
		FlashProgram(argv[optind]);
	DataLinkCleanUp();
	return(EXIT_SUCCESS);
	}


/*
 * HelpOptions explains what the program does and displays the syntax line.
 */

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"ITT O/I Panel Flash Memory Programmer.\n",
				"Establishes communication with ITT O/I Panel via specified communications\n",
				"port.  Default port used is COM1.  If a hex file is specified, the flash\n",
				"memory will be programmed with its contents.  A .HEX file name extension is\n",
				"appended if none is specified.\n\n",
				"Command line: Flash [ -2 | -e ] <hex file>\n",
				"\t-2  Use COM2 for serial communication\n",
				"\t-e  Erase flash memory\n",
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_FAILURE);
	}


static void FlashAbort(short err) {
	static char	*errStr[] = {
				"Unable to erase flash memory",
				"Timeout error in erasing flash memory",
				"Unable to open hex file",
				"Unable to program flash memory.  Try erasing first",
				"Timeout error in programming flash memory",
				"Unable to verify new checksum write.  Try erasing first",
				"Timeout error in writing new checksum.\n"
				};


	if (err < NUMELE(errStr))
		printf("%s.\n", errStr[err]);
	else
		printf("Sorry, an internal error occurred.\n");
	exit(EXIT_FAILURE);
	}


static void FlashErase(void) {
	char	buff[DLINK_BUF_LEN+1], recv_buff[DLINK_BUF_LEN+1];
	Boolean	done=false;
	short	i=0;
	Jiff	timeout;

	printf("Erasing flash memory...\n");
	buff[NET_COMMAND] = COMMAND_ERASE;	/* erase flash memory */
	timeout = JiffyTime();
	while (!done) {
		switch (DataLinkReceive(recv_buff)) {
			case DLINK_ACK:
				done = true;
				break;
			case DLINK_ERROR:
				timeout = JiffyTime();
				break;
			case DLINK_NONE:
				break;
			default:
				switch ((Byte)recv_buff[PACK_IDX]) {
					case ERR_RESET:
						timeout = JiffyTime();
						break;
					case ERR_INDEX:
					case ERR_DONE:
						DataLinkReset();
						break;
					case ERR_FLASH:
						FlashAbort(ERR_FLASH_ERASE);
						break;
					}
				break;
			}
		if (JiffyElapsed(timeout)) {
			if (i > NUM_TRIES)
				FlashAbort(ERR_FLASH_ERASE_TIME);
			else {
				DataLinkTransmit(1, buff);
				timeout = JiffyTime() + TIME_OUT;
				i++;
				}
			}
		}
	printf("Flash memory successfully erased.\n");
	}


static void FlashProgram(char *name) {
	char	filename[MAXPATH], buff[DLINK_BUF_LEN+1], *src;
	FILE	*fptr;
	Byte	packet[DLINK_BUF_LEN+1], *dst, val;
	short	len, i;
	Jiff	timeout;
	Boolean	done;

	strcpy(filename, name);
	if ((fnsplit(filename, NULL, NULL, NULL, NULL) & EXTENSION) == 0)
		strcat(filename, ".HEX");
	if ((fptr = fopen(filename, "r")) == NULL)
		FlashAbort(ERR_FILE_OPEN);
	printf("Programming flash memory...\n");
	while (fgets(buff, DLINK_BUF_LEN, fptr) != NULL) {
		src = buff;
		dst = packet;
		if (*src++ != ':')
			continue;
		src = get_hex(src,&val);
		len = val;
		*dst++ = COMMAND_PROGRAM;	/* flash write */
		*dst++ = len;
		src = get_hex(src,dst++);
		src = get_hex(src,dst++);
		if (*src++ != '0')
			continue;
		if (*src++ != '0')
			continue;
		/*
		 * now src -> data, len bytes long for address
		 */
		for (i = 0; i < len; i++)
			src = get_hex(src,dst++);
		/*
		 * Uncomment the following block of code to see data transmitted
		 */

		/*
		*(src+2) = '\r';
		printf("%s\n", buff);
		for (i=0; i<len+4; i++)
			printf("%2X  ", packet[i]);
		*/

		timeout = JiffyTime();
		i = 0;
		done = false;
		while (!done) {
			switch (DataLinkReceive(buff)) {
				case DLINK_ACK:
					done = true;
					break;
				case DLINK_ERROR:
					timeout = JiffyTime();
					break;
				case DLINK_NONE:
					break;
				default:
					switch ((Byte)buff[PACK_IDX]) {
						case ERR_RESET:
							timeout = JiffyTime();
							break;
						case ERR_INDEX:
						case ERR_DONE:
							DataLinkReset();
							break;
						case ERR_FLASH:
							FlashAbort(ERR_FLASH_PROG);
							break;
						}
					break;
				}
			if (JiffyElapsed(timeout)) {
				if (i > NUM_TRIES)
					FlashAbort(ERR_FLASH_PROG_TIME);
				else {
					DataLinkTransmit(len+4, (char *)packet);
					timeout = JiffyTime() + TIME_OUT;
					i++;
					}
				}
			}
		}
	printf("Performing checksum on flash memory...\n");
	buff[NET_COMMAND] = COMMAND_CHECKSUM;  /* cause O/I Panel to compute checksum on new data */
	timeout = JiffyTime();
	i = 0;
	done = false;
	while (!done) {
		switch (DataLinkReceive(buff)) {
			case DLINK_ACK:
				done = true;
				break;
			case DLINK_ERROR:
				timeout = JiffyTime();
				break;
			case DLINK_NONE:
				break;
			default:
				switch ((Byte)buff[PACK_IDX]) {
					case ERR_RESET:
						break;
					case ERR_INDEX:
					case ERR_DONE:
						DataLinkReset();
						break;
					case ERR_FLASH:
						FlashAbort(ERR_FLASH_CSUM);
						break;
					}
				break;
			}
		if (JiffyElapsed(timeout)) {
			if (i > NUM_TRIES)
				FlashAbort(ERR_FLASH_CSUM_TIME);
			else {
				DataLinkTransmit(1, buff);
				timeout = JiffyTime() + TIME_OUT;
				i++;
				}
			}
		}
	printf("Flash memory programmed successfully.\n");
	fclose(fptr);
	}


static char *get_hex(char *p, Byte *s) {
	static Byte	convert[] = {0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15};

	*s  = convert[*p++ - '0'] << 4;
	*s += convert[*p++ - '0'];
	return(p);
	}
