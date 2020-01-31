#include <alloc.h>
#include <dos.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cbase.h>
#include <ctime.h>
#include <ckeyin.h>

#include "h\error.h"
#include "h\gen.h"
#include "h\lex.h"
#include "h\symbol.h"
#include "h\semantic.h"

#include "h\pdma32.h"

typedef enum { A_ACTION, A_BEEP, A_CLEAR, A_ECHO, A_HALT, A_SET } A_TYPE;
typedef enum { C_KEY, C_STATE } C_TYPE;

typedef struct case_action {
	A_TYPE			type;
	union {
		short		val;
		char		*id;
		} u;
	struct case_action	*next;
	} case_action;

typedef struct case_state {
	C_TYPE			type;
	union {
		char		*id;
		short		ch;
		} u;
	case_action		*action;
	case_action		*actionLast;
	struct case_state	*next;
	} case_state;


static void	GenPerformClear(Word val);
static void	GenPerformSet(Word val);
static void	GenPrint(char *format, ...);


/*
 * Structure and list to facilitate the expedient search for key presses.
 */

typedef struct {
	char	*name;
	short	key;
	} KPRESS;

static KPRESS KPressTab[] = {		/* alphabetic key sequences */
	{ "@DOWN_KEY",	kDown		},
	{ "@END_KEY",	kEnd		},
	{ "@ENTER_KEY",	kReturn		},
	{ "@ESC_KEY",	kEscape		},
	{ "@F10_KEY",	kF10		},
	{ "@F1_KEY",	kF1		},
	{ "@F2_KEY",	kF2		},
	{ "@F3_KEY",	kF3		},
	{ "@F4_KEY",	kF4		},
	{ "@F5_KEY",	kF5		},
	{ "@F6_KEY",	kF6		},
	{ "@F7_KEY",	kF7		},
	{ "@F8_KEY",	kF8		},
	{ "@F9_KEY",	kF9		},
	{ "@HOME_KEY",	kHome		},
	{ "@LEFT_KEY",	kLeft		},
	{ "@PGDN_KEY",	kPageDown	},
	{ "@PGUP_KEY",	kPageUp		},
	{ "@RIGHT_KEY",	kRight		},
	{ "@SPACE_KEY",	kSpace		},
	{ "@TAB_KEY",	kTab		},
	{ "@UP_KEY",	kUp		},
	};

static Word pageTable[] = {
	0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000,
	0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000
	};

static case_state	*caseState=NULL, *caseStateLast=NULL;
static case_state	*caseKey=NULL, *caseKeyLast=NULL;
static case_action	*caseStartup=NULL, *caseStartupLast=NULL;

static Boolean		stateChange=true;
static int		md, d[7];

static short	GenCheckKeypress(char *name);
static int	GenCompare(const void *a, const void *b);
static void	GenEnd(void);
static void	GenPatternMask(char *id);
static Boolean	GenPerform(case_action *action);
static void	GenPerformAction(char *id);
static void	GenPerformEcho(char *id);
static void	GenStates(void);


void GenCaseAction(void *obj, char *id) {
	Symbol		*sym;
	case_action	*action;
	case_state	*state;

	if ((sym = SymbolFind(id)) == NULL) {
		ErrorSevere("Referencing undefined .ACTION '%s'", id);
		return;
		}
	if (!IS_ACTION(sym)) {
		ErrorSevere("'%s' is not of type .ACTION", id);
		return;
		}
	if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
		e_OutOfMemory("GenCaseActionState1");
	action->type = A_ACTION;
	if ((action->u.id = strdup(id)) == NULL)
		e_OutOfMemory("GenCaseActionState2");
	action->next = NULL;
	if (obj == NULL) {	/* startup list */
		if (caseStartup == NULL)
			caseStartup = action;
		else
			caseStartupLast->next = action;
		caseStartupLast = action;
		}
	else {			/* case list */
		state = (case_state *)obj;
		if (state->action == NULL)
			state->action = action;
		else
			state->actionLast->next = action;
		state->actionLast = action;
		}
	}


