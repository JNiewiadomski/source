#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\coord.h>

#include "error.h"
#include "gen.h"
#include "lex.h"
#include "parse.h"
#include "symbol.h"

#include <seashell\resclass.h>
#include <seashell\resource.h>

#define SYNCH		L_SEMI_COLON

static void	ParseDirList(void);
static void	ParseResource(void);
static void	ParseSemiColon(void);
static void	ParseStateList(void);
static void	ParseStart(void);


// Parse parses the file name by using Lex to provide the next available
// token and using a recursive descent algorithm.  Parse will also attempt
// to include all files with the extension .INI in the current subrirectory
// at the start of compilation.  This allows the user to predefine
// environments that stay the same over many different compilations.

void Parse(char *in_name, char *out_path) {
	char	name[MAXFILE], ext[MAXEXT], file_name[MAXPATH];

	fnsplit(in_name, NULL, NULL, name, ext);
	//
	// copy in_name and append default extension if not specified
	//
	strcpy(file_name, in_name);
	if (strlen(ext) == 0)
		strcat(file_name, RES_IN_EXT);

	if (!LexInclude(file_name))
		ErrorFatal("Unable to open file '%s'", file_name);
	//
	// make a default output file name and resource memo
	//
	strcpy(file_name, name);
	strcat(file_name, RES_OUT_EXT);
	GenOutPath(out_path);
	GenOutFile(file_name);
	GenOutMemo("Resource File");
	//
	// now we can begin to parse the source files
	//
	ParseStart();
	if (ErrorStats()) {		// if severe error, delete out file
		GenDelete();
		exit(ERR_SYNTAX);
		}
	}


// Expects the next token to be a character.  If it is, it advances
// to the next token and returns the value.  Otherwise an error is issued
// and a -1 is returned.

char ParseChar(void) {
	char	val;

	if (LexMatch(L_CHAR)) {
		LexTerm();
		val = *yytext;
		LexUnTerm();
		LexAdvance();
		}
	else {
		ErrorSevere("Expecting an character value");
		val = -1;
		}
	return (val);
	}


//	<dir_list>:
//		<dir> ; <dir_list>
//		î
//	<dir>:
//		.FILE <string>
//		.MEMO <string>

static void ParseDirList(void) {
	Boolean	done=false;

	while (!done)
		switch (LexLook()) {
			case L_FILE:
				LexAdvance();
				if (LexMatch(L_ASCIIZ)) {
					GenOutFile(yytext);
					LexAdvance();
					}
				else
					ErrorSevere("Expecting quoted file name as argument");
				ParseSemiColon();
				break;
			case L_MEMO:
				LexAdvance();
				if (LexMatch(L_ASCIIZ)) {
					GenOutMemo(yytext);
					LexAdvance();
					}
				else
					ErrorSevere("Expecting quoted memo as argument");
				ParseSemiColon();
				break;
			default:
				done = true;
				break;
			}
	}


// Verifies that the expected token matches the current Lookahead token.
// If it does, we advance to the next token.  Otherwise we issue a diagnostic
// describing the dilemma.

void ParseMatchAdvance(L_TOKEN tok, char *msg) {
	if (LexMatch(tok))
		LexAdvance();
	else
		ErrorWarning("Inserting missing token '%s'", msg);
	}


//	<res>:
//		resource <res_type> ( <res_id> <res_attr_list> ) { <res_data> }
//	<res_type>:
//		bar      | color   | dialog | error  | form   | help   |
//		internat | list    | menu   | prompt | status | string |
//		version  | video   | window
//	<res_id>:
//		<id> | <value>
//	<res_attr_list>:
//		, <res_attr> <res_attr_list>
//		î
//	<res_attr>:
//		preload
//		purgeable

