#include <ctype.h>
#include <dir.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <cbase.h>
#include <ctool.h>

#include "h\common.h"
#include "h\error.h"
#include "h\gen.h"
#include "h\lex.h"
#include "h\symbol.h"
#include "h\semantic.h"

#define INIT_PATTERN	"*.INI"
#define PDI_EXT		".PDI"
#define BAT_EXT		".BAT"

#define SYNCH		L_SEMI_COLON

/*
 * convTab is a lookup table to do conversions of numbers.  In particular,
 * this table helps us determine the value of an ASCII hex digit by using
 * the ASCII value itself (upper case) to index into the table and determine
 * its value.  We are assuming that the lexical analyzer passes to us a
 * valid ASCII string representing an unsigned integer constant.
 */

static DWord	convTab[] = {	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
				0,  0,  0,  0,  0,  0,  0,
				10, 11, 12, 13, 14, 15
				};

static void	ParseActionGroup(void);
static void	ParseActionList(void);
static void	ParseCaseAction(void *state);
static void	ParseCaseActionBlock(void *state);
static void	ParseCaseActionGroup(void *state);
static void	ParseCaseList(void);
static void	*ParseCaseState(void);
static void	ParseDeviceClock(void);
static void	ParseDeviceInterrupt(void);
static void	ParseDeviceList(void);
static void	ParseDeviceMode(void);
static void	ParseDeviceSelect(void);
static int	ParseEdge(void);
static Boolean	ParseLegalLookahead(L_TOKEN first_arg, ...);
static void	ParseMaskBlock(Symbol *sym);
static void	ParseMaskLine(Symbol *sym);
static char	*ParseMaskString(void);
static Word	ParseOutput(void);
static Q_TYPE	ParseQualify(long *val);
static void	ParseSemiColon(void);
static void	ParseSkipStatement(void);
static long	ParseSpeed(void);
static void	ParseStart(void);
static void	ParseStartupList(void);
static void	ParseState(void);
static void	ParseStateList(void);
static void	ParseStatement(void);
static void	ParseTextBlock(Symbol *sym);
static void	ParseTextGroup(void);
static void	ParseTextLine(Symbol *sym);
static void	ParseTextList(void);
static long	ParseTime(void);
static Word	ParseValue(void);


// Parse parses the file name by using Lex to provide the next available
// token and using a recursive descent algorithm.  Parse will also attempt
// to include all files with the extension .INI in the current subrirectory
// at the start of compilation.  This allows the user to predefine
// environments that stay the same over many different compilations.

Boolean Parse(const char *arg_name, Boolean batch) {
	char		name[MAXFILE], ext[MAXEXT], file_name[MAXPATH];
	FILE		*fp;
	Boolean		ok;
	struct ffblk	ffblk;
	int		rval;

	SymbolInit();
	fnsplit(arg_name, NULL, NULL, name, ext);
	/*
	 * copy arg_name and append default extension if not specified
	 */
	strcpy(file_name, arg_name);
	if (strlen(ext) == 0)
		strcat(file_name, PDI_EXT);
	if (!LexInclude(file_name))
		ErrorFatal("Unable to open file '%s'", file_name);
	/*
	 * find and include all files with a .INI extension
	 */
	rval = findfirst(INIT_PATTERN, &ffblk, 0);
	while (rval == 0) {
		LexInclude(ffblk.ff_name);
		rval = findnext(&ffblk);
		}
	/*
	 * now we can begin to parse the source files
	 */
	ParseStart();
	if (batch) {
		strcpy(file_name, name);
		strcat(file_name, BAT_EXT);
		if (ErrorNoSevere()) {
			fp = fopen(file_name, "wt");
			fprintf(fp, "%s %s /address %d /batch /interrupt %d", PROGRAM_NAME, arg_name, dev.baseAddress, dev.interruptLevel);
			fclose(fp);
			}
		else
			unlink(file_name);
		}
	ok = ErrorNoSevere();
	if (ok)
		GenStart();
	else
		ErrorStats();
	return (ok);
	}


/*
 *	<action_group>		->	<id> = <mask_line>
 *				|	<id> = { <mask_block> }
 */

