typedef enum {
	L_EOI,		/* end of input */

	L_INCLUDE,	/* #include preprocessor directive */

	L_ACTION,	/* .ACTION statement */
	L_CASE,		/* .CASE statement */
	L_DEVICE,	/* .DEVICE statement */
	L_END,		/* .END statement */
	L_STARTUP,	/* .STARTUP statement */
	L_STATE,	/* .STATE statement */
	L_TEXT,		/* .TEXT statement */

	L_BYTE,		/* @BYTE */
	L_DISABLE,	/* @DISABLE */
	L_EXTERNAL,	/* @EXTERNAL */
	L_KEY,		/* keyboard character */
	L_OUTPUT,	/* auxiliary outputs */
	L_PDMA_32,	/* @PDMA_32 */
	L_TERMINAL,	/* @TERMINAL */
	L_TIMER,	/* @TIMER */
	L_WORD,		/* @WORD */

	L_BASE,		/* BASE option */
	L_BEEP,		/* BEEP option */
	L_CLEAR,	/* CLEAR option */
	L_CLOCK,	/* CLOCK option */
	L_ECHO,		/* ECHO option */
	L_HALT,		/* HALT option */
	L_HZ,		/* Hz option */
	L_IF,		/* IF option */
	L_INTERRUPT,	/* INTERRUPT option */
	L_KHZ,		/* KHz option */
	L_MHZ,		/* MHz option */
	L_MODE,		/* MODE option */
	L_MS,		/* mS option */
	L_NEGATIVE,	/* NEGATIVE option */
	L_POSITIVE,	/* POSITIVE option */
	L_REPEAT,	/* REPEAT option */
	L_S,		/* S option */
	L_SELECT,	/* SELECT option */
	L_SET,		/* SET option */
	L_THEN,		/* THEN option */
	L_TIME,		/* TIME option */
	L_US,		/* uS option */

	L_EQUAL,	/* equal sign '=' */
	L_ID,		/* identifier */
	L_LCURLY,	/* left curly bracket '{' */
	L_LPAREN,	/* left parentheses '(' */
	L_MASK,		/* [ 0 | 1 | * | - ] */
	L_RCURLY,	/* right curly bracket '}' */
	L_RPAREN,	/* right parentheses ')' */
	L_SEMI_COLON,	/* statement terminator */
	L_STRING,	/* string constant */
	L_UINT,		/* unsigned integer */

	L_BAD		/* catch-all */
	} L_TOKEN;


extern char	*yytext;	/* in lex.c */
extern int	yylen;
extern int	yylineno;
extern char	yyname[];


L_TOKEN Lex(void);
void	LexAdvance(void);
Boolean LexInclude(char *name);
L_TOKEN	LexLook(void);
Boolean LexMatch(L_TOKEN token);
void	LexTerm(void);
void	LexUnTerm(void);
