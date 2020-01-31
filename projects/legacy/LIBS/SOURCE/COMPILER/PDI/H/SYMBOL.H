#define	MAX_ID_LEN	32

#define BYTE_BIT	8
#define WORD_BIT	16

typedef enum { T_ACTION, T_STATE, T_TEXT } T_TYPE;
typedef enum { Q_NONE, Q_REPEAT, Q_TIME } Q_TYPE;

struct act_rec {
	char	mask[WORD_BIT+1];
	Q_TYPE	qualify;
	long	val;
	};

struct st_rec {
	Word	andMask;
	Word	cmpMask;
	};

typedef struct Link {
	union {
		struct act_rec	action;
		struct st_rec	state;
		char		*text;
		} u;
	struct Link		*next;
	} Link;

typedef struct Symbol {
	char		name[MAX_ID_LEN+1];	/* name of identifier */
	T_TYPE		type;			/* type of identifier */
	Link		*val;			/* first value in chain */
	Link		*eval;			/* last value in chain */
	struct Symbol	*next;			/* free-chain linked list */
	} Symbol;

#define IS_ACTION(p)	((p)->type == T_ACTION)
#define IS_STATE(p)	((p)->type == T_STATE)
#define IS_TEXT(p)	((p)->type == T_TEXT)


void	SymbolAdd(Symbol *sym);
void	SymbolAddLink(Symbol *sym, Link *p);
void	SymbolDiscard(Symbol *sym);
void	SymbolDiscardLinkChain(Symbol *sym);
Symbol	*SymbolFind(char *name);
void	SymbolInit(void);
Symbol	*SymbolNew(char *name, T_TYPE type);
Link	*SymbolNewLink(void);
