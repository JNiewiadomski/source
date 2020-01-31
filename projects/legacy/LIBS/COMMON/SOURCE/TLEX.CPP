#include <ctype.h>
#include <dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\tlex.h>

// **************************
// * Build-In KeyWord Table *
// **************************

static KeyWord	pKTab[] = {
	{ "#include",	L_INCLUDE	},
	{ "#pragma",	L_PRAGMA	},
	};

static int	pKTabSize = NUMELE(pKTab);

// ****************************
// * Include Stream Structure *
// ****************************

// The TInc structure provides the mechanism to keep track of the current
// location within the current input stream.  The input stream can change
// when the #include directive is encountered.  The number of include levels
// is limited by available memory.
//
// The longest line length supported is BUFSIZ, which had the value of 512.

class TInc {
	enum	{ T_BAD=-1 };

	int	fYYOld;			// save character replaced with '\0'
public:
	istream	*fIn;			// input stream
	char	fYYName[MAXPATH];	// input file name
	char	fYYBuffer[BUFSIZ+1];	// input line buffer
	LToken	fLookahead;		// lookahead token
	char	*fYYText;		// lexeme (not '\0' terminated)
	int	fYYLen;			// lexeme length
	int	fYYLineNo;		// input line number
	TInc	*fParent;		// parent input stream

	TInc(istream *in, const char *name, TInc *parent);

	void	Advance(void) { fLookahead=L_EOI; }
	Boolean	ReadLine(void);
	void	Term(void);
	void	UnTerm(void);
	};


// Initialize all of the fields within the include structure.

TInc::TInc(istream *in, const char *name, TInc *parent) {
	fIn = in;
	if (name) {
		strcpy(fYYName, name);
		strupr(fYYName);
		}
	else
		*fYYName = '\0';
	*fYYBuffer = '\0';
	fLookahead = L_EOI;
	fYYText = "";
	fYYLen = 0;
	fYYLineNo = 0;
	fParent = parent;
	fYYOld = T_BAD;
	}


// Returns True if a line of text was successfully read from the input
// stream.

Boolean TInc::ReadLine(void) {
	fIn->get(fYYBuffer, sizeof(fYYBuffer));
	if (fIn->eof())
		return (False);
	fIn->get();			// remove line terminator
	return (True);
	}


// Use Term() to make the current lexeme ASCIIZ.  Use UnTerm() before doing
// any other structure manipulation.

void TInc::Term(void) {
	char	*s;

	if (fYYOld != T_BAD)		// Term() already called
		return;
	s = fYYText + fYYLen;
	fYYOld = *s;
	*s = '\0';
	}


// Use UnTerm() to undo what Term() did, ie. Term() made the current lexeme
// ASCIIZ.

void TInc::UnTerm(void) {
	if (fYYOld == T_BAD)
		return;
	fYYText[fYYLen] = fYYOld;
	fYYOld = T_BAD;
	}

// **********************
// * TLex Class Methods *
// **********************

// The TLex class implements a general purpose lexical analyzer.  The lexical
// analyzer is based upon the C++ language including:
//
//	// introduces comments which span to the end of the line
//	#include directive includes other source files
//	the basic source code tokens, such as strings, character constants,
//		and arithmetic and logical operators
//
// The starting input stream may be stdin, a buffer in memory or a file.  The
// starting input stream must already be defined.

TLex::TLex(istream *in, const char *name) {
	fOrgIn = in;
	if ((fOrgName = new char[strlen(name)+1]) != NULL)
		strcpy(fOrgName, name);
	fInc = NULL;
	fKTab = NULL;
	fKTabSize = 0;
	Reset();				// start from the beginning
	}


// TLex class destructor releases allocated resources.

TLex::~TLex() {
	Flush();				// release resources
	delete fOrgName;
	}


// Next call to Look() or Get() will return next unread token.

void TLex::Advance(void) {
	if (fInc)
		fInc->Advance();
	}


// Do a binary search for a possible keyword in list.  Return the LToken
// if it's in the table, bad_tok otherwise.

LToken TLex::CheckKeyword(const KeyWord *list, Word size, LToken bad_tok) {
	char	old;
	KeyWord	*found, dummy;

	if (!fInc || !list)
		return (bad_tok);
	old = fInc->fYYText[fInc->fYYLen];
	fInc->fYYText[fInc->fYYLen] = '\0';
	dummy.name = fInc->fYYText;
	found = (KeyWord *) bsearch(&dummy, list, size, sizeof(KeyWord), Compare);
	fInc->fYYText[fInc->fYYLen] = old;
	return (found ? found->tok : bad_tok);
	}


// Comparison function called by bsearch.  Return 0 is the two strings are
// the same.

int TLex::Compare(const void *a, const void *b) {
	return (strcmp(((KeyWord *)a)->name, ((KeyWord *)b)->name));
	}