static void ParseActionGroup(void) {
	Symbol	*sym;

	if (!ParseLegalLookahead(L_ID, L_EOI))
		return;
	if ((sym = SemanticID(T_ACTION)) == NULL) {
		ParseSkipStatement();
		return;
		}
	LexAdvance();
	if (LexMatch(L_EQUAL))
		LexAdvance();
	else
		ErrorWarning("Inserting missing equal sign");
	if (LexMatch(L_LCURLY)) {
		LexAdvance();
		ParseMaskBlock(sym);
		if (LexMatch(L_RCURLY))
			LexAdvance();
		else
			ErrorWarning("Inserting missing right curly bracket");
		}
	else
		ParseMaskLine(sym);
	}


/*
 *	<action_list>		->	<action_group> ; <action_list>
 *				|	î
 */

static void ParseActionList(void) {
	while (LexMatch(L_ID)) {
		ParseActionGroup();
		ParseSemiColon();
		}
	}


/*
 *	<case_action>		->      <id>
 *				|	BEEP <value>
 *				|	CLEAR <output>
 *				|	ECHO <id>
 *				|	HALT
 *				|	SET <output>
 */

static void ParseCaseAction(void *state) {
	switch (LexLook()) {
		case L_ID:
			LexTerm();
			GenCaseAction(state, yytext);
			LexUnTerm();
			LexAdvance();
			break;
		case L_BEEP:
			LexAdvance();
			GenCaseActionBeep(state, ParseValue());
			break;
		case L_CLEAR:
			LexAdvance();
			GenCaseActionClear(state, ParseOutput());
			break;
		case L_ECHO:
			LexAdvance();
			if (LexMatch(L_ID)) {
				LexTerm();
				GenCaseActionEcho(state, yytext);
				LexUnTerm();
				LexAdvance();
				}
			else
				ErrorSevere("Expecting label to text string");
			break;
		case L_HALT:
			LexAdvance();
			GenCaseActionHalt(state);
			break;
		case L_SET:
			LexAdvance();
			GenCaseActionSet(state, ParseOutput());
			break;
		default:
			ErrorSevere("Expecting case action");
			break;
		}
	}


/*
 *	<case_action_block>	->      <case_action> ; <case_action_block>
 *				|	<case_action> ;
 */

static void ParseCaseActionBlock(void *state) {
	L_TOKEN	tok;

	do {
		ParseCaseAction(state);
		ParseSemiColon();
		tok = LexLook();
		} while (tok==L_ID || tok==L_BEEP || tok==L_CLEAR || tok==L_ECHO || tok==L_HALT || tok==L_SET);
	}


/*
 *	<case_action_group>	->      <case_action>
 *				|	{ <case_action_block> }
 */

static void ParseCaseActionGroup(void *state) {
	if (LexMatch(L_LCURLY)) {
		LexAdvance();
		ParseCaseActionBlock(state);
		if (ParseLegalLookahead(L_RCURLY, L_EOI))
			LexAdvance();
		}
	else
		ParseCaseAction(state);
	}


/*
 *	<case_list>		->      <case> <case_list>
 *				|	î
 *	<case>			->	IF <case_state> THEN <case_action_group> ;
 */

static void ParseCaseList(void) {
	void	*state;

	while (LexMatch(L_IF)) {
		LexAdvance();
		state = ParseCaseState();
		if (LexMatch(L_THEN))
			LexAdvance();
		else
			ErrorWarning("Inserting missing THEN statement");
		ParseCaseActionGroup(state);
		ParseSemiColon();
		}
	}


/*
 *	<case_state>		->      <id>
 *				|	<key_press>
 */

static void *ParseCaseState(void) {
	void	*p=NULL;

	switch (LexLook()) {
		case L_ID:
			LexTerm();
			p = GenCaseState(yytext);
			LexUnTerm();
			LexAdvance();
			break;
		case L_KEY:
			LexTerm();
			p = GenCaseKey(yytext);
			LexUnTerm();
			LexAdvance();
			break;
		default:
			ErrorSevere("Expecting ID or keypress");
			break;
		}
	return (p);
	}


/*
 *	<device_clock>		->	EXTERNAL
 *				|	TIMER <value> <speed>
 */