void GenCaseActionBeep(void *obj, Word val) {
	case_action	*action;
	case_state	*state;

	if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
		e_OutOfMemory("GenCaseActionBeep");
	action->type = A_BEEP;
	action->u.val = val;
	action->next = NULL;
	if (obj == NULL) {	/* startup list */
		if (caseStartup == NULL)
			caseStartup = action;
		else
			caseStartupLast->next = action;
		caseStartupLast = action;
		}
	else {			/* case list */
		state = (case_state *)obj;
		if (state->action == NULL)
			state->action = action;
		else
			state->actionLast->next = action;
		state->actionLast = action;
		}
	}


void GenCaseActionClear(void *obj, Word val) {
	case_action	*action;
	case_state	*state;

	if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
		e_OutOfMemory("GenCaseActionClear");
	action->type = A_CLEAR;
	action->u.val = val;
	action->next = NULL;
	if (obj == NULL) {	/* startup list */
		if (caseStartup == NULL)
			caseStartup = action;
		else
			caseStartupLast->next = action;
		caseStartupLast = action;
		}
	else {			/* case list */
		state = (case_state *)obj;
		if (state->action == NULL)
			state->action = action;
		else
			state->actionLast->next = action;
		state->actionLast = action;
		}
	}


void GenCaseActionEcho(void *obj, char *id) {
	Symbol		*sym;
	case_action	*action;
	case_state	*state;

	if ((sym = SymbolFind(id)) == NULL) {
		ErrorSevere("Referencing undefined .TEXT '%s'", id);
		return;
		}
	if (!IS_TEXT(sym)) {
		ErrorSevere("'%s' is not of type .TEXT", id);
		return;
		}
	if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
		e_OutOfMemory("GenCaseActionEcho1");
	action->type = A_ECHO;
	if ((action->u.id = strdup(id)) == NULL)
		e_OutOfMemory("GenCaseActionEcho2");
	action->next = NULL;
	if (obj == NULL) {	/* startup list */
		if (caseStartup == NULL)
			caseStartup = action;
		else
			caseStartupLast->next = action;
		caseStartupLast = action;
		}
	else {			/* case list */
		state = (case_state *)obj;
		if (state->action == NULL)
			state->action = action;
		else
			state->actionLast->next = action;
		state->actionLast = action;
		}
	}


void GenCaseActionHalt(void *obj) {
	case_action	*action;
	case_state	*state;

	if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
		e_OutOfMemory("GenCaseActionHalt");
	action->type = A_HALT;
	action->next = NULL;
	if (obj == NULL) {	/* startup list */
		if (caseStartup == NULL)
			caseStartup = action;
		else
			caseStartupLast->next = action;
		caseStartupLast = action;
		}
	else {
		state = (case_state *)obj;
		if (state->action == NULL)
			state->action = action;
		else
			state->actionLast->next = action;
		state->actionLast = action;
		}
	}


void GenCaseActionSet(void *obj, Word val) {
	case_action	*action;
	case_state	*state;

	if ((action = (case_action *) malloc(sizeof(case_action))) == NULL)
		e_OutOfMemory("GenCaseActionSet");
	action->type = A_SET;
	action->u.val = val;
	action->next = NULL;
	if (obj == NULL) {	/* startup list */
		if (caseStartup == NULL)
			caseStartup = action;
		else
			caseStartupLast->next = action;
		caseStartupLast = action;
		}
	else {			/* case list */
		state = (case_state *)obj;
		if (state->action == NULL)
			state->action = action;
		else
			state->actionLast->next = action;
		state->actionLast = action;
		}
	}


void *GenCaseKey(char *id) {
	case_state	*key;

	if ((key = (case_state *) malloc(sizeof(case_state))) == NULL)
		e_OutOfMemory("GenCaseKey1");
	key->type = C_KEY;
	if ((key->u.ch = GenCheckKeypress(id)) == kNull)
		e_Internal("GenCaseKey2");
	key->action = NULL;
	key->actionLast = NULL;
	key->next = NULL;
	if (caseKey == NULL)
		caseKey = key;
	else
		caseKeyLast->next = key;
	caseKeyLast = key;
	return (key);
	}


