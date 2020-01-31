#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include <cbase.h>

#include "h\pdma32.h"

static void	ErrorAbort(char *info);

main(int argc, char **argv) {
	char	ch;
	int	md, d[7];
	Boolean	wordWidth=true;

	if (argc>1 && ((ch=*(argv[1]))=='b' || ch=='B'))
		wordWidth = false;
	md = 0;				/* initialize */
	d[0] = 0x0300;			/* base i/o address */
	d[1] = (wordWidth) ? 5 : 1;	/* d.m.a. level */
	d[2] = 5;			/* interrupt level */
	d[3] = (wordWidth) ? 1 : 0;	/* byte=0/word=1 mode */
	if (pdma32(md, d) != 0)
		ErrorAbort("Unable to initialize PDMA-32 hardware");
	printf("%s wide input.\n", wordWidth ? "Word" : "Byte");
	while (!kbhit()) {
		md = 5;			/* digital input */
		d[1] = 1;		/* port b */
		d[2] = 1;		/* change configuration */
		if (pdma32(md, d) != 0)
			ErrorAbort("Input was not successful");
		printf("PDMA-32 Digital Input = 0x%04X\r", d[0]);
		}
	getch();
	putch('\n');
	return (EXIT_SUCCESS);
	}


static void ErrorAbort(char *info) {
	printf("%s.\n", info);
	exit(EXIT_FAILURE);
	}
