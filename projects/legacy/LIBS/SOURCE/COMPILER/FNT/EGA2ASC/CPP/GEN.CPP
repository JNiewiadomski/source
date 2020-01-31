#include <dir.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>

#include "error.h"
#include "gen.h"

#define FONT_EGA_EXT	".EGA"
#define FONT_ASC_EXT	".ASC"

#define BUF_SIZE	513

static FILE	*fInPtr, *fOutPtr;	// pointer to src and dst files
static char	src_name[MAXPATH], dst_name[MAXPATH];


static void	GenFont(char *name);
static void	GenSyntaxError(char *format, ...);


void Gen(char *src, char *dst) {
	char	drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE];

	// build source file name
	strcpy(src_name, src);
	if (!(fnsplit(src_name, drive, dir, name, NULL) & EXTENSION))
		strcat(src_name, FONT_EGA_EXT);
	strupr(src_name);
	if ((fInPtr = fopen(src_name, "rb")) == NULL)
		ErrorFatal("Unable to open file '%s'", src_name);

	// build destination file name
	if (dst) {
		strcpy(dst_name, dst);
		if (!(fnsplit(dst_name, NULL, NULL, name, NULL) & EXTENSION))
			strcat(dst_name, FONT_ASC_EXT);
		}
	else
		fnmerge(dst_name, drive, dir, name, FONT_ASC_EXT);
	strupr(dst_name);
	if ((fOutPtr = fopen(dst_name, "wt")) == NULL)
		ErrorFatal("Unable to create file '%s'", dst_name);

	GenFont(name);

	// clean up after ourselves
	fclose(fOutPtr);
	fclose(fInPtr);
	}


// Note that EGA fonts always have a pixel width of 8.

static void GenFont(char *name) {
	short	height, index, ch, row;

	strupr(name);
	height = (short)(filelength(fileno(fInPtr)) / 256L);
	fprintf(fOutPtr, "%s,%d,8,2\n", name, height);

	for (index=0; index<256; index++) {
		if ((index >= 32 && index < 127) || (index > 127))
			fprintf(fOutPtr, " -------- %03Xh ('%c')\n", index, index);
		else
			fprintf(fOutPtr, " -------- %03Xh\n", index);
		for (row=height; row; row--) {
			if (feof(fInPtr))
				GenSyntaxError("Unexpected End Of File");
			ch = fgetc(fInPtr);
			fputc('|', fOutPtr);
			fputc((ch & 128) ? '*' : ' ', fOutPtr);
			fputc((ch & 64) ? '*' : ' ', fOutPtr);
			fputc((ch & 32) ? '*' : ' ', fOutPtr);
			fputc((ch & 16) ? '*' : ' ', fOutPtr);
			fputc((ch & 8) ? '*' : ' ', fOutPtr);
			fputc((ch & 4) ? '*' : ' ', fOutPtr);
			fputc((ch & 2) ? '*' : ' ', fOutPtr);
			fputc((ch & 1) ? '*' : ' ', fOutPtr);
			fputc('|', fOutPtr);
			fputc('\n', fOutPtr);
			}
		}

	fprintf(fOutPtr, " -------- \n");
	}


static void GenSyntaxError(char *format, ...) {
	va_list	args;

	printf("Syntax Error: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	fclose(fOutPtr);
	fclose(fInPtr);
	unlink(dst_name);
	exit(ERR_SYNTAX);
	}