static void ParseDeviceClock(void) {
	long	desired=0L;

	switch (LexLook()) {
		case L_EXTERNAL:
			LexAdvance();
			SemanticDeviceClock(DEV_CLOCK_EXTERNAL, desired);
			break;
		case L_TIMER:
			LexAdvance();
			desired = ParseValue();
			desired *= ParseSpeed();
			SemanticDeviceClock(DEV_CLOCK_TIMER, desired);
			break;
		default:
			ErrorSevere("Expecting device clock source");
			break;
		}
	}


/*
 *	<device_interrupt>	->	EXTERNAL <edge>
 *				|	TERMINAL
 *				|	TIMER
 *				|	DISABLE
 */

static void ParseDeviceInterrupt(void) {
	switch (LexLook()) {
		case L_DISABLE:
			LexAdvance();
			SemanticDeviceInterrupt(DEV_INTER_DISABLE);
			break;
		case L_EXTERNAL:
			LexAdvance();
			SemanticDeviceInterrupt(ParseEdge());
			break;
		case L_TERMINAL:
			LexAdvance();
			SemanticDeviceInterrupt(DEV_INTER_TERMINAL);
			break;
		case L_TIMER:
			LexAdvance();
			SemanticDeviceInterrupt(DEV_INTER_TIMER);
			break;
		default:
			ErrorSevere("Expecting interrupt source");
			break;
		}
	}


/*
 *	<device_list>		->	<device_spec> <device_list>
 *				|	î
 *	<device_spec>		->	SELECT <device_select> ;
 *				|	BASE <value> ;
 *				|	MODE <device_mode> ;
 *				|	CLOCK <device_clock> ;
 *				|	INTERRUPT <device_interrupt>
 */

static void ParseDeviceList(void) {
	Boolean	done=false;

	while (!done) {
		switch (LexLook()) {
			case L_SELECT:
				LexAdvance();
				ParseDeviceSelect();
				break;
			case L_BASE:
				LexAdvance();
				SemanticDeviceBase(ParseValue());
				break;
			case L_MODE:
				LexAdvance();
				ParseDeviceMode();
				break;
			case L_CLOCK:
				LexAdvance();
				ParseDeviceClock();
				break;
			case L_INTERRUPT:
				LexAdvance();
				ParseDeviceInterrupt();
				break;
			default:
				return;
			}
		ParseSemiColon();
		}
	}


/*
 *	<device_mode>		->	@BYTE | @WORD
 */

static void ParseDeviceMode(void) {
	if (ParseLegalLookahead(L_BYTE, L_WORD, L_EOI)) {
		SemanticDeviceMode(LexMatch(L_BYTE) ? DEV_BYTE : DEV_WORD);
		LexAdvance();
		}
	}


/*
 *	<device_select>		->	@PDMA_32
 */

static void ParseDeviceSelect(void) {
	if (LexMatch(L_PDMA_32)) {
		LexAdvance();
		SemanticDeviceSelect(L_PDMA_32);
		}
	else {
		LexTerm();
		ErrorSevere("Unrecognized device '%s'", yytext);
		LexUnTerm();
		}
	}


/*
 *	<device_edge>		->	POSITIVE | NEGATIVE
 */

static int ParseEdge(void) {
	int	slope=DEV_INTER_EXT_POS;

	switch (LexLook()) {
		case L_POSITIVE:
			LexAdvance();
			break;
		case L_NEGATIVE:
			slope = DEV_INTER_EXT_NEG;
			LexAdvance();
			break;
		default:
			ErrorSevere("Expecting external interrupt slope");
			break;
		}
	return (slope);
	}


/*
 * Simple error detection and recovery.  Arguments are L_EOI-terminated
 * list of those tokens that can legitimately come next in the input.  If the
 * list is empty, the end of the file must come next.  Print an error message
 * if necessary.  Error recovery is performed by discarding all input symbols
 * until one that's in the input list is found.
 *
 * Return true if there's no error or if we recovered from the error,
 * false if we can't recover.
 */

#define MAXFIRST	16

