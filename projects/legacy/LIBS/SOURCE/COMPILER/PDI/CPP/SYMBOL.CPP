#include <stdlib.h>
#include <string.h>

#include <cbase.h>

#include "h\error.h"
#include "h\hash.h"
#include "h\symbol.h"

#define L_CHUNK	10			/* SymbolLinkNew() gets this many at one shot */

static Symbol	*Symbol_free=NULL;	/* free-list of recycled symbols */
static Link	*Link_free=NULL;	/* free-list of recycled links */
static HASH_TAB	*Symbol_tab;		/* the actual table */


static short SymbolCompare(void *s1, void *s2);


/*
 * Adds sym to the Hash table.
 */

void SymbolAdd(Symbol *sym) {
	HashAdd(Symbol_tab, sym);
	}


/*
 * Add a link to the end of the symbol list.  If sym->val is NULL, there is
 * no list, so p is made the first link.
 */

void SymbolAddLink(Symbol *sym, Link *p) {
	if (sym->val == NULL)
		sym->val = p;
	else
		sym->eval->next = p;
	p->next = NULL;
	sym->eval = p;
	}


/*
 * Called by HashFind() and HashNext() when looking for a specific bucket.
 */

static short SymbolCompare(void *s1, void *s2) {
	return (strcmp(((Symbol *)s1)->name, ((Symbol *)s2)->name));
	}


/*
 * Discard a single Symbol record and any attached links.  Link the freed
 * Symbol onto the free-list.
 */

void SymbolDiscard(Symbol *sym) {
	if (!sym)
		return;
	SymbolDiscardLinkChain(sym);
	sym->next = Symbol_free;
	Symbol_free = sym;
	}


/*
 * Discard all links in the chain.  If the type is T_TEXT, the allocated
 * memory is freed.  Since strings are of different lengths, there is no
 * point in trying to re-use them.  There's no point in discarding the nodes
 * one at a time since they're already linked together.  So find the first
 * and last nodes in the input chain and link the whole list directly.
 */

void SymbolDiscardLinkChain(Symbol *sym) {
	T_TYPE	type;
	Link	*p;

	if (sym == NULL || (p = sym->val) == NULL)
		return;
	if ((type = sym->type) == T_TEXT)
		free(p->u.text);
	while (p->next) {
		p = p->next;
		if (type == T_TEXT)
			free(p->u.text);
		}
	p->next = Link_free;
	Link_free = sym->val;
	sym->val = sym->eval = NULL;
	}


/*
 * Returns a pointer to the symbol if it was found in the table, NULL
 * otherwise.
 */

Symbol *SymbolFind(char *name) {
	return ((Symbol *)HashFind(Symbol_tab, name));
	}


/*
 * Initializes the symbol table data structure.
 */

void SymbolInit(void) {
	Symbol_tab = HashMake(127, SymbolCompare);
	}


/*
 * Check the free-list to see if there are any reuseable Symbol records.
 * If there is, unlink it and use it.  Otherwise allocate room for a new
 * one.  Copy the name into the new symbol, initialize the data structure,
 * and return a pointer to it.  The new symbol structure is added to the
 * hash table.
 */

Symbol *SymbolNew(char *name, T_TYPE type) {
	Symbol	*sym_p;

	if (!Symbol_free)
		sym_p = (Symbol *) HashNew(sizeof(Symbol));
	else {
		sym_p = Symbol_free;
		Symbol_free = Symbol_free->next;
		}
	strncpy(sym_p->name, name, sizeof(sym_p->name));
	sym_p->type = type;
	sym_p->val = sym_p->eval = NULL;
	return (sym_p);
	}


/*
 * Return a new link.  L_CHUNK nodes are allocated from malloc at one time.
 */

Link *SymbolNewLink(void) {
	Link	*p;
	short	i;

	if (!Link_free) {
		if ((Link_free = (Link *) malloc(sizeof(Link) * L_CHUNK)) == NULL)
			e_OutOfMemory("SymbolLinkNew");
		for (p=Link_free, i=L_CHUNK; --i > 0; ++p)
			p->next = p + 1;
		p->next = NULL;
		}
	p = Link_free;
	Link_free = Link_free->next;
	return (p);
	}
