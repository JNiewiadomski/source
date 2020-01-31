#include <limits.h>
#include <math.h>
#include <string.h>

#include <cbase.h>

#include "h\error.h"
#include "h\lex.h"
#include "h\symbol.h"
#include "h\semantic.h"

struct dev_rec dev = {
	L_PDMA_32,		/* device selected */
	10000000.0,		/* crystal clock speed */
	0x0300,			/* base address */
	DMA_WORD_TRANSFER,	/* D.M.A. level = word */
	5,			/* interrupt level */
	DEV_WORD,		/* port mode = word I/O */
	1,			/* number of words to transfer */
	DEV_OUTPUT,		/* direction = output */
	DEV_AUTO_OFF,		/* auto-recycle = off */
	DEV_CLOCK_TIMER,	/* clock source = internal timer */
	0,			/* buffer address segment */
	0,			/* buffer address offset */
	65535U,			/* counter 0 */
	65535U,			/* counter 1 */
	2.328378E-03,		/* frequence of internal timer in Hz */
	429483622.5,		/* unit time in uS */
	DEV_INTER_DISABLE,	/* interrupt source = none */
	{ 0, 0, 0 }		/* auxiliary data bits */
	};

/*
 * The action pattern mask is added to the sym linked list.  If the mask
 * is less then WORD_BIT bytes in length, then the mask is prefixed by the
 * difference in '0's.  All '-' are converted to '0's.  Since they are don't
 * care's, it doesn't matter to the user.  The Gen module to generate
 * actions doesn't want '-'s either.
 */

void SemanticAction(Symbol *sym, char *mask, Q_TYPE qualify, long val) {
	Link	*p;
	short	diff;
	char	*s;

	p = SymbolNewLink();
	s = p->u.action.mask;
	diff = WORD_BIT - strlen(mask);
	while (diff--)
		*s++ = '0';
	strcpy(s, mask);
	for ( ; *s; s++)
		if (*s == '-')
			*s = '0';
	p->u.action.qualify = qualify;
	p->u.action.val = val;
	SymbolAddLink(sym, p);
	}


/*
 * Sets the selected base address in the device record.
 */

void SemanticDeviceBase(int address) {
	dev.baseAddress = address;
	}


/*
 * Sets the selected clock source and counter divisors if used in the device
 * record.  The counter devisors are calculated for optimum integer division
 * ratios for the internal timer.
 */

void SemanticDeviceClock(int source, long desired) {
	double	_e, new_val, partial;
	long	limit, div0, div1;

	dev.clockSource = source;
	if (source == DEV_CLOCK_EXTERNAL)
		return;		/* if source external, don't do calculation */
	if (desired < 1) {
		ErrorSevere("Invalid speed of %ld Hz selected", desired);
		return;
		}
	limit = sqrt(dev.xtalSpeed / desired);
	if (limit < 2 || limit > UINT_MAX) {
		ErrorSevere("Transfer rate of %ld Hz cannot be achieved", desired);
		return;
		}
	dev.counter0 = dev.counter1 = 0;
	for (_e=1E+10, div0=limit; div0>=2; div0--) {
		partial = dev.xtalSpeed / div0;
		div1 = (partial / desired) + .5;
		if (div1 > UINT_MAX)
			div0 = 1;
		else {
			new_val = fabs(desired - (partial / div1));
			if (_e > new_val) {
				_e = new_val;
				dev.counter0 = (Word)div0;
				dev.counter1 = (Word)div1;
				}
			}
		}
	dev.frequency = dev.xtalSpeed / ((double)dev.counter0 * (double)dev.counter1);
	dev.unitTime = 1000000.0 / dev.frequency;
	if (desired != dev.frequency)
		ErrorWarning("Actual clock speed will be %.15g Hz", dev.frequency);
	}


/*
 * Sets the selected interrupt source in the device record.  The autoRecycle
 * mode is implicitly set based on the interrupt source.
 */

void SemanticDeviceInterrupt(int source) {
	dev.interruptSource = source;
	dev.autoRecycle = (source == DEV_INTER_TERMINAL) ? DEV_AUTO_OFF : DEV_AUTO_ON;
	}


/*
 * Sets the selected port mode and D.M.A level in the device record.
 */

void SemanticDeviceMode(int mode) {
	dev.portMode = mode;
	dev.dmaLevel = (mode == DEV_BYTE) ? DMA_BYTE_TRANSFER : DMA_WORD_TRANSFER;
	}


/*
 * Sets the selected hardware device and crystal speed in the device record.
 */

void SemanticDeviceSelect(L_TOKEN select) {
	dev.select = select;
	dev.xtalSpeed = (select == L_PDMA_32) ? 10000000.0 : 0;
	}


/*
 * Copies the current lexeme to id.  If the current lexeme is longer than
 * MAX_ID_LEN, a diagnostic is issued and the name is truncated.  The symbol
 * table is searched for an entry indexed by id.  If found, a diagnostic
 * error message is issued and NULL returned.  Otherwise id is added to the
 * symbol table and a pointer to the symbol is returned.
 */

Symbol *SemanticID(T_TYPE type) {
	short	len;
	char	id[MAX_ID_LEN+1];
	Symbol	*sym;

	if ((len = yylen) > MAX_ID_LEN) {
		len = MAX_ID_LEN;
		ErrorWarning("Identifier truncated to first %d characters", len);
		}
	strncpy(id, yytext, len);
	*(id + len) = '\0';
	if (SymbolFind(id) != NULL) {
		ErrorSevere("Illegal attempt to redefine identifier '%s'", id);
		return (NULL);
		}
	sym = SymbolNew(id, type);
	SymbolAdd(sym);
	return (sym);
	}


/*
 * The state pattern mask is added to the sym linked list.  If the mask
 * is less then BYTE_BIT bytes in length, then the mask is prefixed by the
 * difference number of don't care characters.  For the andMask, we convert
 * all #'s into 1's and all -'s into 0's.  For the cmpMask, we convert all
 * #'s into #'s and all -'s into 0's.  This setup allows us to do a quick
 * check if the state's match by ANDing the input pattern with the andMask
 * and then comparing it to cmpMask.
 */

void SemanticState(Symbol *sym, char *mask) {
	Link	*p;
	short	diff;
	char	*s, pat[BYTE_BIT+1];
	Word	andMask, cmpMask, bitPos;

	p = SymbolNewLink();
	s = pat;
	diff = BYTE_BIT - strlen(mask);
	while (diff--)
		*s++ = '-';
	strcpy(s, mask);
	for (andMask=cmpMask=0, bitPos=0x0080, s=pat; *s; bitPos>>=1, s++)
		switch (*s) {
			case '0':
				andMask |= bitPos;
				break;
			case '1':
				andMask |= bitPos;
				cmpMask |= bitPos;
				break;
			}
	p->u.state.andMask = andMask;
	p->u.state.cmpMask = cmpMask & andMask;
	SymbolAddLink(sym, p);
	}


/*
 * The current lexeme, which is an L_STRING, is made into a link and added
 * to the chain of sym.
 */

void SemanticText(Symbol *sym) {
	Link	*p;

	p = SymbolNewLink();
	if ((p->u.text = strdup(yytext)) == NULL)
		e_OutOfMemory("SemanticText");
	SymbolAddLink(sym, p);
	}