static Boolean ParseLegalLookahead(L_TOKEN first_arg, ...) {
	va_list	args;
	L_TOKEN	tok;
	L_TOKEN	lookaheads[MAXFIRST], *p=lookaheads, *current;
	Boolean	error_printed=false;
	Boolean	rval=false;

	if (!first_arg)
		return (LexMatch(L_EOI));
	va_start(args, first_arg);
	*p++ = first_arg;
	while ((tok = va_arg(args, L_TOKEN)) != L_EOI && p < &lookaheads[MAXFIRST])
		*p++ = tok;
	*p++ = L_EOI;
	while (!rval && !LexMatch(SYNCH)) {
		for (current=lookaheads; current < p && !rval; ++current)
			if (LexMatch(*current))
				rval = true;
		if (!rval) {
			if (!error_printed) {
				LexTerm();
				ErrorSevere("Syntax error: '%s'", yytext);
				LexUnTerm();
				error_printed = true;
				}
			LexAdvance();
			}
		}
	va_end(args);
	if (!rval && LexMatch(SYNCH))
		LexAdvance();
	return (rval);
	}


/*
 *	<mask_block>		->	<mask_line> ; <mask_block>
 *				|	<mask_line> ;
 */

static void ParseMaskBlock(Symbol *sym) {
	ParseMaskLine(sym);
	ParseSemiColon();
	while (LexMatch(L_MASK)) {
		ParseMaskLine(sym);
		ParseSemiColon();
		}
	}


/*
 *	<mask_line>		->	<mask_string> <qualify>
 */

static void ParseMaskLine(Symbol *sym) {
	char	*mask;
	Q_TYPE	qualify;
	long	val;
	short	len;

	if ((mask = ParseMaskString()) == NULL)
		return;
	if ((len = strlen(mask)) > WORD_BIT) {
		mask += len - WORD_BIT;
		ErrorWarning("Only the last %d characters used", WORD_BIT);
		}
	qualify = ParseQualify(&val);
	SemanticAction(sym, mask, qualify, val);
	}


/*
 *	<mask_string>		->	[ <mask> ]
 *	<mask>			->	0 <mask>
 *				|	1 <mask>
 *				|	- <mask>
 *				|	* <mask>
 *				|	<blank> <mask>
 *				|	î
 */

static char *ParseMaskString(void) {
	static char	buff[WORD_BIT+1];
	char		*src, *dst, ch;
	short		len, max;
	Boolean		err;

	if (!LexMatch(L_MASK)) {
		ErrorSevere("Expecting a mask string");
		return (NULL);
		}
	src = yytext + 1;
	max = yylen - 1;
	dst = buff;
	len = 0;
	err = false;
	while (max-- > 0 && len < WORD_BIT && (ch = *src++) != ']')
		switch (ch) {
			case '0':
			case '1':
			case '-':
			case '*':
				*dst++ = ch;
				len++;
				break;
			case ' ':
				break;
			default:
				err = true;
				break;
			}
	if (err)
		ErrorWarning("Illegal characters in mask string ignored");
	*dst = '\0';
	LexAdvance();
	return (buff);
	}


/*
 *	<output>		->	@AUX0 | @AUX1 | @AUX2
 */

static Word ParseOutput(void) {
	Word	aux=0;

	if (!LexMatch(L_OUTPUT))
		ErrorSevere("Expecting auxiliary output");
	else {
		aux = *(yytext+yylen-1) - '0';
		LexAdvance();
		}
	return (aux);
	}


/*
 *	<qualify>		->      REPEAT ( <value> )
 *				|	TIME ( <value> <time> )
 *				|	î
 */

static Q_TYPE ParseQualify(long *val) {
	Q_TYPE	qualify;

	switch (LexLook()) {
		case L_REPEAT:
			qualify = Q_REPEAT;
			LexAdvance();
			if (LexMatch(L_LPAREN))
				LexAdvance();
			else
				ErrorWarning("Inserting missing left parenthesis");
			*val = ParseValue();
			if (LexMatch(L_RPAREN))
				LexAdvance();
			else
				ErrorWarning("Inserting missing right parenthesis");
			break;
		case L_TIME:
			qualify = Q_TIME;
			LexAdvance();
			if (LexMatch(L_LPAREN))
				LexAdvance();
			else
				ErrorWarning("Inserting missing left parenthesis");
			*val = ParseValue() * ParseTime();
			if (LexMatch(L_RPAREN))
				LexAdvance();
			else
				ErrorWarning("Inserting missing right parenthesis");
			break;
		default:
			qualify = Q_NONE;
			break;
		}
	return (qualify);
	}


