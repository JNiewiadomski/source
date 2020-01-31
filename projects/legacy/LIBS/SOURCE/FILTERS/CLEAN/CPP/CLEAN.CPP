// Cleans up ASCII text files by converting all control characters into the
// character sequence \xXX, where XX is the hexadecimal value of the control
// character found.  The control characters HT (09h), LF (0Ah), CR (0Dh) and
// SUB (1Ah) are left alone.  A lone CR or LF is turned into a CRLF pair.
//
// Syntax:	Clean [/?] [/e] [filename...]
//
// Input:	If no source files specified, reads in from stdin.
//
// Output:	If no source files specified, writes out to stdout.
//
// Status:	Clean may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//		2	I/O error.
//
// Options:	/?	Displays this help text.
//		/e	Creates an error log file.
//		/z	Turn NUL's (^@) and SUB's (^Z) into line terminators.
//
// History:	jn	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.1    9202.26  jn      Option to remove all control characters.
// 1.0.1  9111.12  jn      Convert illegal characters to \x##.
// 1.0    9103.20  jn      Original program.

#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\old\keyboard.h>
#include <common\ttool.h>


static const char *help_text[] = {
	"\nCleans up ASCII text files by converting all control characters into the\n",
	"character sequence \\xXX, where XX is the hexadecimal value of the control\n",
	"character found.  The control characters HT (09h), LF (0Ah), CR (0Dh) and\n",
	"SUB (1Ah) are left alone.  A lone CR or LF is turned into a CRLF pair.\n\n",
	"Syntax:  Clean [/?] [/e] [filename...]\n\n",
	"Input:   If no source files specified, reads in from stdin.\n\n",
	"Output:  If no source files specified, writes out to stdout.\n\n",
	"Status:  Clean may return the following status codes:\n",
	"          0  No errors.\n",
	"          1  Syntax error.\n",
	"          2  I/O error.\n\n",
	"Options: /?  Displays this help text.\n",
	"         /e  Creates an error log file.\n",
	"         /z  Turn NUL's (^@) and SUB's (^Z) into line terminators.\n"
	};


class TAppTool : public TTool {
	enum { kExit_IO=kExitSyntax+1 };

	FILE	*fInPtr, *fOutPtr, *fErrPtr;
	Boolean	fSpecialCtrl;

	Boolean	DoOption(char option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	TAppTool(const char *name);

	Boolean	Clean(void);
	void	Error(char *format, ...);
	};


int main() {
	TAppTool	app_tool("CLEAN");

	app_tool.Copyright("1.1", "1991-1992");
	return (app_tool.Run());
	}


TAppTool::TAppTool(const char *name) : TTool(name) {
	fOptList = "eEzZ";
	fHelpNum = NUMELE(help_text);
	fHelpText = help_text;

	fInPtr = stdin;
	fOutPtr = stdout;
	fErrPtr = NULL;
	fSpecialCtrl = False;
	}


Boolean TAppTool::DoOption(char option, const char * /*argument*/) {
	switch (option) {
		case 'e':
		case 'E':
			fErrPtr = stderr;
			return (True);
		case 'z':
		case 'Z':
			fSpecialCtrl = True;
			return (True);
		}
	return (False);
	}


int TAppTool::DoWork(short argc, const char **argv) {
	const char	*inFileName;
	char		outFileName[MAXPATH], errFileName[MAXPATH];
	char		drive[MAXDRIVE], dir[MAXDIR], file[MAXFILE];
	Boolean		ok;

	if (argc < 1)
		Clean();
	else {
		if (fErrPtr) {
			sprintf(errFileName, "%s%s%s", fDataPath, fProgName, ".ERR");
			if ((fErrPtr = fopen(errFileName, "wt")) == NULL) {
				fErrPtr = stderr;
				fprintf(stderr, "Unable to create error file %s.\n", errFileName);
				}
			}
		while (argc--) {
			inFileName = *argv++;
			fnsplit(inFileName, drive, dir, file, NULL);
			fnmerge(outFileName, drive, dir, file, ".CLN");
			if ((fInPtr = fopen(inFileName, "rb")) == NULL)
				fprintf(stderr, "Unable to open file %s.\n", inFileName);
			else if ((fOutPtr = fopen(outFileName, "wt")) == NULL) {
				fclose(fInPtr);
				fprintf(stderr, "Unable to create file %s.\n", outFileName);
				}
			else {
				Error("\n*** Error Messages For %s ***\n", inFileName);
				fprintf(stderr, "%s:\n", inFileName);
				ok = Clean();
				fclose(fInPtr);
				fclose(fOutPtr);
				if (ok)
					remove(outFileName);
				}
			}
		if (fErrPtr && fErrPtr != stderr)
			fclose(fErrPtr);
		}
	return (kExitOK);
	}


Boolean TAppTool::Clean(void) {
	Byte	ch;
	Boolean	ok=True, cr=False;
	short	line=1, val;

	while ((val = fgetc(fInPtr)) != -1) {
		ch = val;
		if (ch == A_LF) {
			if (!cr) {
				fputc('\n', fOutPtr);
				Error("Lone LF turned into CRLF on line %d.\n", line);
				line++;
				}
			}
		else if (ch == A_CR) {
			fputc('\n', fOutPtr);
			line++;
			cr = True;
			}
		else if (fSpecialCtrl && (ch == A_NUL || ch == A_CTRLZ)) {
			fputc('\n', fOutPtr);
			Error("Character 0x%02X changed to a line terminator on line %d.\n", ch, line);
			line++;
			cr = True;
			ok = False;
			}
		else if (ch >= ' ' || ch == '\t' || ch == A_CTRLZ) {
			fputc(ch, fOutPtr);
			cr = False;
			}
		else {
			fprintf(fOutPtr, "\\x%02X", ch);
			Error("Character 0x%02X changed to a \\x%02X on line %d.\n", ch, ch, line);
			ok = cr = False;
			}
		}
	return (ok);
	}


void TAppTool::Error(char *format, ...) {
	va_list	argptr;

	if (fErrPtr == NULL)
		return;
	va_start(argptr, format);
	vfprintf(fErrPtr, format, argptr);
	va_end(argptr);
	}