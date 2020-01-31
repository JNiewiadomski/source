// The Resource module provides routines to access the resources located in
// a resource file.  All text should be defined as resources inside this
// resource file to provide for a convenient way to localize the running
// program.

#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\old\coord.h>
#include <common\old\system.h>

#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\error.h>

#define RES_CLOSED	-1
#define RES_AVAIL	(fResHand != RES_CLOSED)

static int	fResHand=RES_CLOSED;	// handle to resource file
static ResTable	*res_tab=NULL;		// resource directory table
static Word	res_num=0;		// number of resources in table
static Boolean	res_mem_exit=False;	// emergency exit due to memory shortage

static void	ResCleanUp(void);
static int	ResCompare(const void *p1, const void *p2);
static void	ResError(char *format, ...);

static void	res_mem_display(void);


// ResAvail() returns true is the resource manager has been initialized.

Boolean ResAvail(void) {
	return ((Boolean)(RES_AVAIL));
	}


// ResCleanUp() frees all allocated memory and closes the resource file.
// This function is registered by ResInit() and is invoked automatically
// before returning to the operating system.

static void ResCleanUp(void) {
	ResTable	*res_pos;
	Word		num;

	if (res_tab) {				// release all memory
		for (res_pos=res_tab, num=res_num; num; res_pos++, num--)
			if (res_pos->block)	
				r_free(res_pos->block);
		r_free(res_tab);
		res_tab = NULL;
		}
	close(fResHand);			// release file handle
	fResHand = RES_CLOSED;			// mark handle as unused
	if (res_mem_exit)
		fprintf(stderr, "Resource Manager Out Of Memory\n");
	res_mem_display();
	}


// ResCompare() compares the type_id's of the ResTable entries pointed to by
// p1 and p2.  If p1 < p2, a negative value is returned.  If p1 == p2, 0 is
// returned.  If p1 > p2, a positive value is returned.

static int ResCompare(const void *p1, const void *p2) {
	long	val;

	val = ((ResTable *)p1)->type_id - ((ResTable *)p2)->type_id;
	if (val < 0)
		return (-1);
	else if (val > 0)
		return (1);
	return (0);
	}


// ResError() directs an error message to stderr and terminates program
// execution.

static void ResError(char *format, ...) {
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(EXIT_FAILURE);
	}


// ResInit() opens up a path to the programs resource file and initializes
// ResHeader to point to the header of each individual resource label found.
// All resources marked as preload are loaded in as well.

void ResInit(char *format, ...) {
	va_list		args;
	char		res_name[MAXPATH], ch, sig[sizeof(RES_SIG)];
	long		loc;
	Word		size, num;
	ResTable	*res_pos;

	if (RES_AVAIL)				// if initialized, ignore
		return;
	va_start(args, format);
	vsprintf(res_name, format, args);
	va_end(args);
	if ((fResHand = open(res_name, O_RDONLY | O_BINARY)) == RES_CLOSED)
		ResError("Unable to open resource file '%s'\n", res_name);
	atexit(ResCleanUp);			// register cleanup function
	//
	// read past resource file header
	//
	while (read(fResHand, &ch, sizeof(ch)) == sizeof(ch) && ch != A_CTRLZ)
		;
	//
	// check signiature
	//
	read(fResHand, sig, sizeof(RES_SIG));
	if (memcmp(RES_SIG, sig, sizeof(RES_SIG)) != 0)
		ResError("Invalid signiature in resource file '%s'\n", res_name);
	//
	// read in directory table
	//
	read(fResHand, &loc, sizeof(loc));
	lseek(fResHand, loc, SEEK_SET);
	read(fResHand, &res_num, sizeof(res_num));
	if (res_num < 1)
		ResError("Resource file '%s' is empty\n", res_name);
	size = res_num * sizeof(ResTable);
	if ((res_tab = (ResTable *) r_malloc(size)) == NULL)
		ResError("Unable to allocate memory for resource table\n");
	//
	// Read in all resources marked as preload
	//
	read(fResHand, res_tab, size);
	for (res_pos=res_tab, num=res_num; num; res_pos++, num--)
		if (res_pos->preload) {
			lseek(fResHand, res_pos->loc, SEEK_SET);
			read(fResHand, &size, sizeof(size));
			if ((res_pos->block = r_malloc(size)) != NULL)
				read(fResHand, res_pos->block, size);
			}
	}