/*
 * ParseSemiColon has the job of verifying that the current lookahead
 * character is a ';'.  If not, an error message is issued and tokens
 * are discarded until an L_SEMI_COLON is found or L_EOI is reached.
 */

static void ParseSemiColon(void) {
	L_TOKEN	tok;

	if (!LexMatch(L_SEMI_COLON)) {
		ErrorSevere("Expecting the statement terminator ';'");
		while ((tok = LexLook()) != L_SEMI_COLON && tok != L_EOI)
			LexAdvance();
		}
	LexAdvance();
	}


/*
 * Discard current statement by advancing lexical tokens until we find a
 * statement terminator or we encounter the end of input.
 */

static void ParseSkipStatement(void) {
	L_TOKEN	tok;

	while ((tok=LexLook()) != L_EOI && tok != SYNCH)
		LexAdvance();
	}


/*
 *	<speed>			->	HZ | KHZ | MHZ
 */

static long ParseSpeed(void) {
	long	speed=0L;

	switch (LexLook()) {
		case L_HZ:
			LexAdvance();
			speed = 1L;
			break;
		case L_KHZ:
			LexAdvance();
			speed = 1000L;
			break;
		case L_MHZ:
			LexAdvance();
			speed = 1000000L;
			break;
		default:
			ErrorSevere("Expecting speed measure");
			break;
		}
	return (speed);
	}


/*
 * The parse begins with the nonterminal <start>.
 * 	<start>			->	<statement_list> .END
 *	<statement_list>	->	<statement> <statement_list>
 *      			|	î
 */

static void ParseStart(void) {
	Boolean	done=false;

	while (!done) {
		ParseLegalLookahead(L_INCLUDE, L_DEVICE, L_TEXT, L_STATE, L_ACTION, L_CASE, L_STARTUP, L_END, L_EOI);
		switch (LexLook()) {
			case L_INCLUDE:
			case L_DEVICE:
			case L_TEXT:
			case L_STATE:
			case L_ACTION:
			case L_CASE:
			case L_STARTUP:
				ParseStatement();
				break;
			case L_END:
				LexAdvance();
				done = true;
				break;
			case L_EOI:
				ErrorWarning("Inserting missing .END statement");
				done = true;
				break;
			default:
				LexTerm();
				ErrorSevere("Unrecognized statement '%s'", yytext);
				LexUnTerm();
				LexAdvance();
				break;
			}
		}
	if (!LexMatch(L_EOI))
		ErrorWarning("Statements following .END will be ignored");
	}


/*
 *	<startup_list>		->      <case_action> ; <startup_list>
 *				|	î
 */

static void ParseStartupList(void) {
	L_TOKEN	tok;

	tok = LexLook();
	while (tok==L_ID || tok==L_BEEP || tok==L_CLEAR || tok==L_ECHO || tok==L_HALT || tok==L_SET) {
		ParseCaseAction(NULL);
		ParseSemiColon();
		tok = LexLook();
		}
	}


/*
 *	<state>			->	<id> = <mask_string>
 */

static void ParseState(void) {
	Symbol	*sym;
	char	*mask, *s;
	short	len;

	if (!LexMatch(L_ID))
		return;
	if ((sym = SemanticID(T_STATE)) == NULL) {
		ParseSkipStatement();
		return;
		}
	LexAdvance();
	if (LexMatch(L_EQUAL))
		LexAdvance();
	else
		ErrorWarning("Inserting missing equal sign");
	if ((mask = ParseMaskString()) != NULL) {
		if ((s = strchr(mask, '*')) != NULL) {
			do {
				*s = '-';
				} while ((s = strchr(s, '*')) != NULL);
			ErrorWarning("Wild card '*' treated as don't care");
			}
		if ((len = strlen(mask)) > BYTE_BIT) {
			mask += len - BYTE_BIT;
			ErrorWarning("Only the last %d characters used", BYTE_BIT);
			}
		SemanticState(sym, mask);
		}
	}


/*
 *	<state_list>		->	<state> ; <state_list>
 *				|	î
 */

