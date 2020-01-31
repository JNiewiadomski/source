#include <string.h>

#include <common\common.h>

#include "hash.h"
#include "symbol.h"

static Symbol	*Symbol_free=NULL;	// free-list of recycled symbols
static HASH_TAB	*Symbol_tab;		// the actual table


static short SymbolCompare(void *s1, void *s2);


// Adds sym to the Hash table.

void SymbolAdd(Symbol *sym) {
	HashAdd(Symbol_tab, sym);
	}


// Called by HashFind() and HashNext() when looking for a specific bucket.

static short SymbolCompare(void *s1, void *s2) {
	return (strcmp(((Symbol *)s1)->name, ((Symbol *)s2)->name));
	}


// Discard a single Symbol record.  Link the freed Symbol onto the
// free-list.

void SymbolDiscard(Symbol *sym) {
	if (!sym)
		return;
	sym->next = Symbol_free;
	Symbol_free = sym;
	}


// Returns a pointer to the symbol if it was found in the table, NULL
// otherwise.

Symbol *SymbolFind(char *name) {
	return ((Symbol *)HashFind(Symbol_tab, name));
	}


// Initializes the symbol table data structure.

void SymbolInit(void) {
	Symbol_tab = HashMake(127, SymbolCompare);
	}


// Check the free-list to see if there are any reuseable Symbol records.
// If there is, unlink it and use it.  Otherwise allocate room for a new
// one.  Copy the name into the new symbol, initialize the data structure,
// and return a pointer to it.  The new symbol structure is added to the
// hash table.

Symbol *SymbolNew(char *name) {
	Symbol	*sym_p;

	if (!Symbol_free)
		sym_p = (Symbol *) HashNew(sizeof(Symbol));
	else {
		sym_p = Symbol_free;
		Symbol_free = Symbol_free->next;
		}
	strncpy(sym_p->name, name, sizeof(sym_p->name));
	sym_p->val = 0;
	return (sym_p);
	}
