// ****************
// * Menu Manager *
// ****************

// The Menu Manager takes care of the management of all menus, allowing the
// user to open different menus and selecting any choices within the menus.
// If a menu item has an associated popup menu, that popup menu will be
// automatically handled as well.  The caller regains control when the user
// either closes all menus or makes a selection from the menu.

#include <limits.h>

#include <common\common.h>
#include <common\old\coord.h>

#include <seashell\menu.h>

typedef DWord		ENAB_TYPE;

#define MAX_ITEMS	(sizeof(ENAB_TYPE) * CHAR_BIT)
#define MAX_LIST	100
#define MAX_MENUS	20
#define MAX_BAR_LEN	80
#define NO_MENU		0
#define NO_ID		0
#define WMBAR		WNONE
#define CHECKMARK	0xFB
#define MENU_STR	"QWERTYUIOP    ASDFGHJKL     ZXCVBNM"
#define HBAR1		0xC4
#define POINTRIGHT	0x10
#define POPUP_SYMBOL	POINTRIGHT
#define WMENU		W1111
#define MAX_ERRORS	(min(MAX_MENUS, 21))

// masks for enabling/disabling menu items
#define ENAB_HIGH	0xFFFFFFFFL
#define ENAB_LSB	0x00000001L
#define ENAB_MSB	0x80000000L

struct MenuItems {
	short		num;			// number of menu items
	short		width;			// width of menu window
	ENAB_TYPE	enable;			// bit map of enabled items
	short		loc;			// location of menu name in menu bar
	char		*items[MAX_ITEMS+1];	// menu name and items
	char		hot[MAX_ITEMS+1];	// hot key character
	char		equiv[MAX_ITEMS+1];	// control character equivalent
	char		check[MAX_ITEMS+1];	// check mark character for menu item
	short		popup[MAX_ITEMS+1];	// popup menu id
	short		help[MAX_ITEMS+1];	// help id
	short		flash;			// menu name mask - control appearance
	Boolean		changed;		// flag set if menu contents were updated
	};

struct MenuList {
	short		id;			// resource menu id
	MenuItems	*menu;			// pointer to MenuItems structure
	};

struct MenuDef {
	short	vsnum;				// associated virtual screen
	Rect	r;				// current location of menu bar
	short	num;				// number of menus in list
	short	id[MAX_MENUS+1];		// list of associated menu id's
	short	level;				// level of menus open
	short	current;			// current hilited menu
	MenuDef	*prev;				// previous menu definition
	};

extern short	menuLow, menuHigh, menuHot, menuInvLow, menuInvHigh;
extern MenuFunc	menuCallBack;
extern short	mListNum;
extern MenuList	mList[MAX_LIST];
extern MenuDef	*theMenu;

void		MenuBarCalcLoc(void);
void		MenuBarClear(void);
void		MenuBarDraw(void);
void		MenuCalcSize(MenuItems *mp);
void		MenuClose(short vsnum);
void		MenuDoCommand(short id, short item);
void		MenuDrawItems(short vsnum, short id, short pos, short num);
void		MenuErrorCommand(short item);
void		MenuGetPos(short count, short width, short *top, short *lef, short *max, short *num);
short		MenuGetSelection(short keyCode, short *index);
void		MenuHilite(short id);
void		MenuIDAlloc(short id, MenuItems *menu);
MenuItems	*MenuIDFind(short id, Boolean fail=True);
MenuItems	*MenuIDFree(short id);
void		MenuItemFlash(short vsnum, short id, short row, short pos);
void		MenuItemHilite(short vsnum, short id, short row, short pos, Boolean hilite);
void		MenuItemMakeStr(char *bptr, char check, char *item, char hot, char equiv, short popup, short width);
short		MenuItemNext(short id, short item);
short		MenuItemPrev(short id, short item);
short		MenuOpen(short top, short lef, short num, short width);
short		MenuPullDown(short top, short lef, short id);
