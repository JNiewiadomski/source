#include <limits.h>
#include <stdlib.h>

#include <cbase.h>

#include "h\error.h"
#include "h\hash.h"


/*
 * If you modify these constants, don't convert the floating-point numbers
 * to fractions, or else they'll end up as zeros.  For example,
 * (UINT_MAX * (1/8)) won't work because the 1/8 will evaluate to 0 (integer
 * arithmetic is used).
 */

#define SEVENTY_FIVE_PERCENT	((short)(UINT_MAX * .75))
#define TWELVE_PERCENT		((short)(UINT_MAX * .125))
#define HIGH_BITS		(~((Word)(~0) >> TWELVE_PERCENT))

static Word Hash(Byte *name);


/*
 * The algorithm uses a shift-and-XOR strategy to randomize the input key.
 * The main iteration of the loop shifts the accumulated hash value to the
 * left by a few bits and adds in the current character.  When the number
 * gets too large, it is randomized by XORing it with a shifted version of
 * itself.
 */

static Word Hash(Byte *name) {
	Word	h=0;			/* hash value */
	Word	g;

	for ( ; *name; ++name) {
		h = (h << TWELVE_PERCENT) + *name;
		if ((g = h & HIGH_BITS) > 0)
			h = (h ^ (g >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
		}
	return (h);
	}


/*
 * Add a symbol to the hash table.  A pointer to the current hash-table
 * element, p, is initialized by calling the hash function indirectly
 * through the hash pointer in the HASH_TAB header.  Note that the BUCKET
 * pointer (sym) comes into the routine pointing at the user area.  It is
 * decremented to point at the BUCKET header after the hash function is
 * called.  The new node is linked to the head of a chain found at the
 * previously computed array element.
 *
 * Note that the chain of BUCKETs is a doubly-linked list.  You need the
 * backwards pointer to delete an arbitrary element in the table without
 * having to search for that element.  The only obscure point is the two
 * stars in the definition of the backwards pointer -- the forward pointer
 * (next) is a pointer to a BUCKET, but the backwards pointer (prev) is a
 * pointer to a BUCKET pointer.  You need this extra level of indirection
 * because the head of the chain is a simple pointer, not an entire BUCKET
 * structure.  The backwards pointer for the leftmost node in the chain
 * points at the head-of-chain pointer.  All other backwards pointers hold
 * the address of the next field from the previous node.
 */

void *HashAdd(HASH_TAB *tabp, void *isym) {
	BUCKET	**p, *tmp;
	BUCKET	*sym = (BUCKET *) isym;

	p = &(tabp->table)[Hash((Byte *)sym--) % tabp->size];
	tmp = *p;
	*p = sym;
	sym->prev = p;
	sym->next = tmp;
	if (tmp)
		tmp->prev = &sym->next;
	tabp->numSyms++;
	return ((void *)(sym + 1));
	}


/*
 * Remove a symbol from the hash table.  "sym" is a pointer returned from a
 * previous HashFind() call.  It points initially at the user space, but is
 * decremented to get at the BUCKET header.  The pointer from the previois
 * node (the one that points at the node to delete) is modified first so
 * that it points around the deleted node.  Then the backwards pointer from
 * the next node is adjusted.  The if is required because the next pointer
 * is NULL on the last node in the chain.  The double indirection on the
 * backwards pointer makes this code work regardless of the position of the
 * node in the chain -- the first and last nodes are not special cases.
 */

void HashDelete(HASH_TAB *tabp, void *isym) {
	BUCKET	*sym = (BUCKET *) isym;

	if (tabp && sym) {
		--tabp->numSyms;
		--sym;
		if ((*(sym->prev) = sym->next) != NULL)
			sym->next->prev = sym->prev;
		}
	}


/*
 * Return a pointer to the hash table element having a particular name or
 * NULL if the name isn't in the table.
 */

void *HashFind(HASH_TAB *tabp, void *sym) {
	BUCKET	*p;

	//
	// table empty
	//
	if (!tabp)
		return (NULL);
	p = (tabp->table)[Hash((Byte *)sym) % tabp->size];
	while (p && (*tabp->cmp)(sym, p+1))
		p = p->next;
	return ((void *)(p ? p+1 : NULL));
	}


/*
 * Free the memory allocated by a previous call to HashNew().  The sym
 * pointer is backed up to its original position and then free'd.
 */

void HashFree(void *sym) {
	free ((BUCKET *)(sym) - 1);
	}


/*
 * Make a hash table of the indicated size.  We allocate a single chunck of
 * memory big enough to hold both the header and an additional area that will
 * be used as the array.  The table is declared as a one-element array.  I'm
 * taking advantage of the fact that C doesn't do array-boundary checking
 * when the array is accessed.  It's a good idea to make maxsym a prime
 * number.  Some useful sizes are: 47, 61, 89, 113, 127, 157, 193, 211, 257,
 * 293, 337, 367, 401.  If maxsym is zero, 127 is used.
 */

HASH_TAB *HashMake(Word maxsym, HashCmpFunc cmp) {
	HASH_TAB	*ht;

	if (!maxsym)
		maxsym = 127;
	/*
	 *			      |<--- space for bucket --->|<-- and header -->|
	 */
	if ((ht = (HASH_TAB *) calloc(1, (maxsym * sizeof(BUCKET *)) + sizeof(HASH_TAB))) == NULL)
		e_OutOfMemory("HashMake");
	ht->size	= maxsym;
	ht->numSyms	= 0;
	ht->cmp		= cmp;
	return (ht);
	}


/*
 * Allocate space for a new symbol; return a pointer to the user space.
 */

void *HashNew(short size) {
	BUCKET	*sym;

	if ((sym = (BUCKET *) calloc(1, size + sizeof(BUCKET))) == NULL)
		e_OutOfMemory("HashNew");
	/*
	 * return pointer to user space
	 */
	return ((void *)(sym + 1));
	}


/*
 * Return a pointer to the next node in the current chain that has the same
 * key as the last node found (or NULL if there is no such node).  "last" is
 * a pointer returned from a previous HashFind() or HashSymbolNext() call.
 */

void *HashNext(HASH_TAB *tabp, void *ilast) {
	BUCKET	*last = (BUCKET *) ilast;

	for (--last; last->next; last=last->next)
		/*
		 * keys match
		 */
		if ((tabp->cmp)(last+1, last->next+1) == 0)
			return ((char *)(last->next + 1));
	return (NULL);
	}


/*
 * The print function is called with two arguments:
 *
 *		(*print)(sym, param)
 *
 * Sym is a pointer to a BUCKET user area and param is the third argument to
 * HashPrint().
 */

void HashPrint(HASH_TAB *tabp, HashPrintFunc print, void *param) {
	BUCKET	*sym, **symtab;
	short	i;

	if (!tabp || tabp->size == 0)
		return;
	for (symtab=tabp->table, i=tabp->size; --i >= 0; symtab++) {
		/*
		 * Print all symbols in the current chain.  The +1 in the
		 * print call increments the pointer to the applications
		 * area of the bucket.
		 */
		for (sym=*symtab; sym; sym=sym->next)
			(*print)(sym+1, param);
		}
	}