// ConvertEscapes() converts all escape sequences within the current lexeme
// to their character counterparts.  This function should only be called for
// L_ASCIIZ and L_CHAR_CONST tokens.

void TLex::ConvertEscapes(void) {
	char	*dst, *temp;

	dst = fInc->fYYText;
	while ((dst=strchr(dst, '\\')) != NULL) {
		temp = dst + 1;
		*dst++ = Escape(*temp++);
		memmove(dst, temp, strlen(temp)+1);
		}
	}


// Escape() returns the character represented by the escape character ch.

char TLex::Escape(char ch) {
	switch (ch) {
		case 'a': return ('\a');	// audible bell
		case 'b': return ('\b');	// backspace
		case 'f': return ('\f');	// formfeed
		case 'n': return ('\n');	// newline (linefeed)
		case 'r': return ('\r');	// carriage return
		case 't': return ('\t');	// horizontal tab
		case 'v': return ('\v');	// vertical tab
		}
	return (ch);
	}


// Flush() returns the system resources used to implement the include
// mechanism.

void TLex::Flush(void) {
	while (UnInclude())			// release memory
		;
	}


// Get() returns the next token found in the current input stream, ignoring
// white space and comments.  L_EOI is returned when we run out of input.
// Use GetText() to find out the literal text and GetLen() for its length.
//
// *** NOTE ***
//
// These strings are not ASCIIZ (with the exception of L_ASCIIZ and
// L_CHAR_CONST).  Use fYYLen to find out how many characters are valid.  If
// you would like to treat fYYText as an ASCIIZ string, do the following:
// call Term() to make it ASCIIZ, use it or copy it, and then call UnTerm()
// to change it back.

LToken TLex::Get(void) {
	if (!fInc)				// any more input?
		return (L_EOI);
	if (fInc->fLookahead != L_EOI) {	// if we have a lookahead,
		LToken token=fInc->fLookahead;	// return its value
		fInc->fLookahead = L_EOI;
		return (token);
		}

	char	*current;

	current = fInc->fYYText + fInc->fYYLen;	// skip current lexeme
	while (True) {				// loop for # directives
		while (True) {
			while (isspace(*current))
				++current;
			if (*current && *current != '/' && *(current+1) != '/')
				break;		// we're at next token
			if (!ReadLine())
				return (L_EOI);
			current = fInc->fYYBuffer;
			++fInc->fYYLineNo;
			}
		fInc->fYYText = current;
		fInc->fYYLen = 1;
		switch (*current) {
			case '{': return (L_LCURLY);
			case '}': return (L_RCURLY);
			case '(': return (L_LPAREN);
			case ')': return (L_RPAREN);
			case '[': return (L_LSQUARE);
			case ']': return (L_RSQUARE);
			case '~': return (L_BIT_NOT);
			case '*': return (L_ASTERISK);
			case '/': return (L_DIVIDE);
			case '%': return (L_MODULUS);
			case '+': return (L_PLUS);
			case '-': return (L_MINUS);
			case '&': return (L_BIT_AND);
			case '^': return (L_BIT_XOR);
			case '|': return (L_BIT_OR);
			case '!':
				if (*(current+1) == '=') {
					fInc->fYYLen = 2;
					return (L_NOT_EQUALS);
					}
				return (L_LOGICAL_NOT);
			case '<':
				switch (*(current+1)) {
					case '<':
						fInc->fYYLen = 2;
						return (L_LSHIFT);
					case '=':
						fInc->fYYLen = 2;
						return (L_LT_EQ);
					default:
						return (L_LT);
					}
			case '>':
				switch (*(current+1)) {
					case '>':
						fInc->fYYLen = 2;
						return (L_RSHIFT);
					case '=':
						fInc->fYYLen = 2;
						return (L_GT_EQ);
					default:
						return (L_GT);
					}
			case '=':
				if (*(current+1) == '=') {
					fInc->fYYLen = 2;
					return (L_EQUALS);
					}
				return (L_ASSIGN);
			case '"':
			case '\'':
				//
				// Save the quote mark
				//
				char quote=*current;
				//
				// Point past first quote.
				//
				fInc->fYYText++;
				for (current++; *current && *current!=quote; current++)
					if (*current == '\\' && *(current+1))
						current++;
				//
				// Don't go crazy if the string doesn't end with a
				// quote.
				//
				if (!*current)
					fInc->fYYLen = (int) (current - fInc->fYYText);
				else {
					//
					// Get rid of second quote by
					// making it ASCIIZ!
					//
					// NOTE: fYYLen is off by 1.
					//
					*current = '\0';
					fInc->fYYLen = strlen(fInc->fYYText) + 1;
					}
				return (quote=='"' ? L_ASCIIZ : L_CHAR_CONST);
			case '#':
				current++;
				while (isalpha(*++current))
					;
				fInc->fYYLen = (int)(current - fInc->fYYText);
				switch (CheckKeyword(pKTab, pKTabSize, L_BAD)) {
					case L_INCLUDE:
						while (isspace(*current))
							current++;
						if (*current == '"')
							current++;
						fInc->fYYText = ++current;
						while (*current && *current != '"')
							current++;
						if (*current) {
							fInc->fYYLen = (int)(current - fInc->fYYText + 1);
							*current = '\0';
							}
						else
							fInc->fYYLen = (int)(current - fInc->fYYText);
						Include(fInc->fYYText);
						break;
					case L_PRAGMA:
						return (L_PRAGMA);
					}
				// if unrecognized # directive, eat it up
				break;
			case '.':
				if (!isdigit(*current+1))
					return (L_BAD);
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				while (isdigit(*++current) || *current == '.')
					;
				fInc->fYYLen = (int)(current - fInc->fYYText);
				return (L_VALUE);
			default:
				if (isalnum(*current) || *current=='_') {
					while (isalnum(*++current) || *current=='_')
						;
					fInc->fYYLen = (int)(current - fInc->fYYText);
					return (CheckKeyword(fKTab, fKTabSize, L_ID));
					}
				return (L_BAD);
			}
		}
	}