static void ParseResource(void) {
	short		res_type;
	short		res_id=0;
	Boolean		res_preload=false, res_purgeable=false;
	Byte		num;
	static L_TOKEN	resTypes[] = {
				L_BAR,		L_COLOR,	L_COORD,
				L_DIALOG,	L_ERROR,	L_FORM,
				L_HELP,		L_INTERNAT,	L_LIST,
				L_MENU,		L_PROMPT,	L_STATUS,
				L_STRING,	L_VERSION,	L_VIDEO,
				L_WINDOW,	L_EOI
				};

	if ((res_type = LexInList(resTypes)) == -1) {
		ErrorSevere("Invalid resource type specified");
		return;
		}
	LexAdvance();
	ParseMatchAdvance(L_LPAREN, "(");
	res_id = ParseValue();
	while (LexMatch(L_COMMA)) {
		LexAdvance();
		switch (LexLook()) {
			case L_PRELOAD:
				res_preload = true;
				break;
			case L_PURGEABLE:
				res_purgeable = true;
				break;
			default:
				LexTerm();
				ErrorSevere("Unrecognized attribute '%s'", yytext);
				LexUnTerm();
				break;
			}
		LexAdvance();
		}
	ParseMatchAdvance(L_RPAREN, ")");
	ParseMatchAdvance(L_LCURLY, "{");
	GenBinInsert(res_type, res_id, res_preload, res_purgeable);
	switch (res_type) {
		case R_Bar:
			num = GenResourceBar();
			break;
		case R_Color:
			num = GenResourceColor();
			break;
		case R_Coord:
			num = GenResourceCoord();
			break;
		case R_Dialog:
			num = GenResourceDialog();
			break;
		case R_Error:
			num = GenResourceError();
			break;
		case R_Form:
			num = GenResourceForm();
			break;
		case R_Help:
			num = GenResourceHelp();
			break;
		case R_Internat:
			num = GenResourceInternat();
			break;
		case R_List:
			num = GenResourceList();
			break;
		case R_Menu:
			num = GenResourceMenu();
			break;
		case R_Prompt:
			num = GenResourcePrompt();
			break;
		case R_Status:
			num = GenResourceStatus();
			break;
		case R_String:
			num = GenResourceString();
			break;
		case R_Version:
			num = GenResourceVersion();
			break;
		case R_Video:
			num = GenResourceVideo();
			break;
		case R_Window:
			num = GenResourceWindow();
			break;
		default:
			e_Internal("ParseResource");
			break;
		}
	GenBinNumSet(res_type, res_id, num);
	ParseMatchAdvance(L_RCURLY, "}");
	}


// ParseSemiColon has the job of verifying that the current lookahead
// character is a ';'.  If not, an error message is issued and tokens
// are discarded until an L_SEMI_COLON is found or L_EOI is reached.

static void ParseSemiColon(void) {
	L_TOKEN	tok;

	if (!LexMatch(L_SEMI_COLON)) {
		ErrorSevere("Expecting statement terminator ';'");
		while ((tok = LexLook()) != L_SEMI_COLON && tok != L_EOI)
			LexAdvance();
		}
	LexAdvance();
	}


//	<state_list>:
//		<read> ; <state_list>
//		<res> ; <state_list>
//		î
//	<read>:
//		read <string>
//	<res>:
//		resource <res_type> ( <res_id> <res_attr> ) { <res_data> }

static void ParseStateList(void) {
	Boolean		done=false;

	while (!done)
		switch (LexLook()) {
			case L_READ:
				LexAdvance();
				if (LexMatch(L_ASCIIZ))
					GenRead();
				else
					ErrorSevere("Expecting quoted file name to read");
				ParseSemiColon();
				break;
			case L_RESOURCE:
				LexAdvance();
				ParseResource();
				ParseSemiColon();
				break;
			default:
				done = true;
				break;
			}
	}


// The parse begins with the nonterminal <start>.
// 	<start>:
//		<dir_list> <state_list>

static void ParseStart(void) {
	ParseDirList();
	GenStart();
	ParseStateList();
	GenStop();
	if (!LexMatch(L_EOI)) {
		LexTerm();
		ErrorSevere("Parser not expecting more input (%s)", yytext);
		LexUnTerm();
		}
	}


// Expects the next token to be a numerical value.  If it is, it advances
// to the next token and returns the value.  Otherwise an error is issued
// and a -1 is returned.

short ParseValue(void) {
	Symbol	*sym;
	short	val;

	switch (LexLook()) {
		case L_VALUE:
			LexTerm();
			val = atoi(yytext);
			LexUnTerm();
			LexAdvance();
			break;
		case L_ID:
			LexTerm();
			if ((sym = SymbolFind(yytext)) == NULL)
				ErrorSevere("Identifier '%s' not defined", yytext);
			else
				val = sym->val;
			LexUnTerm();
			LexAdvance();
			break;
		case L_NULL:
			val = NULL_ADR;
			LexAdvance();
			break;
		default:
			ErrorSevere("Expecting <id> or <value>");
			val = -1;
			break;
		}
	return (val);
	}
