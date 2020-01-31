// HexLink - combines and links hex files into binary object file.
//
// Syntax:	HexLink [-h] [-l] [-o] input_file...
//
// Description:	Combines and links hex files into binary object file.
//
// Status:	HexLink sets the status code to the following values:
//		 0	No Error
//		 1	Syntax
//		 2	Could Not Open File
//		 3	Bad Record Format
//		 4	Could Not Allocate Memory
//               5      No .HEX Records Found
//
// Options:	-h	Display help screen.
//		-l	Load hex offset.
//		-o	Output file name (default is A.OUT)
//
// History:	DS	David Schmidt
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9104.08  DS      Original program.

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <ctype.h>

#include <common\common.h>
#include <common\getargs.h>

#define BUFSIZE 550

enum {
	EXIT_OK,
	EXIT_SYNTAX,
	EXIT_NOT_OPEN,
	EXIT_BAD_HEX,
	EXIT_NO_MEM,
	EXIT_NO_DATA
	};

static void	HelpOptions(void);


void main(int argc,char *argv[]) {
	Byte	buffer[BUFSIZ], far *memory, count_field, type_field, c, *s;
	Byte	checksum, byte;
	long	last_address = 0;
	int	opt, infile;
	FILE	*fh;
	Word	address_field, offset=0x0000;
	char	*progName, *out_file = "A.OUT";

	progName = getname("HEXLINK");
	printf(COPYRIGHT_FORMAT, progName, "1.0", "1991");
	while ((opt = getopt("l:o:")) != OPT_DONE)
		switch (opt) {
			case 'l':
				sscanf(optarg, "%x", &offset);
				break;
			case 'o':
				out_file = optarg;
				break;
			default:
				HelpOptions();
				break;
			}

	if (optind == argc)
		HelpOptions();

	if ((memory = (Byte far *) farmalloc(0x10000L)) == NULL) {
		printf("HexLink: Unable to allocate memory.\n");
		exit(EXIT_NO_MEM);
		}
	_fmemset(memory,0xff,0xffff);
	for (infile = optind ; infile != argc; infile += 1) {

		if ((fh = fopen(argv[infile],"rt")) == NULL) {
			printf("HexLink: Could not open %s.\n", argv[infile]);
			exit(EXIT_NOT_OPEN);
			}

		while (!feof(fh)) {
			fgets((char *)buffer,BUFSIZ,fh);
			buffer[strlen((char *)buffer) - 1] = '\0';
			if (buffer[0] != ':') continue;
			s = buffer + 1;
			while (*s) if (!isxdigit(*s++)) {
				printf("HexLink: bad hex record(%c) in %s\n",*(s-1),argv[infile]);
				puts((char *)buffer);
				exit(EXIT_BAD_HEX);
				}


			s = buffer + 1;
			count_field = (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10) << 4;
			count_field += (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10);
			checksum = count_field;

			address_field = (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10) << 12;
			address_field += (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10) << 8;
			address_field += (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10) << 4;
			address_field += (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10);
			checksum += (address_field >> 8) + (address_field & 0xff);

			address_field += offset;
			type_field = (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10) << 4;
			type_field += (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10);
			checksum += type_field;

			if (type_field > 1) {
				printf("HexLink: Bad hex record in %s.\n",argv[infile]);
				puts((char *)buffer);
				exit(EXIT_BAD_HEX);
				}

			if (type_field == 1) break;
			for (s = buffer + 9; *s ;) {
				byte = (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10) << 4;
				byte += (isdigit(c = *s++) ? c - '0' : toupper(c) - 'A' + 10);
				checksum = (checksum + byte) & 0xff;
				if (*s == '\0') break;
				*(Byte far *)(memory + address_field++) = byte;
				}

			if (checksum != 0) {
				printf("HexLink: bad checksum record in %s\n",argv[infile]);
				puts((char *)buffer);
				exit(EXIT_BAD_HEX);
				}

			if (address_field > last_address) last_address = address_field;

			}

		fclose(fh);
		}

		if	(last_address == 0L) {
			puts("HexLink: No .HEX records found.");
			exit(EXIT_NO_DATA);
			}

		if	((fh = fopen(out_file,"wb")) == NULL) {
			printf("HexLink: Could not open %s\n", out_file);
			exit(EXIT_NOT_OPEN);
			}

		fwrite(memory,sizeof(char),(size_t)last_address,fh);

		fclose(fh);

	exit(EXIT_OK);
	}


// HelpOptions explains what the program does and displays the syntax line.

static void HelpOptions(void) {
	char		**s;
	short		i;
	static char	*msgStr[] = {
				"\nCombines and links hex files into binary object file.\n\n",
				"Syntax:  HexLink [-h] [-l#] [-ofile] input_file...\n\n",
				"Status:  HexLink sets the status code to the following values:\n",
				"          0  No Error\n",
				"          1  Syntax\n",
				"          2  Could Not Open Input\n",
				"          3  Bad Record Format\n",
				"          4  Could Not Allocate Memory\n",
				"          5  No .HEX Records Found\n\n",
				"Options: -h  Display help screen.\n",
				"         -l  Load hex offset.\n",
				"         -o  Output file name (default is A.OUT)\n"
				};

	for (s=msgStr, i=NUMELE(msgStr); i>0; i--)
		printf(*s++);
	exit(EXIT_SYNTAX);
	}
