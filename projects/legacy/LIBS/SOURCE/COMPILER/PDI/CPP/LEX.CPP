#include <ctype.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cbase.h>

#include "h\error.h"
#include "h\lex.h"

#define T_BAD		-1

#define COMMENT		'!'

#define BUFLEN	256

typedef struct {
	char	*name;
	L_TOKEN	tok;
	} KWORD;

static KWORD Ktab[] = {			/* alphabetic keywords */
	{ "#INCLUDE",	L_INCLUDE	},

	{ ".ACTION",	L_ACTION	},
	{ ".CASE",	L_CASE		},
	{ ".DEVICE",	L_DEVICE	},
	{ ".END",	L_END		},
	{ ".STARTUP",	L_STARTUP	},
	{ ".STATE",	L_STATE		},
	{ ".TEXT",	L_TEXT		},

	{ "@AUX0",	L_OUTPUT	},
	{ "@AUX1",	L_OUTPUT	},
	{ "@AUX2",	L_OUTPUT	},
	{ "@BYTE",	L_BYTE		},
	{ "@DISABLE",	L_DISABLE	},
	{ "@DOWN_KEY",	L_KEY		},
	{ "@END_KEY",	L_KEY		},
	{ "@ENTER_KEY",	L_KEY		},
	{ "@ESC_KEY",	L_KEY		},
	{ "@EXTERNAL",	L_EXTERNAL	},
	{ "@F10_KEY",	L_KEY		},
	{ "@F1_KEY",	L_KEY		},
	{ "@F2_KEY",	L_KEY		},
	{ "@F3_KEY",	L_KEY		},
	{ "@F4_KEY",	L_KEY		},
	{ "@F5_KEY",	L_KEY		},
	{ "@F6_KEY",	L_KEY		},
	{ "@F7_KEY",	L_KEY		},
	{ "@F8_KEY",	L_KEY		},
	{ "@F9_KEY",	L_KEY		},
	{ "@HOME_KEY",	L_KEY		},
	{ "@LEFT_KEY",	L_KEY		},
	{ "@PDMA_32",	L_PDMA_32	},
	{ "@PGDN_KEY",	L_KEY		},
	{ "@PGUP_KEY",	L_KEY		},
	{ "@RIGHT_KEY",	L_KEY		},
	{ "@SPACE_KEY",	L_KEY		},
	{ "@TAB_KEY",	L_KEY		},
	{ "@TERMINAL",	L_TERMINAL	},
	{ "@TIMER",	L_TIMER		},
	{ "@UP_KEY",	L_KEY		},
	{ "@WORD",	L_WORD		},
	{ "BASE",	L_BASE		},
	{ "BEEP",	L_BEEP		},
	{ "CLEAR",	L_CLEAR		},
	{ "CLOCK",	L_CLOCK		},
	{ "ECHO",	L_ECHO		},
	{ "HALT",	L_HALT		},
	{ "HZ",		L_HZ		},
	{ "IF",		L_IF		},
	{ "INTERRUPT",	L_INTERRUPT	},
	{ "KHZ",	L_KHZ		},
	{ "MHZ",	L_MHZ		},
	{ "MODE",	L_MODE		},
	{ "MS",		L_MS		},
	{ "NEGATIVE",	L_NEGATIVE	},
	{ "POSITIVE",	L_POSITIVE	},
	{ "REPEAT",	L_REPEAT	},
	{ "S",		L_S		},
	{ "SELECT",	L_SELECT	},
	{ "SET",	L_SET		},
	{ "THEN",	L_THEN		},
	{ "TIME",	L_TIME		},
	{ "US",		L_US		}
	};

typedef struct inc_struct {
	char			*yytext;
	int			yylen;
	int			yylineno;
	char			yyname[MAXPATH];
	char			yybuffer[BUFLEN];
	FILE			*yyfp;
	L_TOKEN			Lookahead;
	struct inc_struct	*yyinclude;
	} IncludeRecord;

char	*yytext		= "";			/* lexeme (not '\0' terminated) */
int	yylen		= 0;			/* lexeme length */
int	yylineno	= 0;			/* input line number */
char	yyname[MAXPATH];			/* input file name */

static char		yybuffer[BUFLEN];
static IncludeRecord	*yyinclude=NULL;	/* include stack */
static short		yylevel=0;		/* include nesting level */
static short		yyold=T_BAD;		/* save character replaced with '\0' */

static FILE	*yyfp=stdin;			/* input stream */
static L_TOKEN	Lookahead=L_BAD;		/* lookahead token */


