// *********************
// * General Functions *
// *********************

// The GENERAL module provides reoutines to perform common low-level tasks.

#include <common\old\general.h>


// repmem() replicate values through memory
// 	dstp	- pointer to destination block
// 	valp	- pointer to values
// 	num_val	- number of bytes of values
// 	num_rep	- number of replications (num_val * num_rep bytes will be moved)

void repmem(char *dstp, const char *valp, int num_val, int num_rep) {
	register int	i;

	while (num_rep-- > 0)
		for (i=0; i<num_val; i++)
			*dstp++ = valp[i];
	}


// repstr() builds a string from repeated character
//
//	dst - destination string
//	ch  - character to repeat
//	num - number of repetitions

void repstr(char *dst, char ch, int num) {
	while (num-- > 0)
		*dst++ = ch;
	*dst = '\0';
	}


// trim -- removes all leading and trailing blanks and returns a pointer
// to the beginning of the string.

char *trim(char *s) {
	char	*f, *m;

	f = m = s;
	while (*m == ' ')
		m++;
	while (*m)
		*f++ = *m++;
	while (m>s && *(m-1)==' ')
		m--;
	*m = '\0';
	return (s);
	}
