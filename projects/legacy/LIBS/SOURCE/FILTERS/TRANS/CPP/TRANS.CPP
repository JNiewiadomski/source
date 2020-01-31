// Trans - translation filter.
//
// Syntax:	Trans [/?] [/l] [/z] match_string replace_string
//
// Description:	Copies standard input to standard output, replacing all
//		occurrences of match_string with replace_string.
//		Trans recognizes the following wild cards:
//			#	match any digit (0 - 9)
//
// Status:	Trans may return the following status codes:
//		0	No errors.
//		1	Syntax error.
//		2	No translations made.
//
// Options:	/?	Display help screen.
//		/l	Specifies line terminating character.
//		/z	Suppresses insertion of ^Z at end-of-file.
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.4    9205.15  jn      Use C++ streams; append ^Z.
// 1.3    9204.30  jn      Updated to new COMMON library.
// 1.2    9110.23  jn      Option to specify line terminator character.
// 1.1    9105.29  jn
// 1.0    9105.29  jn      Original program.

#include <ctype.h>
#include <iostream.h>
#include <string.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\ttool.h>


#define BUF_SIZE	512
#define WILD_CARDS	"#"


class TAppTool : public TTool {
	enum { kExitNotFound=kExitSyntax+1 };

	char	fLT;			// line terminator
	Boolean	fCtrlZ;			// append ^Z?

	char	*Match(char *original, const char *match);
	Boolean	MatchPattern(const char *original, const char *match);
protected:
	Boolean	DoOption(char option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	TAppTool(const char *name);
	};


static TAppTool	app("Trans");

static const char *help_text[] = {
		"\nCopies standard input to standard output, replacing all occurrences of\n",
		"match_string with replace_string.\n",
		"Trans recognizes the following wild cards:\n",
		"         #   match any digit (0 - 9)\n\n",
		"Syntax:  TRANS [/?] [/l] [/z] match_string replace_string\n\n",
		"Status:  Trans may return the following status codes:\n",
		"          0  No errors.\n",
		"          1  Syntax error.\n",
		"          2  No translations made.\n\n",
		"Options: /?  Displays this help text.\n",
		"         /l  Specifies line terminating character (default=LF).\n",
		"         /z  Suppresses insertion of ^Z at end-of-file.\n\n"
		};


main() {
	return (app.Run());
	}


TAppTool::TAppTool(const char *name) : TTool(name) {
	fOptList = "l:L:zZ";
	fHelpNum = NUMELE(help_text);
	fHelpText = help_text;
	fLT = A_LF;
	fCtrlZ = True;
	}


Boolean TAppTool::DoOption(char option, const char *argument) {
	switch (option) {
		case 'l':
		case 'L':
			fLT = *argument;
			return (True);
		case 'z':
		case 'Z':
			fCtrlZ = False;
			return (True);
		}
	return (False);
	}


int TAppTool::DoWork(short argc, const char **argv) {
	const char	*match, *replace;
	Boolean		found;

	Copyright("1.4", "1991, 1992");
	if (argc != 2) {
		Help(kErrParam);
		return (kExitSyntax);
		}
	match = argv[0];
	replace = argv[1];
	found = False;
	while (True) {
		short	ch;
		char	buff[BUF_SIZE+1], *bp, *s;

		cin.get(buff, sizeof(buff), fLT);
		if (cin.gcount() == 0 && cin.eof()) {
			if (fCtrlZ)
				cout << (char) A_CTRLZ;
			break;
			}
		bp = buff;
		while ((s = Match(bp, match)) != NULL) {
			ch = *s;
			*s = '\0';
			cout << bp << replace;
			*s = ch;
			bp = s + strlen(match);
			found = True;
			}
		cout << bp;
		if (!cin.eof())
			cout << (char) cin.get();
		}
	return (found ? kExitOK : kExitNotFound);
	}


// Scans original for the first occurrence of the substring match.  Match
// returns a pointer to the element in original where match begins.  If
// match does not occur in original, Match returns NULL.

char *TAppTool::Match(char *original, const char *match) {
	short	len;

	if (strpbrk(match, WILD_CARDS) == NULL)
		return (strstr(original, match));
	len = strlen(original) - strlen(match) + 1;
	while (len > 0 && *original) {
		if (MatchPattern(original, match))
			return (original);
		original++;
		len--;
		}
	return (NULL);
	}


Boolean TAppTool::MatchPattern(const char *original, const char *match) {
	short	matchLen;

	matchLen = strlen(match);
	if (strlen(original) < matchLen)
		return (False);
	for ( ; matchLen > 0; matchLen--, original++, match++)
		switch (*match) {
			case '#':
				if (!isdigit(*original))
					return (False);
				break;
			default:
				if (*original != *match)
					return (False);
				break;
			}
	return (True);
	}