// ResLoad() searches for the resource specified by type:id.  If the resource
// does not exist, a warning is issued and NULL is returned.  If the resource
// exists but needs to be read in, a block of memory is allocated to hold
// the resource and it is read in.  If no memory can be allocated NULL is
// returned.  Otherwise a pointer to the block of memory is returned.

void *ResLoad(ResType type, short id) {
	ResTable	key, *res_pos;
	Word		size;

	if (!RES_AVAIL)
		return (NULL);
	key.type_id = ShortShort(type, id);
	if ((res_pos = (ResTable *) bsearch(&key, res_tab, res_num, sizeof(ResTable), ResCompare)) == NULL) {
		Error(Err_Warning, "Referencing undefined resource (type=%d, id=%d).", type, id);
		return (NULL);
		}
	if (!res_pos->block) {
		lseek(fResHand, res_pos->loc, SEEK_SET);
		read(fResHand, &size, sizeof(size));
		if ((res_pos->block = r_malloc(size)) != NULL)
			read(fResHand, res_pos->block, size);
		}
	return (res_pos->block);
	};


// ResSetPurge() makes the resource of type:id purgeable or unpurgeable
// based on purgeable.

void ResSetPurge(ResType type, short id, Boolean purgeable) {
	ResTable	key, *res_pos;

	if (!RES_AVAIL)
		return;
	key.type_id = ShortShort(type, id);
	if ((res_pos = (ResTable *) bsearch(&key, res_tab, res_num, sizeof(ResTable), ResCompare)) == NULL) {
		Error(Err_Warning, "Referencing undefined resource (type=%d, id=%d).", type, id);
		return;
		}
	res_pos->purgeable = purgeable;
	};


// ResUnLoad() searches for the resource specified by type:id.  If the
// resource exists and the resource is in memory, the memory is freed and the
// pointer marked as such.

void ResUnLoad(ResType type, short id) {
	ResTable	key, *res_pos;

	if (!RES_AVAIL)
		return;
	key.type_id = ShortShort(type, id);
	if ((res_pos = (ResTable *) bsearch(&key, res_tab, res_num, sizeof(ResTable), ResCompare)) == NULL)
		return;
	if (res_pos->block) {
		r_free(res_pos->block);
		res_pos->block = NULL;
		}
	};


// Memory management utilities
//
// This portion of the Resource manager contains routines to protect the
// programmer from errors in calling memory allocation/free routines.  The
// standard library calls supported are:
//		free
//		malloc
//
// The interface header redefines these standard library calls as macros.
// When the client code is compiled, the macros expand to calls to this
// module.  This module then calls the system memory routines, with
// additional error checking.
//
// The allocation routines in this module add a data structure to the top of
// allocated memory blocks which tag them as legal memory blocks.
//
// When the free routine is called, the memory block to be freed is checked
// for legality.  If the block is not legal, the memory list is dumped to
// stderr and the program is terminated.

#define MEMTAG	0xA55A			// value for tag

struct MemHdr {
	Word		tag;		// special identification tag
	size_t		size;		// size of allocation block
	char		*file;		// allocation source file name
	Word		line;		// allocation source line number
	struct MemHdr	*next;		// next memory block
	struct MemHdr	*prev;		// previous memory block
	};

// Alignment macros
// The macro ALIGN_SIZE defines the size of the largest object that must be
// aligned on your processor.  The RESERVE_SIZE macro calculates the nearest
// multiple of ALIGN_SIZE that is larger than the memory block header.
#define ALIGN_SIZE	sizeof(double)
#define HDR_SIZE	sizeof(MemHdr)
#define RESERVE_SIZE	(((HDR_SIZE+(ALIGN_SIZE-1))/ALIGN_SIZE)*ALIGN_SIZE)

