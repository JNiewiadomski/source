#include <dir.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>

#include "error.h"
#include "gen.h"

#define FONT_ASC_EXT	".ASC"
#define FONT_ASM_EXT	".ASM"

#define BUF_SIZE	513

static FILE	*fInPtr, *fOutPtr;	// pointer to src and dst files
static short	lineNum=0;
static char	src_name[MAXPATH], dst_name[MAXPATH], buf[BUF_SIZE];


static void	GenFont(void);
static void	GenSyntaxError(char *format, ...);


void Gen(char *src, char *dst) {
	char	drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE];

	// build source file name
	strcpy(src_name, src);
	if (!(fnsplit(src_name, drive, dir, name, NULL) & EXTENSION))
		strcat(src_name, FONT_ASC_EXT);
	strupr(src_name);
	if ((fInPtr = fopen(src_name, "rt")) == NULL)
		ErrorFatal("Unable to open file '%s'", src_name);

	// build destination file name
	if (dst) {
		strcpy(dst_name, dst);
		if (!(fnsplit(dst_name, NULL, NULL, NULL, NULL) & EXTENSION))
			strcat(dst_name, FONT_ASM_EXT);
		}
	else
		fnmerge(dst_name, drive, dir, name, FONT_ASM_EXT);
	strupr(dst_name);
	if ((fOutPtr = fopen(dst_name, "wt")) == NULL)
		ErrorFatal("Unable to create file '%s'", dst_name);

	GenFont();

	// clean up after ourselves
	fclose(fOutPtr);
	fclose(fInPtr);
	}


static void GenGetLine(void) {
	fgets(buf, BUF_SIZE, fInPtr);
	buf[strlen(buf)-1] = '\0';		// get rid og carriage return
	lineNum++;
	}


static void GenFont(void) {
	char	*s, ch, name[127];
	short	height, width, sets, cur_set, size, count, line, out_byte, col;
	short	size_out, count_hi;
	Byte	bit_pos;

	GenGetLine();
	s = strtok(buf, ",");
	strcpy(name, s);			// get font family name
	s = strtok(NULL, ",");
	height = atoi(s);			// get font height
	s = strtok(NULL, ",");
	width = atoi(s);			// get font width
	size = (height * width) / 8;
	if ((height * width) % 8)		// calculate character size
		size++;
	s = strtok(NULL, ",");
	sets = atoi(s);				// get # of font sets (128 each)

	fprintf(fOutPtr, "CHAR_SET\tSEGMENT\tCODE\n\n");
	fprintf(fOutPtr, "\t\tRSEG\tCHAR_SET\n\n");
	for (cur_set=0; cur_set<sets; cur_set++)
		fprintf(fOutPtr, "\t\tPUBLIC\t%s_%d_FONT\n", name, cur_set);

	for (cur_set=0; cur_set<sets; cur_set++) {
		fprintf(fOutPtr, "\n%s_%d_FONT:\n", name, cur_set);
		fprintf(fOutPtr, "\tDB\t%03XH,%03XH,%03XH\t; height, width, size\n", height, width, size);
		count = cur_set ? 128 : 0;
		for (count_hi=count+128; count<count_hi; count++) {
			GenGetLine();
			if (*buf != ' ')
				GenSyntaxError("Out of Synch at %d (%s)", count, buf);
			fprintf(fOutPtr, "\tDB\t");
			bit_pos = 0x80;
			out_byte = 0x0000;
			size_out = size;
			for (line=height; line; line--) {
				GenGetLine();
				s = buf;
				if (*s != '|')
					GenSyntaxError("Out of Synch at %d (%s)", count, s);
				for (s++, col=width; col; col--) {
					ch = (*s) ? *s++ : ' ';
					if (ch == '*')
						out_byte |= bit_pos;
					bit_pos >>= 1;
					if (!bit_pos) {
						fprintf(fOutPtr, "%03XH", out_byte);
						if (size_out > 1)
							fprintf(fOutPtr, ",");
						bit_pos = 0x80;
						out_byte = 0x0000;
						size_out--;
						}
					}
				}
			if (size_out)
				fprintf(fOutPtr, "%03XH", out_byte);
			fprintf(fOutPtr, "\t; %03Xh", count);
			if ((count >= 32 && count < 127) || (count > 127))
				fprintf(fOutPtr, " ('%c')", count);
			fprintf(fOutPtr, "\n");
			}

		}
	fprintf(fOutPtr, "\n\tEND\n");
	}


static void GenSyntaxError(char *format, ...) {
	va_list	args;

	printf("Syntax Error (%d): ", lineNum);
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	fclose(fOutPtr);
	fclose(fInPtr);
	unlink(dst_name);
	exit(ERR_SYNTAX);
	}
