#include <dir.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>

#include "error.h"
#include "gen.h"

#define FONT_ASC_EXT	".ASC"
#define FONT_FNT_EXT	".FNT"

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
			strcat(dst_name, FONT_FNT_EXT);
		}
	else
		fnmerge(dst_name, drive, dir, name, FONT_FNT_EXT);
	strupr(dst_name);
	if ((fOutPtr = fopen(dst_name, "wb")) == NULL)
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


#define LOC_MAX	128

static void GenFont(void) {
	char	*s, ch, name[127];
	short	height, width, sets, cur_set, size, count, line, out_byte, col;
	short	size_out, count_hi;
	Byte	bit_pos;
	long	loc_loc, loc_tab[LOC_MAX];

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
	if (sets > LOC_MAX)
		GenSyntaxError("Font family cannot contain more than %d sets.", LOC_MAX);

	fwrite(name, strlen(name)+1, 1, fOutPtr);
	fwrite(&height, sizeof(height), 1, fOutPtr);
	fwrite(&width, sizeof(width), 1, fOutPtr);
	fwrite(&sets, sizeof(sets), 1, fOutPtr);
	loc_loc = ftell(fOutPtr);
	for (cur_set=0; cur_set<sets; cur_set++)
		fwrite(&loc_loc, sizeof(loc_loc), 1, fOutPtr);

	for (cur_set=0; cur_set<sets; cur_set++) {
		loc_tab[cur_set] = ftell(fOutPtr);
		count = cur_set ? 128 : 0;
		for (count_hi=count+128; count<count_hi; count++) {
			GenGetLine();
			if (*buf != ' ')
				GenSyntaxError("Out of Synch at %d (%s)", count, buf);
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
						fprintf(fOutPtr, "%c", out_byte);
						bit_pos = 0x80;
						out_byte = 0x0000;
						size_out--;
						}
					}
				}
			if (size_out)
				fprintf(fOutPtr, "%c", out_byte);
			}
		}
	fseek(fOutPtr, loc_loc, SEEK_SET);
	fwrite(loc_tab, sizeof(loc_loc), sets, fOutPtr);
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