void *GenCaseState(char *id) {
	Symbol		*sym;
	case_state	*state;

	if ((sym = SymbolFind(id)) == NULL) {
		ErrorSevere("Referencing undefined .STATE '%s'", id);
		return (NULL);
		}
	if (!IS_STATE(sym)) {
		ErrorSevere("'%s' is not of type .STATE", id);
		return (NULL);
		}
	if ((state = (case_state *) malloc(sizeof(case_state))) == NULL)
		e_OutOfMemory("GenCaseState1");
	state->type = C_STATE;
	if ((state->u.id = strdup(id)) == NULL)
		e_OutOfMemory("GenCaseState2");
	state->action = NULL;
	state->actionLast = NULL;
	state->next = NULL;
	if (caseState == NULL)
		caseState = state;
	else
		caseStateLast->next = state;
	caseStateLast = state;
	return (state);
	}


/*
 * Do a binary search for a possible keyword in KPressTab.  Return the
 * key press if it's in the table, kNull otherwise.
 */

static short GenCheckKeypress(char *name) {
	KPRESS	*found, dummy;

	dummy.name = name;
	found = (KPRESS *) bsearch(&dummy, KPressTab, sizeof(KPressTab)/sizeof(KPRESS), sizeof(KPRESS), GenCompare);
	return (found ? found->key : kNull);
	}


/*
 * Comparison function called by bsearch.  Return 0 is the two strings are
 * the same.
 */

static int GenCompare(const void *a, const void *b) {
	return (strcmp(((KPRESS *)a)->name, ((KPRESS *)b)->name));
	}


#define PDMA_INITIALIZE		0
#define PDMA_SETUP_TRANSFER	1
#define PDMA_STATUS		2
#define PDMA_SET_TIMER		3
#define PDMA_DIGITAL_OUTPUT	4
#define PDMA_DIGITAL_INPUT	5
#define PDMA_AUX_OUTPUT		6
#define PDMA_INTERRUPT_ENABLE	7
#define PDMA_INTERRUPT_DISABLE	8

#define PDMA_PORTA		0
#define PDMA_PORTB		1

#define K		1024UL
#define PAGE		(64UL * K)
#define BYTE_BLOCK	(2UL * PAGE)
#define WORD_BLOCK	(3UL * PAGE)

/*
 * GenStart attempts to initialize the PDMA-32 hardware, allocate memory,
 * and begin executing the startup and case sequences defines by the program
 * that was just parsed.
 */

void GenStart(void) {
	char	huge *block;
	DWord	size;
	short	i;

	md = PDMA_INITIALIZE;
	d[0] = dev.baseAddress;
	d[1] = dev.dmaLevel;
	d[2] = dev.interruptLevel;
	d[3] = dev.portMode;
	if (pdma32(md, d) != 0)
		ErrorRunTime("PDMA-32 hardware not found");
	size = dev.portMode==DEV_BYTE ? BYTE_BLOCK : WORD_BLOCK;
	if ((block = (char huge *)farmalloc(size)) == NULL)
		ErrorRunTime("Unable to allocate sufficient far memory");
	dev.buffSeg = FP_SEG(block);
	for (i=0; dev.buffSeg >= pageTable[i]; i++)
		;
	dev.buffSeg = pageTable[i];
	dev.buffOff = 0;
	md = PDMA_SET_TIMER;
	d[0] = dev.counter0;
	d[1] = dev.counter1;
	if (pdma32(md, d) != 0)
		ErrorRunTime("Unable to set PDMA-32 timers");
	md = PDMA_AUX_OUTPUT;
	for (i=NUMELE(dev.aux)-1; i>=0; i--)
		d[i] = 0;
	if (pdma32(md, d) != 0)
		ErrorRunTime("Unable to clear auxiliary outputs");
	if (dev.interruptSource != DEV_INTER_DISABLE) {
		md = PDMA_INTERRUPT_ENABLE;
		d[0] = dev.interruptSource;
		if (pdma32(md, d) != 0)
			ErrorRunTime("Unable to enable hardware interrupt");
		}
	if (dev.clockSource == DEV_CLOCK_TIMER)
		GenPrint("Clock rate set to %.15g Hz\n", dev.frequency);
	if (GenPerform(caseStartup))
		GenStates();
	GenEnd();
	}


