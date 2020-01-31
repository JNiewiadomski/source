// This module implements a lexical analyzer.

#define NULL_ADR	0

typedef enum {
	L_EOI,		// end of input

	L_DEFINE,	// #define directive
	L_INCLUDE,	// #include directive

	L_FILE,		// .FILE directive
	L_MEMO,		// .MEMO directive

	L_NULL,		// NULL address argument

	L_BAR,		// bar resource type
	L_BLACK,	// black color
	L_BLUE,		// blue color
	L_BROWN,	// brown color
	L_COLOR,	// color resource type
	L_COORD,	// coord resource type
	L_CYAN,		// cyan color
	L_DARKGRAY,	// dark gray color
	L_DIALOG,	// dialog resource type
	L_ERROR,	// error resource type
	L_EUROPE,	// europe date format
	L_FLASH,	// menu name flash attribute
	L_FORM,		// form resource type
	L_GREEN,	// green color
	L_HELP,		// help resource type
	L_HIGH,		// monochrome high intensity attribute
	L_HIGHINVERSE,	// monochrome high intensity inverse attribute
	L_HIGHLINE,	// monochrome high intensity underline attribute
	L_INVERSE,	// monochrome inverse attribute
	L_INTERNAT,	// international resource type
	L_JAPAN,	// japan date format
	L_LIGHTBLUE,	// light blue color
	L_LIGHTCYAN,	// light cyan color
	L_LIGHTGRAY,	// light gray color
	L_LIGHTGREEN,	// light green color
	L_LIGHTMAGENTA,	// light magenta color
	L_LIGHTRED,	// light red color
	L_LINE,		// monochrome underline attribute
	L_LIST,		// list resource type
	L_LOW,		// monochrome low intensity attribute
	L_MAGENTA,	// magenta color
	L_MENU,		// menu resource type
	L_NOFLASH,	// menu name noflash attribute
	L_PRELOAD,	// preload resource attribute
	L_PROMPT,	// prompt resource type
	L_PURGEABLE,	// purgeable resource attribute
	L_READ,		// read statement
	L_RED,		// red color
	L_RESOURCE,	// resource statement
	L_STATUS,	// status resource type
	L_STRING,	// string resource type
	L_USA,		// usa date format
	L_VERSION,	// version resource type
	L_VIDEO,	// video resource type
	L_W0000,	// window frame
	L_W1111,	// window frame
	L_W2111,	// window frame
	L_W2222,	// window frame
	L_WHITE,	// white color
	L_WINDOW,	// window resource type
	L_WNONE,	// window frame
	L_YELLOW,	// yellow color

	L_ASCIIZ,	// asciiz string
	L_ASTERISK,	// asterisk *
	L_CHAR,		// character
	L_COLON,	// colon :
	L_COMMA,	// comma ,
	L_ID,		// identifier
	L_EQUALS,	// equals =
	L_LCURLY,	// left curly bracket {
	L_LPAREN,	// left parentheses (
	L_RCURLY,	// right curly bracket }
	L_RPAREN,	// right parentheses }
	L_SEMI_COLON,	// semicolon statement terminator ;
	L_VALUE,	// numeric value

	L_BAD		// catch-all
	} L_TOKEN;


extern char	*yytext;	// in lex.c
extern int	yylen;
extern int	yylineno;
extern char	yyname[];


L_TOKEN Lex(void);
void	LexAdvance(void);
Boolean LexInclude(char *name);
short	LexInList(L_TOKEN *tokens);
L_TOKEN	LexLook(void);
Boolean LexMatch(L_TOKEN token);
void	LexTerm(void);
void	LexUnTerm(void);
