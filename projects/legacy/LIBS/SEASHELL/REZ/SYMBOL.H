#define	MAX_ID_LEN	32

typedef struct Symbol {
	char		name[MAX_ID_LEN+1];	// name of identifier
	short		val;			// numeric value of symbol
	struct Symbol	*next;			// free-chain linked list
	} Symbol;


void	SymbolAdd(Symbol *sym);
void	SymbolDiscard(Symbol *sym);
Symbol	*SymbolFind(char *name);
void	SymbolInit(void);
Symbol	*SymbolNew(char *name);
