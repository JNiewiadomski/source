#include <ctype.h>

#include "menu\_menu.h"

void MenuItemMakeStr(char *bptr, char check, char *item, char hot, char equiv, short popup, short width) {
	short	i;
	char	ch;
	Boolean	found;

	if (*item == '-')
		for (i=0; i<width; i++)
			*bptr++ = HBAR1;
	else {
		*bptr++ = check;
		*bptr++ = ' ';
		found = False;
		for (i=2; *item; i++) {
			ch = *item++;
			if (ch==hot && !found) {
				*bptr++ = '{';
				*bptr++ = ch;
				*bptr++ = '}';
				found = True;
				}
			else
				*bptr++ = ch;
			}
		for ( ; i<width-2; i++)
			*bptr++ = ' ';
		if (equiv == ' ') {
			*bptr++ = ' ';
			*bptr++ = popup ? POPUP_SYMBOL : ' ';
			}
		else {
			*bptr++ = isdigit(equiv) ? 'F' : '^';
			*bptr++ = equiv;
			}
		}
	*bptr = '\0';
	}
