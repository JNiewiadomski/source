typedef struct BUCKET {
	struct BUCKET	*next;
	struct BUCKET	**prev;
	} BUCKET;

typedef short	(*HashCmpFunc)(void *b1, void *b2);
typedef void	(*HashPrintFunc)(BUCKET *outp, void *param);

typedef struct {
	short		size;		// max number of elements in table
	short		numSyms;	// number of elements currently in table
	HashCmpFunc	cmp;		// comparison function
	BUCKET		*table[1];	// first element of actual hash table
	} HASH_TAB;

void		*HashAdd(HASH_TAB *tabp, void *isym);
void		HashDelete(HASH_TAB *tabp, void *isym);
void		*HashFind(HASH_TAB *tabp, void *sym);
void		HashFree(void *sym);
HASH_TAB	*HashMake(Word maxsym, HashCmpFunc cmp);
void		*HashNew(short size);
void		*HashNext(HASH_TAB *tabp, void *i_last);
void		HashPrint(HASH_TAB *tabp, HashPrintFunc print, void *param);
