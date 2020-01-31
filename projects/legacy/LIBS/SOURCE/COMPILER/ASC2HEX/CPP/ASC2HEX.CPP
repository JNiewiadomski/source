#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

main(int, char *argv[]) {
	unsigned int	addr;
	int		cnt, x, byte;
	unsigned char	cks;
	char		buffer[40];

	if (argv[1] == NULL) addr = 0;
	else sscanf(argv[1],"%05x",&addr);

	for (;;) {
		cnt = 0;	
		while (cnt < 32) {
			if ((buffer[cnt] = getchar()) == EOF) break;
			if (!isxdigit(buffer[cnt])) continue;
			cnt += 1;
			}
		if (cnt == 0) break;
		if (cnt & 1) exit(1);
		buffer[cnt] = '\0';
		cks = cnt >> 1;
		cks += (addr >> 8);
		cks += (addr & 255);
		for (x = 0; x < cnt; x += 2) {
			sscanf(&buffer[x],"%02x",&byte);
			cks += byte;
			}
		cks = -cks;
		printf(":%02X%04X00%s%02X\n", cnt>>1, addr, buffer, cks);
		addr += (cnt >> 1);
		}
	printf("%s\n",":00000001FF");
	return (0);
	}