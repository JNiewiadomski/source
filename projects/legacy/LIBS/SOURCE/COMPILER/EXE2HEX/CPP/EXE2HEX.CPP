#include <stdio.h>

#include <common\common.h>

enum { EXIT_OK, EXIT_PARAM, EXIT_FILE };

main(int argc, char *argv[]) {
	Word	addr;
	int	t, bsize=16;
	FILE	*fptr;
	Byte	x, cnt, cks, buffer[128];

	if (argc < 3) {
		puts("Syntax: EXE2HEX fn Haddr [BlkSize]");
		return(EXIT_PARAM);
		}
	if (argc == 4)
		sscanf(argv[3],"%i",&bsize);
	sscanf(argv[2],"%05x",&addr);
	if ((fptr = fopen(argv[1],"rb")) == NULL)
		return(EXIT_FILE);
	fread(buffer,sizeof(char),128,fptr); /* get first record */
	t = *(int *)(buffer+10); /* number of paragraphs */
	t <<= 4;
	t += 511;
	t &= 511;

	fseek(fptr, (long) (t + 1), SEEK_SET);
	while (!feof(fptr)) {
		cks = cnt = fread(buffer,sizeof(char),bsize,fptr);
		if (cnt == 0) break;
		printf(":%02X%04X00",cnt,addr);
		cks += (addr >> 8);
		cks += (addr & 255);
		for (x = 0; x != cnt; x += 1) {
			printf("%02X",buffer[x]);
			cks += buffer[x];
			}
		printf("%02X\n",(-cks) & 255);
		addr += cnt;
		}
	fclose(fptr);
	return(EXIT_OK);
	}