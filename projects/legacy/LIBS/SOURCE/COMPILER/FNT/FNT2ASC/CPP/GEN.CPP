#include <dir.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>

#include "error.h"
#include "gen.h"

#define FONT_FNT_EXT	".FNT"
#define FONT_ASC_EXT	".ASC"

#define BUF_SIZE	513

short		header=0, footer=0;
Boolean		double_width = false;

static FILE	*fInPtr, *fOutPtr;	// pointer to src and dst files
static char	src_name[MAXPATH], dst_name[MAXPATH];


static void	GenFont(void);
static void	GenSyntaxError(char *format, ...);
static void	GenWarning(char *format, ...);


void Gen(char *src, char *dst) {
	char	drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE];

	// build source file name
	strcpy(src_name, src);
	if (!(fnsplit(src_name, drive, dir, name, NULL) & EXTENSION))
		strcat(src_name, FONT_FNT_EXT);
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

	GenFont();

	// clean up after ourselves
	fclose(fOutPtr);
	fclose(fInPtr);
	}


#define LOC_MAX		128
#define WIDTH_MAX	100

static void GenFont(void) {
	char	name[127], *s, separator[WIDTH_MAX+1], padding[WIDTH_MAX+3];
	short	height, width, size, sets, cur_set, count, count_hi, bit_pos;
	short	row, col, ch, cur_header, cur_footer;
	long	loc_tab[LOC_MAX];
	Boolean	clipping;

	s = name;
	while ((*s++ = fgetc(fInPtr)) != NULL)
		;
	fread(&height, sizeof(height), 1, fInPtr);
	fread(&width, sizeof(width), 1, fInPtr);
	if (width > WIDTH_MAX)
		ErrorFatal("Maximum character width is %d", WIDTH_MAX);
	size = (height * width) / 8;
	if ((height * width) % 8)		// calculate character size
		size++;
	fread(&sets, sizeof(sets), 1, fInPtr);
	if (height+header+footer < 2)
		GenSyntaxError("Character height cannot be less than 2 pixels");
	if (double_width)
		fprintf(fOutPtr, "%s,%d,%d,%d\n", name, height+header+footer, width*2, sets);
	else
		fprintf(fOutPtr, "%s,%d,%d,%d\n", name, height+header+footer, width, sets);

	fread(loc_tab, sizeof(loc_tab[0]), sets, fInPtr);
	for (s=separator, count=(double_width ? width*2 : width); count; count--)
		*s++ = '-';
	*s = '\0';

	sprintf(padding, "|%*s|\n", double_width ? width*2 : width, " ");

	for (cur_set=0; cur_set<sets; cur_set++) {
		fseek(fInPtr, loc_tab[cur_set], SEEK_SET);
		count = cur_set ? 128 : 0;
		for (count_hi=count+128; count<count_hi; count++) {
			clipping = false;
			if ((count >= 32 && count < 127) || (count > 127))
				fprintf(fOutPtr, " %s %03Xh ('%c')\n", separator, count, count);
			else
				fprintf(fOutPtr, " %s %03Xh\n", separator, count);
			bit_pos = 0x0000;
			for (cur_header=header; cur_header>0; cur_header--)
				fprintf(fOutPtr, padding);
			cur_footer = -footer;
			for (row=height; row; row--) {
				if (!cur_header && row>cur_footer)
					fprintf(fOutPtr, "|");
				for (col=width; col; col--) {
					if (!bit_pos) {
						if (feof(fInPtr))
							GenSyntaxError("Unexpected End Of File");
						ch = fgetc(fInPtr);
						bit_pos = 0x0080;
						}
					if (!cur_header && row>cur_footer) {
						fputc((ch & bit_pos) ? '*' : ' ', fOutPtr);
						if (double_width)
							fputc((ch & bit_pos) ? '*' : ' ', fOutPtr);
						}
					else if (ch & bit_pos)
						clipping = true;
					bit_pos >>= 1;
					}
				if (!cur_header && row>cur_footer)
					fprintf(fOutPtr, "|\n");
				else if (cur_header)
					cur_header++;
				}
			for (row=footer; row>0; row--)
				fprintf(fOutPtr, padding);
			if (clipping)
				GenWarning("Clipping character %d", count);
			}
		}

	fprintf(fOutPtr, " %s \n", separator);
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


static void GenWarning(char *format, ...) {
	va_list	args;

	printf("Warning: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	}