// Conversion macros
// These macros convert the internal pointer to a memory block header to/from
// the pointer used by the client code.
#define CLIENT_2_HDR(block)	((MemHdr *)(((char *)(block))-RESERVE_SIZE))
#define HDR_2_CLIENT(block)	((void *)(((char *)(block))+RESERVE_SIZE))

// Local variables
static DWord	mem_size=0;		// amount of memory used
static Word	mem_num=0;		// number of outstanding memory allocs
static MemHdr	*mem_list=NULL;		// list of memory blocks


static void	mem_list_add(MemHdr *block);
static void	mem_list_delete(MemHdr *block);


// res_free() deallocates a memory block allocated by a previous call to
// res_malloc.

void res_free(void *block, char *file, Word line) {
	MemHdr	*b;

	b = CLIENT_2_HDR(block);
	if (b->tag != MEMTAG) {
		Error(Err_Warning, "Memory tag: %p, %s(%d)", b, file, line);
		return;
		}
	b->tag = ~MEMTAG;
	mem_size -= b->size;
	mem_list_delete(b);
	free(b);
	}


// mem_list_add links in the passed block to the memory list

static void mem_list_add(MemHdr *block) {
	block->next = mem_list;
	block->prev = NULL;
	if (mem_list != NULL)
		mem_list->prev = block;
	mem_list = block;
	mem_num++;
	}


// res_list_delete unlinks the passed block from the memory list

static void mem_list_delete(MemHdr *block) {
	if (block->next != NULL)
		block->next->prev = block->prev;
	if (block->prev != NULL)
		block->prev->next = block->next;
	else
		mem_list = block->next;
	mem_num--;
	}


// res_alloc() attempts to allocate size bytes of contiguous dynamic memory.
// On success it returns a pointer to the block of memory.  If it fails, it
// attempts to free as much memory of purgeable resources as needed to
// allocate the requested amount.

void *res_alloc(Word size, char *file, Word line) {
	MemHdr		*block;
	ResTable	*res_pos;
	Word		num;

	if ((block = (MemHdr *) malloc(RESERVE_SIZE+size)) == NULL) {
		if (!RES_AVAIL) {
			res_mem_exit = True;
			exit(EXIT_FAILURE);
			}
		for (res_pos=res_tab, num=res_num; !block && num; res_pos++, num--)
			if (res_pos->purgeable && res_pos->block) {
				r_free(res_pos->block);
				res_pos->block = NULL;
				block = (MemHdr *) malloc(RESERVE_SIZE+size);
				}
		if (!block) {
			res_mem_exit = True;
			exit(EXIT_FAILURE);
			}
		}
	block->tag = MEMTAG;
	block->size = size;
	mem_size += size;
	block->file = file;
	block->line = line;
	mem_list_add(block);
	return (HDR_2_CLIENT(block));
	};


static void res_mem_display(void) {
	MemHdr	*block;
	char	*status;

	if (mem_size == 0 && mem_num == 0)
		return;
	block = mem_list;
	fprintf(stdout, "\nThe following memory has not been freed:\n\n");
	fprintf(stdout, "Status   Address        Size    Line  File\n");
	fprintf(stdout, "--------------------------------------------------\n");
	while (block != NULL) {
		status = block->tag == MEMTAG ? "Valid  " : "Invalid";
		fprintf(stdout, "%s  %p  %8u  %6u  %s\n", status, block, block->size, block->line, block->file);
		mem_size -= block->size;
		mem_num--;
		if (block->tag != MEMTAG)
			break;
		block = block->next;
		}
	fprintf(stdout, "--------------------------------------------------\n");
	fprintf(stdout, "                    %8lu  %6u\n", mem_size, mem_num);
	}
