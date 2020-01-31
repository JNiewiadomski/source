#ifndef _GENERAL_H
#define _GENERAL_H

// ****************************
// * General Purpose Routines *
// ****************************

void	repmem(char *dstp, const char *valp, int num_val, int num_rep);
void	repstr(char *dstp, char ch, int num);
char	*trim(char *s);

#endif