static L_TOKEN	LexCheckKeyword(char *name, short len, L_TOKEN tok);
static int	LexCompare(const void *a, const void *b);
static Boolean	LexRead(void);
static Boolean	LexUnInclude(void);


/*
 * Lex returns the next token found in the current input stream, ignoring
 * white space and comments.  L_EOI is returned when we run out of input.
 * Use yytext to find out the literal text and yylen for its length.
 *
 * NOTE:	These strings are not ASCIIZ (with the exception of
 *		L_STRING's).  Use yylen to find out how many characters are
 * valid.  If you would like to treat yytext as an ASCIIZ string, do the
 * following: call LexTerm() to make it ASCIIZ, use it or copy it, and then
 * call LexUnTerm() to change it back.
 *
 */

L_TOKEN Lex(void) {
	char	*current;

	current = yytext + yylen;	/* skip current lexeme */
	while (true) {			/* get the next one */
		while (isspace(*current))
			++current;
		while (!*current || *current==COMMENT) {
			/*
			 * get new lines, skipping any leading white space
			 * on the line, until a nonblank line is found
			 */
			current = yybuffer;
			if (!LexRead()) {
				*current = '\0';
				return (L_EOI);
				}
			++yylineno;
			while (isspace(*current))
				++current;
			}
		for ( ; *current; ++current) {
			/*
			 * get the next token
			 */
			yytext = current;
			yylen = 1;
			switch (*current) {
				case COMMENT:
					*(current+1) = '\0';
					break;
				case EOF:
					return (L_EOI);
				case '{':
					return (L_LCURLY);
				case '}':
					return (L_RCURLY);
				case '=':
					return (L_EQUAL);
				case '(':
					return (L_LPAREN);
				case ')':
					return (L_RPAREN);
				case ';':
					return (L_SEMI_COLON);
				case '#':
					current++;
					while (isalnum(*++current) || *current=='_')
						;
					yylen = (int)(current - yytext);
					switch (LexCheckKeyword(yytext, yylen, L_BAD)) {
						case L_INCLUDE:
							while (isspace(*current))
								current++;
							if (*current != '"') {
								ErrorSevere("#INCLUDE argument must be enclosed in quotes");
								*(current+1) = '\0';
								break;
								}
							yytext = ++current;
							while (*current && *current != '"')
								current++;
							if (!*current) {
								ErrorSevere("Unterminated String");
								yylen = (int) (current - yytext);
								}
							else {
								yylen = (int) (current - yytext + 1);
								*current = '\0';
								}
							if (LexInclude(yytext))
								*(current+1) = '\0';
							else
								ErrorSevere("Unable to include file '%s'", yytext);
							break;
						default:
							LexTerm();
							ErrorSevere("Unrecognized preprocessor directive '%s'", yytext);
							LexUnTerm();
							break;
						}
					break;
				case '"':
					yytext++;
					while (*++current && *current != '"')
						;
					if (!*current) {
						ErrorSevere("Unterminated String");
						yylen = (int) (current - yytext);
						}
					else {
						yylen = (int) (current - yytext + 1);
						/*
						 * We want to get rid of the surrounding
						 * quotes, so we point past first one
						 * and get rid of second one by making it
						 * ASCIIZ!  Note length is off by 1.
						 */
						*current = '\0';
						}
					return (L_STRING);
				case '[':
					while (*++current && *current != ']')
						;
					yylen = (int) (current - yytext + 1);
					if (!*current)
						ErrorSevere("Unterminated Mask");
					return (L_MASK);
				case '.':
				case '@':
					while (isalnum(*++current) || *current=='_' || *current=='$')
						;
					yylen = (int)(current - yytext);
					return (LexCheckKeyword(yytext, yylen, L_BAD));
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					while (isxdigit(*current))
						current++;
					if (*current=='b' || *current=='B' || *current=='h' || *current=='H')
						current++;
					yylen = (int)(current - yytext);
					return (L_UINT);
				default:
					if (!isalnum(*current) && *current!='_' && *current!='$')
						ErrorWarning("Ignoring illegal input '%c'", *current);
					else {
						while (isalnum(*++current) || *current=='_' || *current=='$')
							;
						yylen = (int)(current - yytext);
						return (LexCheckKeyword(yytext, yylen, L_ID));
						}
					break;
				}
			}
		}
	}


/*
 * LexAdvance advances the Lookahead to the next input symbol.
 */

void LexAdvance(void) {
	Lookahead = L_BAD;
	}