/*
 * GenEnd turns off interrupts and cleans up the state of the PDMA-32 board.
 * Notice that the length is set to the minimum of 1.  Setting length to 0
 * is equivalent to setting it to 65536.
 */

static void GenEnd(void) {
	md = PDMA_INTERRUPT_DISABLE;	/* turn off ISR */
	pdma32(md, d);
	md = PDMA_SETUP_TRANSFER;
	d[0] = 1;			/* 1 bytes/words to transfer */
	d[2] = DEV_INPUT;		/* data direction */
	d[3] = DEV_AUTO_OFF;		/* auto-recycle off */
	d[4] = DEV_CLOCK_TIMER;		/* transfer clock source */
	d[5] = dev.buffSeg;
	d[6] = dev.buffOff;
	pdma32(md, d);
	md = PDMA_DIGITAL_OUTPUT;	/* turn off all outputs */
	d[0] = 0;
	d[1] = PDMA_PORTB;
	pdma32(md, d);
	md = PDMA_AUX_OUTPUT;		/* turn off all auxiliary ports */
	d[0] = d[1] = d[2] = 0;
	pdma32(md, d);
	}


/*
 * Searches through the state transition lists when stateChange occurs.
 * For any match, the appropriate action is performed and the process
 * repeats.
 */

static void GenStates(void) {
	case_state	*state;
	short		ch;
	Word		data, cmpVal;
	Link		*lk;
	Boolean		done;
	CKeyIn		keyboard;

	if (caseKey==NULL && caseState==NULL)
		return;
	while (true) {
		if (keyboard.Avail() && (state = caseKey)!=NULL) {
			for (done=false, ch=keyboard.Read(); !done && state!=NULL; state=state->next)
				if (state->u.ch == ch) {
					if (!GenPerform(state->action))
						return;
					done = true;
					}
			}
		else if ((state = caseState) != NULL) {
			md = PDMA_DIGITAL_INPUT;
			d[1] = PDMA_PORTB;
			if (dev.portMode==DEV_BYTE)
				d[2] = DEV_CONFIG_NO_CHANGE;
			else
				d[2] = DEV_CONFIG_CHANGE;
			pdma32(md, d);
			data = d[0];
			if (dev.portMode == DEV_WORD)
				data >>= BYTE_BIT;
			for (done=false; !done && state!=NULL; state=state->next) {
				lk = SymbolFind(state->u.id)->val;
				cmpVal = data & lk->u.state.andMask;
				if (cmpVal == lk->u.state.cmpMask) {
					if (!GenPerform(state->action))
						return;
					done = true;
					}
				}
			}
		}
	}


/*
 * GenPerform does all actions linked to the action list.  If the HALT
 * command, GenPerform returns false, otherwise true.
 */

static Boolean GenPerform(case_action *action) {
	CKeyIn	keyboard;
	CTimer	timer(30);

	for ( ; action!=NULL; action=action->next) {
		switch (action->type) {
			case A_ACTION:
				stateChange = false;
				GenPerformAction(action->u.id);
				while (!stateChange && !keyboard.Avail())
					;
				break;
			case A_BEEP:
				sound(action->u.val);
				timer.Reset();
				while (!timer.Expired())
					;
				nosound();
				break;
			case A_CLEAR:
				GenPerformClear(action->u.val);
				break;
			case A_ECHO:
				GenPerformEcho(action->u.id);
				break;
			case A_HALT:
				GenEnd();
				return (false);
			case A_SET:
				GenPerformSet(action->u.val);
				break;
			}
		}
	return (true);
	}


/*
 * GenPerformAction calls GenPatternMask to generates the action patterns
 * into the far memory used for D.M.A. transfer.  Once the desired patterns
 * are generated, the pdma32 driver is called to begin transfer.
 */

static void GenPerformAction(char *id) {
	GenPatternMask(id);
	md = PDMA_SETUP_TRANSFER;
	d[0] = dev.numTransfers;
	d[2] = dev.direction;
	d[3] = dev.autoRecycle;
	d[4] = dev.clockSource;
	d[5] = dev.buffSeg;
	d[6] = dev.buffOff;
	pdma32(md, d);
	}


/*
 * GenPerformClear clears the indicated auxiliary data bit and performs the
 * call to the pdma32 driver.
 */

