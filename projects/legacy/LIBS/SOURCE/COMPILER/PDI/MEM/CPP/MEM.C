#include <alloc.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\pdma-32\pdma32.h"

#define K		1024UL
#define PAGE		(64UL * K)
#define MEMORY_SIZE	(3UL * PAGE)
#define MEMORY_MIN	(2UL * PAGE)

static unsigned long table[] = {	0x0000, 0x1000, 0x2000, 0x3000,
					0x4000, 0x5000, 0x6000, 0x7000,
					0x8000, 0x9000, 0xA000, 0xB000,
					0xC000, 0xD000, 0xE000, 0xF000 };

main() {
	char 		huge *block, huge *end_block;
	char		first1, first2, last1, last2;
	unsigned long	i, j;
	short		k;
	int		md, d[7], err;

	md = 0;
	d[0] = 0x0300; /* base address */
	d[1] = 5; /* dma level */
	d[2] = 14; /* interrupt level */
	d[3] = 1; /* port mode */
	if ((err = pdma32(md, d)) != 0) {
		printf("Unable to access PDMA-32 hardware (err=%d).\n", err);
		exit(EXIT_FAILURE);
		}
	if ((block = farmalloc(MEMORY_SIZE)) == NULL) {
		printf("Unable to allocate %lu bytes of memory.\n", MEMORY_SIZE);
		exit(EXIT_FAILURE);
		}
	i = FP_SEG(block);
	j = i + (MEMORY_SIZE - 1)/16;
	end_block = MK_FP(j, 0);
	printf("Memory allocated from %p to %p.\n", block, end_block);

	for (k=0; i >= table[k]; k++)
		;
	i = table[k];
	j = 0;
	block = MK_FP(i, j);
	printf("i=0x%04X[%lu], j=0x%04X[%lu].\n", i, i, j, j);

	first1 = *block+1;
	*block = first1;
	first2 = *block;
	j = i + (MEMORY_MIN - 1)/16;
	end_block = MK_FP(j, 0);
	last1 = *end_block + 1;
	*end_block = last1;
	last2 = *end_block;
	if (first1 != first2 || last1 != last2) {
		printf("Cannot run on this machine.  Insufficient memory.\n");
		printf("From %p to %p\n", block, end_block);
		printf("first1=%u and first2=%u\n", first1, first2);
		printf("last1=%u and last2=%u\n", last1, last2);
		exit(EXIT_FAILURE);
		}
	printf("%lu bytes of memory begin at %p and end at %p.\n", MEMORY_MIN, block, end_block);
	return (EXIT_SUCCESS);
	}


void pdma32_interrupt(void) {
	}