/*
 * Do a binary search for a possible keyword in KTab.  Return the L_TOKEN
 * if it's in the table, L_ID otherwise.
 */

static L_TOKEN LexCheckKeyword(char *name, short len, L_TOKEN tok) {
	char	old;
	KWORD	*found, dummy;

	old = name[len];
	name[len] = '\0';
	dummy.name = name;
	found = (KWORD *) bsearch(&dummy, Ktab, sizeof(Ktab)/sizeof(KWORD), sizeof(KWORD), LexCompare);
	name[len] = old;
	return (found ? found->tok : tok);
	}


/*
 * Comparison function called by bsearch.  Return 0 is the two strings are
 * the same.
 */

static int LexCompare(const void *a, const void *b) {
	return (strcmp(((KWORD *)a)->name, ((KWORD *)b)->name));
	}


/*
 * Call LexInclude when the input stream needs to be interrupted by a
 * second input stream.
 */

Boolean LexInclude(char *name) {
	char            buf[MAXPATH];
	FILE		*fp;
	IncludeRecord	*inc;

	strcpy(buf, name);
	strupr(buf);
	if ((fp = fopen(buf, "rt")) == NULL)
		return (false);
	if (yylevel > 0) {
		if ((inc = (IncludeRecord *) malloc(sizeof(IncludeRecord))) == NULL) {
			fclose(fp);
			return (false);
			}
		inc->yytext = yytext;
		inc->yylen = yylen;
		inc->yylineno = yylineno;
		strcpy(inc->yyname, yyname);
		strcpy(inc->yybuffer, yybuffer);
		inc->yyfp = yyfp;
		inc->Lookahead = Lookahead;
		inc->yyinclude = yyinclude;
		}
	yytext = "";
	yylen = yylineno = 0;
	strcpy(yyname, buf);
	yyfp = fp;
	Lookahead = L_BAD;
	yyinclude = inc;
	yylevel++;
	return (true);
	}


/*
 * LexLook returns the current Lookahead symbol.
 */

L_TOKEN LexLook(void) {
	if (Lookahead == L_BAD)
		Lookahead = Lex();
	return (Lookahead);
	}


/*
 * LexMatch returns true is token matches the current Lookahead symbol.
 */

Boolean LexMatch(L_TOKEN token) {
	if (Lookahead == L_BAD)
		Lookahead = Lex();
	return ((Boolean)(token == Lookahead));
	}


/*
 * LexRead attempts to read the next line of data from the current data
 * input stream.  On a failure, LexUnInclude is called and the read is
 * reattempted.  This process continues until all of the nesting levels
 * have been exhausted, at which point we return false.  Otherwise we
 * return true.
 */

static Boolean LexRead(void) {
	char	*ip=yybuffer;

	while (!fgets(ip, BUFLEN, yyfp))
		if (!LexUnInclude())
			return (false);
	return (true);
	}


/*
 * Use LexTerm() to make the current lexeme ASCIIZ.  It is imperative that
 * LexUnTerm() is called when we are done and before Lex() is called, either
 * directly or indirectly by LexMatch(), LexLook(), etc.
 */

void LexTerm(void) {
	char	*s;

	if (yyold != T_BAD)
		ErrorFatal("Internal Error: LexTerm");
	s = yytext + yylen;
	yyold = *s;
	*s = '\0';
	}


/*
 * LexUnInclude closes the current input file and restores the environment
 * of the previous include file.  This makes it appear as if the two files
 * were really one huge file.  If no more levels of nesting exist, we
 * return false, otherwise true.
 */

static Boolean LexUnInclude(void) {
	IncludeRecord	*inc;

	fclose(yyfp);
	if (yylevel < 2)
		return (false);
	inc = yyinclude;
	yytext = inc->yytext;
	yylen = inc->yylen;
	yylineno = inc->yylineno;
	strcpy(yyname, inc->yyname);
	strcpy(yybuffer, inc->yybuffer);
	yyfp = inc->yyfp;
	Lookahead = inc->Lookahead;
	yyinclude = inc->yyinclude;
	free(inc);
	yylevel--;
	return (true);
	}


/*
 * Use LexUnTerm() to undo what LexTerm() did, ie. make the current lexeme
 * ASCIIZ.  Failure to pair these calls up can cause Lex() to fail to
 * recognize any more tokens on the current line.
 */

void LexUnTerm(void) {
	if (yyold == T_BAD)
		ErrorFatal("Internal Error: LexUnTerm");
	*(yytext+yylen) = yyold;
	yyold = T_BAD;
	}