// Return the length of the current lexeme.  If the current lexeme is
// L_ASCIIZ or L_CHAR_CONST the length will be off by one.  Use strlen()
// instead.

int TLex::GetLen(void) {
	return (fInc ? fInc->fYYLen : 0);
	}


// Return the current lexeme.  Either use GetLen() to get the length of the
// lexeme, or call Term() and UnTerm() to make it ASCIIZ.

const char *TLex::GetText(void) {
	return (fInc ? fInc->fYYText : "");
	}


// Include() changes the current input stream to the file specified by name.
// The DOS PATH is followed until the file is found.  A file stream is
// allocated and the old input stream is chained to the new input stream.
// All subsequent input will come from the current input stream until eof is
// reached, at which point the previous input stream is restored.

Boolean TLex::Include(const char *name) {
	char		*path;
	ifstream	*in;
	TInc		*new_inc;

	if ((path = searchpath(name)) == NULL)
		return (False);
	if ((in = new ifstream(path)) == NULL)
		return (False);
	if ((new_inc = new TInc(in, path, fInc)) == NULL) {
		delete in;			// clean up
		return (False);			// not enough memory
		}
	fInc = new_inc;				// we are the new instance
	return (True);
	}


// InList() returns an index into the tokens list that match the current
// fLookahead token.  If there is no match, a value of -1 is returned.

int TLex::InList(LToken *tokens) {
	int	i;

	if (fInc) {
		if (fInc->fLookahead == L_EOI)
			fInc->fLookahead = Get();
		for (i=0; *tokens; i++)
			if (*tokens++ == fInc->fLookahead)
				return (i);
		}
	return (-1);
	}


// Return the current token.  Another call to Look() or Get() will return
// the same token.

LToken TLex::Look(void) {
	if (!fInc)
		return (L_EOI);
	if (fInc->fLookahead == L_EOI)
		fInc->fLookahead = Get();
	return (fInc->fLookahead);
	}


// Returns True is the current token matches token.

Boolean TLex::Match(LToken token) {
	if (!fInc)
		return (False);
	if (fInc->fLookahead == L_EOI)
		fInc->fLookahead = Get();
	return ((Boolean)(token == fInc->fLookahead));
	}


// Reads a line from the current input stream.  If the current input stream
// is exhausted, it is released.  If a parent input stream exists, the line
// read is re-attempted.  This process continues until either a line is read
// or all input streams are released.  True is returned if a line was
// successfully read.

Boolean TLex::ReadLine(void) {
	if (!fInc)				// no more input
		return (False);
	while (!fInc->ReadLine())
		if (!UnInclude())
			return (False);
	return (True);
	}


// Reset() releases all include resources, repositions the input stream to
// the beginning of input, and reallocated a new input stream.

void TLex::Reset(void) {
	Flush();				// release resources
	fOrgIn->seekg(0L);			// rewind input stream
	fInc = new TInc(fOrgIn, fOrgName, NULL);
	}


void TLex::Term(void) {
	if (fInc)
		fInc->Term();
	}


// UnInclude() releases the TInc structure resources and restores the
// previous environment.  If a parent exists, the stream is disposed of as
// well.

Boolean TLex::UnInclude(void) {
	if (!fInc)				// no current input stream
		return (False);

	TInc	*parent_inc;

	parent_inc = fInc->fParent;
	if (parent_inc)				// if it has a parent,
		delete fInc->fIn;		// then we allocated memory
	delete fInc;
	fInc = parent_inc;
	return (fInc ? True : False);
	}


void TLex::UnTerm(void) {
	if (fInc)
		fInc->UnTerm();
	}
