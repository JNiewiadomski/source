#include <conio.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void speed(long desired);

main(int argc, char **argv) {
	long	desired;

	if (argc != 2) {
		printf("Syntax: CLOCK <speed>\n");
		exit(EXIT_FAILURE);
		}
	desired = atol(argv[1]);
	if (desired < 1) {
		printf("Invalid speed of %ld selected\n", desired);
		exit(EXIT_FAILURE);
		}
	printf("Desired speed = %ld\n", desired);
	speed(desired);
	return (EXIT_SUCCESS);
	}


#define XTAL_SPEED	10000000.0

void speed(long desired) {
	double		xtal, e, new, partial;
	long		limit, i, j;
	unsigned int	d0=0, d1=0;

	xtal = XTAL_SPEED;
	limit = sqrt(xtal/desired);
	if (limit < 2 || limit > UINT_MAX)
		printf("Error: Transfer rate of %ld cannot be achieved\n", desired);
	for (e=1E+10, i=limit; i>=2; i--) {
		partial = xtal / i;
		j = (partial / desired) + .5;
		if (j > UINT_MAX)
			i = 1;
		else {
			new = fabs(desired - (partial / j));
			if (e > new) {
				e = new;
				d0 = (unsigned int)i;
				d1 = (unsigned int)j;
				}
			}
		}
	printf("Counter 0 = %u and Counter 1 = %u\n", d0, d1);
	printf("Actual speed = %.15g\n", xtal / ((double)d0 * (double)d1));
	}
