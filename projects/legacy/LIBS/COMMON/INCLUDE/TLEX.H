#ifndef _TLEX_H
#define _TLEX_H

#ifndef __FSTREAM_H
#include <fstream.h>
#endif

#ifndef __IOSTREAM_H
#include <iostream.h>
#endif

// **************************
// * Lexical Analyzer Class *
// **************************

typedef int	LToken;			// lexical token type definition

enum {
	L_EOI,		// end of input

	L_INCLUDE,	// #include directive
	L_PRAGMA,	// #pragma directive

	L_ASCIIZ,	// asciiz string
	L_ASTERISK,	// asterisk *
	L_BIT_AND,	// &
	L_BIT_NOT,	// ~
	L_BIT_OR,	// |
	L_BIT_XOR,	// ^
	L_CHAR_CONST,	// character
	L_DIVIDE,	// /
	L_ID,		// identifier
	L_ASSIGN,	// =
	L_EQUALS,	// ==
	L_GT,		// >
	L_GT_EQ,	// >=
	L_LCURLY,	// left curly bracket {
	L_LPAREN,	// left parentheses (
	L_LSHIFT,	// <<
	L_LSQUARE,	// left square bracket [
	L_LT,		// <
	L_LT_EQ,	// <=
	L_LOGICAL_NOT,	// !
	L_MINUS,	// -
	L_MODULUS,	// %
	L_NOT_EQUALS,	// !=
	L_PLUS,		// +
	L_RCURLY,	// right curly bracket }
	L_RPAREN,	// right parentheses )
	L_RSHIFT,	// >>
	L_RSQUARE,	// right square bracket ]
	L_VALUE,	// numeric value

	L_BAD		// catch-all; unrecognized token
	};

struct KeyWord {
	char	*name;			// keyword text
	LToken	tok;			// keyword token value
	};

class TInc;				// defined in source file

class TLex {
	istream	*fOrgIn;		// original input stream
	char	*fOrgName;		// original input name
	TInc	*fInc;			// current input stream

	static int	Compare(const void *a, const void *b);

	char		Escape(char ch);
	void		Flush(void);
	Boolean		Include(const char *name);
	Boolean		ReadLine(void);
	Boolean		UnInclude(void);
protected:
	KeyWord	*fKTab;			// keyword table pointer
	int	fKTabSize;		// size of the keyword table
public:
	TLex(istream *in, const char *name);
	~TLex();

	void		Advance(void);
	LToken		CheckKeyword(const KeyWord *list, Word size, LToken bad_tok);
	void		ConvertEscapes(void);
	LToken		Get(void);
	int		GetLen(void);
	const char	*GetText(void);
	int		InList(LToken *tokens);
	LToken		Look(void);
	Boolean		Match(LToken token);
	void		Reset(void);
	void		Term(void);
	void		UnTerm(void);
	};

#endif
