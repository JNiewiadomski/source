#include <string.h>

#include "menu\_menu.h"

#include <seashell\resource.h>

void MenuItemAppend(short id, char *item, char *meta, short help_id, short popup_id) {
	MenuItems	*mp;
	short		num, len;

	mp = MenuIDFind(id);
	if (mp->num >= MAX_ITEMS-1)
		return;
	num = ++(mp->num);
	len = strlen(item);
	mp->items[num] = (char *) r_malloc(len+1);
	strcpy(mp->items[num], item);
	mp->items[num+1] = NULL;
	mp->hot[num] = '\0';
	mp->equiv[num] = ' ';
	mp->check[num] = ' ';
	mp->popup[num] = popup_id;
	mp->help[num] = help_id;
	while (*meta)
		switch(*meta++) {
			case '!':
				mp->check[num] = CHECKMARK;
				break;
			case '/':
				mp->equiv[num] = *meta++;
				len++;
				break;
			case '(':
				mp->enable &= ~(ENAB_LSB << num);
				break;
			case '=':
				mp->hot[num] = *meta++;
				break;
			}
	if (mp->width < len+4)
		mp->width = len+4;
	mp->changed = True;
	}