static void GenPerformClear(Word val) {
	short	i;

	dev.aux[val] = 0;
	md = PDMA_AUX_OUTPUT;
	for (i=NUMELE(dev.aux)-1; i>=0; i--)
		d[i] = dev.aux[i];
	pdma32(md, d);
	}


/*
 * GenPatternMask generates the action patterns into the far memory
 * used for D.M.A. transfer.  If numTransfers==0, the PDMA-32 driver will
 * do 65536 transfers.  Note:  We are using a DWord to tally up the
 * number of blocks we have generated.  This allows us to determine if an
 * overflow occurs.  When we copy the value to dev.numTransfers, and
 * inplicit conversion occurs (32-bits -> 16-bits).  If the number of
 * transfers was 65536, it will be copied as 0 - just what we needed!
 */

static void GenPatternMask(char *id) {
	Byte	huge *bblock;
	Word	huge *wblock;
	Link	*lk;
	Byte	*mask;
	Word	numRepeats, wild[WORD_BIT], numWild, preVal, i;
	Word	postVal, j;
	DWord	numPat, numTrans=0L, numCount, bitPos;

	bblock = (Byte *)MK_FP(dev.buffSeg, dev.buffOff);
	wblock = (Word *)MK_FP(dev.buffSeg, dev.buffOff);
	for (lk=SymbolFind(id)->val; lk != NULL; lk=lk->next) {
		switch (lk->u.action.qualify) {
			case Q_NONE:
				numRepeats = 1;
				break;
			case Q_REPEAT:
				numRepeats = (Word)lk->u.action.val;
				break;
			case Q_TIME:
				if (dev.clockSource == DEV_CLOCK_EXTERNAL) {
					GenEnd();
					ErrorRunTime("%s TIME undefined when using external clock source", id);
					}
				if (lk->u.action.val < dev.unitTime) {
					GenPrint("Unit time of %.15g uS is indivisible (%s)\n", dev.unitTime, id);
					numRepeats = 1;
					}
				else {
					numRepeats = lk->u.action.val / dev.unitTime;
					if (numRepeats * dev.unitTime != lk->u.action.val)
						GenPrint("%s TIME specified not a multiple of unit time\n", id);
					}
				break;
			}
		mask = (Byte *)lk->u.action.mask;
		bitPos = 0x8000;
		for (preVal=0, numWild=0, i=0; i<WORD_BIT; i++, mask++) {
			switch (*mask) {
				case '1':
					preVal += (Word)bitPos;
					break;
				case '*':
					wild[numWild++] = (Word)bitPos;
					break;
				}
			bitPos >>= 1;
			}
		numPat = (numWild == 0) ? (1) : (1UL << numWild);
		if ((numTrans+=numPat*numRepeats) > PAGE) {
			GenEnd();
			ErrorRunTime(".ACTION '%s' overflows physical page boundary", id);
			}
		for (numCount=0; numCount<numPat; numCount++) {
			postVal = preVal;
			for (bitPos=numCount, j=0; bitPos && j<numWild; bitPos>>=1, j++)
				if (bitPos & 0x0001)
					postVal += wild[j];
			if (dev.portMode == DEV_BYTE)
				for (j=0; j<numRepeats; j++)
					*bblock++ = postVal;
			else
				for (j=0; j<numRepeats; j++)
					*wblock++ = postVal;
			}
		}
	dev.numTransfers = (Word)numTrans;
	}


static void GenPerformEcho(char *id) {
	Symbol	*sym;
	Link	*cur;

	sym = SymbolFind(id);
	for (cur=sym->val; cur!=NULL; cur=cur->next)
		GenPrint("%s\n", cur->u.text);
	}


/*
 * GenPerformSet sets the indicated auxiliary data bit and performs the
 * call to the pdma32 driver.
 */

static void GenPerformSet(Word val) {
	short	i;

	dev.aux[val] = 1;
	md = PDMA_AUX_OUTPUT;
	for (i=NUMELE(dev.aux)-1; i>=0; i--)
		d[i] = dev.aux[i];
	pdma32(md, d);
	}


static void GenPrint(char *format, ...) {
	va_list	args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	}


void pdma32_interrupt() {
	stateChange = true;
	}