static void ParseStateList(void) {
	while (LexMatch(L_ID)) {
		ParseState();
		ParseSemiColon();
		}
	}


/*
 *	<statement>		->	.DEVICE <device_list>
 *				|	.TEXT <text_list>
 *				|	.STATE <state_list>
 *				|	.ACTION <action_list>
 *				|	.CASE <case_list>
 *				|	.STARTUP <startup_list>
 *				|	î
 */

static void ParseStatement(void) {
	switch (LexLook()) {
		case L_DEVICE:
			LexAdvance();
			ParseDeviceList();
			break;
		case L_TEXT:
			LexAdvance();
			ParseTextList();
			break;
		case L_STATE:
			LexAdvance();
			ParseStateList();
			break;
		case L_ACTION:
			LexAdvance();
			ParseActionList();
			break;
		case L_CASE:
			LexAdvance();
			ParseCaseList();
			break;
		case L_STARTUP:
			LexAdvance();
			ParseStartupList();
			break;
		}
	}


/*
 *	<text_block>		->	<text_line> ; <text_block>
 *				|	<text_line> ;
 */

static void ParseTextBlock(Symbol *sym) {
	do {
		ParseTextLine(sym);
		ParseSemiColon();
		} while (LexMatch(L_STRING));
	}


/*
 *	<text_group>		->	<id> = <text_line>
 *				|	<id> = { <text_block> }
 */

static void ParseTextGroup(void) {
	Symbol	*sym;

	if (!LexMatch(L_ID))
		return;
	if ((sym = SemanticID(T_TEXT)) == NULL) {
		ParseSkipStatement();
		return;
		}
	LexAdvance();
	if (LexMatch(L_EQUAL))
		LexAdvance();
	else
		ErrorWarning("Inserting missing equal sign");
	if (LexMatch(L_LCURLY)) {
		LexAdvance();
		ParseTextBlock(sym);
		if (LexMatch(L_RCURLY))
			LexAdvance();
		else
			ErrorWarning("Inserting missing right curly bracket");
		}
	else
		ParseTextLine(sym);
	}


/*
 *	<text_line>		->	" <text> "
 */

static void ParseTextLine(Symbol *sym) {
	if (LexMatch(L_STRING)) {
		SemanticText(sym);
		LexAdvance();
		}
	else
		ErrorSevere("Expecting a string constant");
	}


/*
 *	<text_list>		->	<text_group> ; <text_list>
 *				|	î
 */

static void ParseTextList(void) {
	while (LexMatch(L_ID)) {
		ParseTextGroup();
		ParseSemiColon();
		}
	}


/*
 *	<time>			->	US | MS | S
 */

static long ParseTime(void) {
	long	time=0L;

	switch (LexLook()) {
		case L_US:
			time = 1L;
			LexAdvance();
			break;
		case L_MS:
			time = 1000L;
			LexAdvance();
			break;
		case L_S:
			time = 1000000L;
			LexAdvance();
			break;
		default:
			break;
		}
	return (time);
	}


/*
 * ParseValue verifies that the next token is an unsigned integer.  The
 * value is converted from ASCII to unsigned integer doing any needed
 * conversion such as binary-to-decimal or hex-to-decimal.
 */

#define ERR_UINT	"Constant unrepresentable as an unsigned integer"

static Word ParseValue(void) {
	Word	radix=10;
	short	len, maxLen=5;
	DWord	val=0L;
	char	*cur;

	if (LexMatch(L_UINT)) {
		len = yylen;
		switch (*(yytext+yylen-1)) {
			case 'b':
			case 'B':
				radix = 2;
				len--;
				maxLen = 16;
				break;
			case 'h':
			case 'H':
				radix = 16;
				len--;
				maxLen = 4;
				break;
			}
		cur = yytext;
		while (len && *cur == '0') {
			cur++;
			len--;
			}
		if (len > maxLen)
			ErrorSevere(ERR_UINT);
		else {
			while (len--) {
				val *= radix;
				val += convTab[toupper(*cur++) - '0'];
				}
			if (val > UINT_MAX) {
				ErrorSevere(ERR_UINT);
				val = 0L;
				}
			}
		LexAdvance();
		}
	else
		ErrorSevere("Expecting an unsigned integer constant");
	return ((Word)val);
	}
