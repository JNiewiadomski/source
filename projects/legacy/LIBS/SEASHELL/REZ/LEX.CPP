#include <ctype.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>

#include "error.h"
#include "lex.h"
#include "symbol.h"

#define T_BAD		-1

#define COMMENT		'/'

#define BUFLEN	256

typedef struct {
	char	*name;
	L_TOKEN	tok;
	} KWORD;

static KWORD Ktab[] = {			// alphabetic keywords
	{ "#define",		L_DEFINE	},
	{ "#include",		L_INCLUDE	},

	{ ".FILE",		L_FILE		},
	{ ".MEMO",		L_MEMO		},

	{ "NULL",		L_NULL		},

	{ "bar",		L_BAR		},
	{ "black",		L_BLACK		},
	{ "blue",		L_BLUE		},
	{ "brown",		L_BROWN		},
	{ "color",		L_COLOR		},
	{ "coord",		L_COORD		},
	{ "cyan",		L_CYAN		},
	{ "darkgray",		L_DARKGRAY	},
	{ "dialog",		L_DIALOG	},
	{ "error",		L_ERROR		},
	{ "europe",		L_EUROPE	},
	{ "flash",		L_FLASH		},
	{ "form",		L_FORM		},
	{ "green",		L_GREEN		},
	{ "help",		L_HELP		},
	{ "high",		L_HIGH		},
	{ "highinverse",	L_HIGHINVERSE	},
	{ "highline",		L_HIGHLINE	},
	{ "internat",		L_INTERNAT	},
	{ "inverse",		L_INVERSE	},
	{ "japan",		L_JAPAN		},
	{ "lightblue",		L_LIGHTBLUE	},
	{ "lightcyan",		L_LIGHTCYAN	},
	{ "lightgray",		L_LIGHTGRAY	},
	{ "lightgreen",		L_LIGHTGREEN	},
	{ "lightmagenta",	L_LIGHTMAGENTA	},
	{ "lightred",		L_LIGHTRED	},
	{ "line",		L_LINE		},
	{ "list",		L_LIST		},
	{ "low",		L_LOW		},
	{ "magenta",		L_MAGENTA	},
	{ "menu",		L_MENU		},
	{ "noflash",		L_NOFLASH	},
	{ "preload",		L_PRELOAD	},
	{ "prompt",		L_PROMPT	},
	{ "purgeable",		L_PURGEABLE	},
	{ "read",		L_READ		},
	{ "red",		L_RED		},
	{ "resource",		L_RESOURCE	},
	{ "status",		L_STATUS	},
	{ "string",		L_STRING	},
	{ "usa",		L_USA		},
	{ "version",		L_VERSION	},
	{ "video",		L_VIDEO		},
	{ "w0000",		L_W0000		},
	{ "w1111",		L_W1111		},
	{ "w2111",		L_W2111		},
	{ "w2222",		L_W2222		},
	{ "white",		L_WHITE		},
	{ "window",		L_WINDOW	},
	{ "wnone",		L_WNONE		},
	{ "yellow",		L_YELLOW	}
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

char	*yytext		= "";			// lexeme (not '\0' terminated)
int	yylen		= 0;			// lexeme length
int	yylineno	= 0;			// input line number
char	yyname[MAXPATH];			// input file name

static char		yybuffer[BUFLEN];
static IncludeRecord	*yyinclude=NULL;	// include stack
static short		yylevel=0;		// include nesting level
static short		yyold=T_BAD;		// save character replaced with '\0'

static FILE		*yyfp=stdin;		// input stream
static L_TOKEN		Lookahead=L_BAD;	// lookahead token


static void	LexBad(char ch);
static L_TOKEN	LexCheckKeyword(char *name, short len, L_TOKEN tok);
static int	LexCompare(const void *a, const void *b);
static char	LexEscape(char ch);
static Boolean	LexRead(void);
static Boolean	LexUnInclude(void);


// Lex returns the next token found in the current input stream, ignoring
// white space and comments.  L_EOI is returned when we run out of input.
// Use yytext to find out the literal text and yylen for its length.
//
// NOTE:	These strings are not ASCIIZ (with the exception of
//		L_ASCIIZ's).  Use yylen to find out how many characters are
// valid.  If you would like to treat yytext as an ASCIIZ string, do the
// following: call LexTerm() to make it ASCIIZ, use it or copy it, and then
// call LexUnTerm() to change it back.

L_TOKEN Lex(void) {
	char	*current, *dst;
	Symbol	*sym;
	short	num;

	current = yytext + yylen;	// skip current lexeme
	while (true) {			// get the next one
		while (isspace(*current))
			++current;
		while (!*current || (*(current)==COMMENT && *(current+1)==COMMENT)) {
			//
			// get new lines, skipping any leading white space
			// on the line, until a nonblank line is found
			//
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
			//
			// get the next token
			//
			yytext = current;
			yylen = 1;
			switch (*current) {
				case COMMENT:
					if (*(current+1) == COMMENT)
						*(current+1) = '\0';
					else
						LexBad(*current);
					break;
				case EOF:
					return (L_EOI);
				case '{':
					return (L_LCURLY);
				case '}':
					return (L_RCURLY);
				case '(':
					return (L_LPAREN);
				case ')':
					return (L_RPAREN);
				case ',':
					return (L_COMMA);
				case ';':
					return (L_SEMI_COLON);
				case '*':
					return (L_ASTERISK);
				case ':':
					return (L_COLON);
				case '=':
					return (L_EQUALS);
				case '"':
					yytext++;
					while (*++current && *current != '"')
						if (*current=='\\' && *(current+1))
							current++;
					if (!*current) {
						ErrorSevere("Unterminated String");
						yylen = (int) (current - yytext);
						}
					else {
						//
						// We want to get rid of the surrounding
						// quotes, so we point past first one
						// and get rid of second one by making it
						// ASCIIZ!  NOTE: Length is off by 1.
						//
						*current = '\0';
						//
						// Now deal with escape sequences.
						//
						num = 0;
						dst = yytext;
						while ((dst=strchr(dst, '\\')) != NULL) {
							current = dst + 1;
							*dst++ = LexEscape(*current++);
							memmove(dst, current, strlen(current)+1);
							num++;
							}
						yylen = strlen(yytext) + 1;
						if (num) {
							dst = yytext + num;
							memmove(dst, yytext, yylen);
							yytext = dst;
							}
						}
					return (L_ASCIIZ);
				case '\'':
					yytext++;
					//
					// Check and deal with escape sequence
					//
					if (*yytext == '\\') {
						yytext++;
						*yytext = LexEscape(*yytext);
						}
					if (*(yytext+1) == '\'') {
						//
						// Overwrite quote so length is 1
						//
						yytext++;
						*yytext = *(yytext-1);
						}
					else
						ErrorSevere("Unterminated Character");
					yylen = 1;
					return (L_CHAR);
				case '#':
					current++;
					while (isalnum(*++current) || *current=='_')
						;
					yylen = (int)(current - yytext);
					switch (LexCheckKeyword(yytext, yylen, L_BAD)) {
						case L_DEFINE:
							while (isspace(*current))
								current++;
							//
							// get the symbol
							//
							yytext = current;
							while (isalnum(*current) || *current=='_')
								current++;
							yylen = (int)(current - yytext);
							LexTerm();
							sym = SymbolNew(yytext);
							LexUnTerm();
							while (isspace(*current))
								++current;
							//
							// get the value
							//
							yytext = current;
							if (*current == '-')
								current++;
							while (isdigit(*current))
								current++;
							yylen = (int)(current - yytext);
							LexTerm();
							sym->val = atoi(yytext);
							LexUnTerm();
							SymbolAdd(sym);
							break;
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
				case '.':
					while (isalnum(*++current) || *current=='_')
						;
					yylen = (int)(current - yytext);
					return (LexCheckKeyword(yytext, yylen, L_BAD));
				case '-':
					if (!isdigit(*(current+1))) {
						LexBad(*current);
						break;
						}
				case '0': case '1': case '2': case '3': case '4':
				case '5': case '6': case '7': case '8': case '9':
					while (isxdigit(*++current))
						;
					yylen = (int)(current - yytext);
					return (L_VALUE);
				default:
					if (!isalnum(*current) && *current!='_')
						LexBad(*current);
					else {
						while (isalnum(*++current) || *current=='_')
							;
						yylen = (int)(current - yytext);
						return (LexCheckKeyword(yytext, yylen, L_ID));
						}
					break;
				}
			}
		}
	}


// LexAdvance advances the Lookahead to the next input symbol.

void LexAdvance(void) {
	Lookahead = L_BAD;
	}


// LexBad issues a warning notifying the user that an unexpected character
// in the uinput stream was encountered.

static void LexBad(char ch) {
	ErrorWarning("Ignoring illegal input '%c'", ch);
	}


// Do a binary search for a possible keyword in KTab.  Return the L_TOKEN
// if it's in the table, L_ID otherwise.

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


// Comparison function called by bsearch.  Return 0 is the two strings are
// the same.

static int LexCompare(const void *a, const void *b) {
	return (strcmp(((KWORD *)a)->name, ((KWORD *)b)->name));
	}


// LexEscape() returns the character represented by the escape character ch.

static char LexEscape(char ch) {
	switch (ch) {
		case 'a': return ('\a');	// audible bell
		case 'b': return ('\b');	// backspace
		case 'f': return ('\f');	// formfeed
		case 'n': return ('\n');	// newline (linefeed)
		case 'r': return ('\r');	// carriage return
		case 't': return ('\t');	// tab (horizontal)
		case 'v': return ('\v');	// vertical tab
		case 'z': return (0x1A);	// ^Z
		}
	return (ch);
	}


// Call LexInclude when the input stream needs to be interrupted by a
// second input stream.

Boolean LexInclude(char *name) {
	char            *pp;			// file path pointer
	FILE		*fp=NULL;
	IncludeRecord	*inc;

	if ((pp=searchpath(name)) == NULL || (fp=fopen(pp, "rt")) == NULL)
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
	strcpy(yyname, pp);
	strupr(yyname);		// keep it consistent for error messages
	yyfp = fp;
	Lookahead = L_BAD;
	yyinclude = inc;
	yylevel++;
	return (true);
	}


// LexInList returns an index into the tokens list that match the current
// Lookahead token.  If there is no match, a value of -1 is returned.

short LexInList(L_TOKEN *tokens) {
	short 	i;

	if (Lookahead == L_BAD)
		Lookahead = Lex();
	for (i=0; *tokens; i++)
		if (*tokens++ == Lookahead)
			return (i);
	return (-1);
	}


// LexLook returns the current Lookahead symbol.

L_TOKEN LexLook(void) {
	if (Lookahead == L_BAD)
		Lookahead = Lex();
	return (Lookahead);
	}


// LexMatch returns true is token matches the current Lookahead symbol.

Boolean LexMatch(L_TOKEN token) {
	if (Lookahead == L_BAD)
		Lookahead = Lex();
	return ((Boolean)(token == Lookahead));
	}


// LexRead attempts to read the next line of data from the current data
// input stream.  On a failure, LexUnInclude is called and the read is
// reattempted.  This process continues until all of the nesting levels
// have been exhausted, at which point we return false.  Otherwise we
// return true.

static Boolean LexRead(void) {
	char	*ip=yybuffer;

	if (!yylevel)
		return (false);
	while (!fgets(ip, BUFLEN, yyfp))
		if (!LexUnInclude())
			return (false);
	return (true);
	}


// Use LexTerm() to make the current lexeme ASCIIZ.  It is imperative that
// LexUnTerm() is called when we are done and before Lex() is called, either
// directly or indirectly by LexMatch(), LexLook(), etc.

void LexTerm(void) {
	char	*s;

	if (yyold != T_BAD)
		ErrorFatal("Internal Error: LexTerm");
	s = yytext + yylen;
	yyold = *s;
	*s = '\0';
	}


// LexUnInclude closes the current input file and restores the environment
// of the previous include file.  This makes it appear as if the two files
// were really one huge file.  If no more levels of nesting exist, we
// return false, otherwise true.

static Boolean LexUnInclude(void) {
	IncludeRecord	*inc;

	if (yylevel < 1)
		return (false);
	fclose(yyfp);
	if (--yylevel < 1)
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
	return (true);
	}


// Use LexUnTerm() to undo what LexTerm() did, ie. make the current lexeme
// ASCIIZ.  Failure to pair these calls up can cause Lex() to fail to
// recognize any more tokens on the current line.

void LexUnTerm(void) {
	if (yyold == T_BAD)
		ErrorFatal("Internal Error: LexUnTerm");
	*(yytext+yylen) = yyold;
	yyold = T_BAD;
	}
