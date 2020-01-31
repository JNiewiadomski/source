#ifndef _MENU_H
#define _MENU_H

// ****************
// * Menu Manager *
// ****************

#define LAST_MENU	-1

typedef void	(*MenuFunc)(short id, short item);

void	MenuBarInsert(short id, short beforeID=LAST_MENU);
void	MenuBarPush(short id);
void	MenuBarPop(void);
void	MenuBarRemove(short id);
void	MenuDispose(short id);
void	MenuErrorPost(char *eStr, Boolean goAway);
void	MenuInit(MenuFunc call_back);
Boolean	MenuIsAlt(short ch);
Boolean	MenuIsCtrl(short ch);
Boolean	MenuIsFunc(short ch);
void	MenuItemAppend(short id, char *item, char *meta, short help_id, short popup_id);
void	MenuItemCheck(short id, short item, Boolean checked);
short	MenuItemCount(short id);
void	MenuItemDelete(short id, short item);
void	MenuItemDisable(short id, short item);
void	MenuItemEnable(short id, short item);
char	*MenuItemGet(short id, short item);
char	MenuItemGetEquiv(short id, short item);
void	MenuItemSet(short id, short item, char *s);
void	MenuItemSetEquiv(short id, short item, char equiv);
Boolean	MenuKey(short ch);
void	MenuNew(short id, char *name, Boolean flash, short help_id);
void	MenuNewGet(short id);
void	MenuPopup(short id);
void	MenuSelect(short keyCode);

#endif
