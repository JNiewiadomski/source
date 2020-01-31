#include <fcntl.h>
#include <dir.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\coord.h>

#include "error.h"
#include "gen.h"

#include <seashell\resclass.h>
#include <seashell\resource.h>

#define	DIR_MEMO	".MEMO\t\""
#define DIR_FILE	".FILE\t"

#define NEWLINE		"\r\n"
#define COMMENT		"// "
#define QUOTE		"\""
#define SEMICOLON	";"
#define INDENT		"\t"
#define COMMA		","
#define COMMA_SPACE	", "
#define COMMA_TAB	",\t"
#define COMMA_NEWLINE	",\r\n"
#define ASTERISK	"*"
#define COLON		":"

#define RESOURCE	"\r\nresource "

#define START_BLOCK	") {\r\n"
#define END_BLOCK	"\t};\r\n"

#define PRELOAD		", preload"
#define PURGEABLE	", purgeable"

#define NULL_ADR	0
#define NULL_ADR_STR	"NULL"

static int	fInHand, fOutHand;	// handle to resource source file

static Word	dir_num;
static ResTable	*dir_tab;


static void	GenRes(void);
static void	GenResASCIIZ(char *s);
static void	GenResBar(void);
static void	GenResChar(char ch);
static void	GenResColor(void);
static void	GenResCoord(void);
static void	GenResDialog(void);
static void	GenResError(void);
static void	GenResForm(void);
static void	GenResHelp(void);
static void	GenResId(short id);
static void	GenResInternat(void);
static void	GenResList(void);
static void	GenResMenu(void);
static void	GenResPopup(void);
static void	GenResPrompt(void);
static void	GenResStatus(void);
static void	GenResString(void);
static void	GenResVersion(void);
static void	GenResVideo(void);
static void	GenResWindow(void);


void Gen(char *src, char *dst) {
	char	drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	char	src_name[MAXPATH], dst_name[MAXPATH], ch, sig[sizeof(RES_SIG)];
	long	resDirLoc;
	Word	size;

	// build source file name
	strcpy(src_name, src);
	if (!(fnsplit(src_name, drive, dir, name, ext) & EXTENSION)) {
		strcat(src_name, RES_OUT_EXT);
		strcpy(ext, RES_OUT_EXT);
		}
	strupr(src_name);
	if ((fInHand = open(src_name, O_RDONLY | O_BINARY)) == -1)
		ErrorFatal("Unable to open file '%s'", src_name);

	// build destination file name
	if (dst) {
		strcpy(dst_name, dst);
		if (!(fnsplit(dst_name, NULL, NULL, NULL, NULL) & EXTENSION))
			strcat(dst_name, RES_IN_EXT);
		}
	else
		fnmerge(dst_name, drive, dir, name, RES_IN_EXT);
	strupr(dst_name);
	if ((fOutHand = open(dst_name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) == -1)
		ErrorFatal("Unable to create file '%s'", dst_name);

	// write out .FILE info only if extension not default or not same name
	if (dst || stricmp(ext, RES_OUT_EXT)) {
		write(fOutHand, DIR_FILE, sizeof(DIR_FILE)-1);
		GenResASCIIZ(src_name);
		write(fOutHand, SEMICOLON, sizeof(SEMICOLON)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}

	// write out .MEMO info
	write(fOutHand, DIR_MEMO, sizeof(DIR_MEMO)-1);
	while (read(fInHand, &ch, sizeof(ch)) == sizeof(ch) && ch != A_CTRLM)
		write(fOutHand, &ch, sizeof(ch));
	read(fInHand, &ch, sizeof(ch));
	read(fInHand, &ch, sizeof(ch));
	write(fOutHand, QUOTE, sizeof(QUOTE)-1);
	write(fOutHand, SEMICOLON, sizeof(SEMICOLON)-1);
	write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
	write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);

	// check signiature
	read(fInHand, sig, sizeof(RES_SIG));
	if (memcmp(RES_SIG, sig, sizeof(RES_SIG)) != 0) {
		close(fOutHand);
		ErrorFatal("Signiature mismatch (%s)", sig);
		}
	write(fOutHand, COMMENT, sizeof(COMMENT)-1);
	write(fOutHand, RES_SIG, sizeof(RES_SIG)-1);
	write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);

	// read in directory table
	read(fInHand, &resDirLoc, sizeof(resDirLoc));
	lseek(fInHand, resDirLoc, SEEK_SET);
	read(fInHand, &dir_num, sizeof(dir_num));
	if (dir_num > 0) {
		size = dir_num * sizeof(ResTable);
		if ((dir_tab = (ResTable *) malloc(size)) == NULL)
			e_OutOfMemory("Gen");
		read(fInHand, dir_tab, size);
		GenRes();
		}

	// clean up after ourselves
	close(fOutHand);
	close(fInHand);
	}


static void GenRes(void) {
	short		type;
	char		id[7];
	static char	*resType[] = {
				"bar (",	"color (",	"coord (",
				"dialog (",	"error (",	"form (",
				"help (",	"internat (",	"list (",
				"menu (",	"popup (",	"prompt (",
				"status (",	"string (",	"version (",
				"video (",	"window ("
				};

	for ( ; dir_num > 0; dir_num--, dir_tab++) {
		write(fOutHand, RESOURCE, sizeof(RESOURCE)-1);
		type = (short)(dir_tab->type_id >> 16);
		write(fOutHand, resType[type], strlen(resType[type]));
		itoa((short)(dir_tab->type_id & 0x0000FFFF), id, 10);
		write(fOutHand, id, strlen(id));
		if (dir_tab->preload)
			write(fOutHand, PRELOAD, sizeof(PRELOAD)-1);
		if (dir_tab->purgeable)
			write(fOutHand, PURGEABLE, sizeof(PURGEABLE)-1);
		write(fOutHand, START_BLOCK, sizeof(START_BLOCK)-1);
		switch (type) {
			case R_Bar:
				GenResBar();
				break;
			case R_Color:
				GenResColor();
				break;
			case R_Coord:
				GenResCoord();
				break;
			case R_Dialog:
				GenResDialog();
				break;
			case R_Error:
				GenResError();
				break;
			case R_Form:
				GenResForm();
				break;
			case R_Help:
				GenResHelp();
				break;
			case R_Internat:
				GenResInternat();
				break;
			case R_List:
				GenResList();
				break;
			case R_Menu:
				GenResMenu();
				break;
			case R_Popup:
				GenResPopup();
				break;
			case R_Prompt:
				GenResPrompt();
				break;
			case R_Status:
				GenResStatus();
				break;
			case R_String:
				GenResString();
				break;
			case R_Version:
				GenResVersion();
				break;
			case R_Video:
				GenResVideo();
				break;
			case R_Window:
				GenResWindow();
				break;
			}
		write(fOutHand, END_BLOCK, sizeof(END_BLOCK)-1);
		}
	}


// Outputs a quoted ASCIIZ string surrounded by double quotes (").  Escape
// sequences are expanded.

#define ASCIIZ_ESC_REAL		"\a\b\f\n\r\t\v\x01A\\\""
#define ASCIIZ_ESC_SLASH	"\\"
#define ASCIIZ_ESC_SYMBOL	"abfnrtvz\\\""

static void GenResASCIIZ(char *src) {
	char	*esc;

	write(fOutHand, QUOTE, sizeof(QUOTE)-1);
	while (*src) {
		if ((esc = strchr(ASCIIZ_ESC_REAL, *src)) == NULL)
			write(fOutHand, src, sizeof(*src));
		else {
			write(fOutHand, ASCIIZ_ESC_SLASH, 1);
			write(fOutHand, ASCIIZ_ESC_SYMBOL+(unsigned)(esc-ASCIIZ_ESC_REAL), 1);
			}
		src++;
		}
	write(fOutHand, QUOTE, sizeof(QUOTE)-1);
	}


static void GenResBar(void) {
	Word		size;
	ResBar		*bar_tab;
	register short	num;
	short		*id;
	char		buf[7];

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((bar_tab = (ResBar *) malloc(size)) == NULL)
		e_OutOfMemory("GenResBar");
	read(fInHand, bar_tab, size);
	num = bar_tab->num;
	id = bar_tab->id;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		itoa(*id++, buf, 10);
		write(fOutHand, buf, strlen(buf));
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(bar_tab);
	}


// Outputs a quoted CHARACTER surrounded by single quotes (').  Escape
// sequences are expanded.

#define CHAR_ESC_REAL	"\a\b\f\n\r\t\v\x01A\\'"
#define CHAR_ESC_SLASH	"\\"
#define CHAR_ESC_SYMBOL	"abfnrtvz\\'"
#define CHAR_QUOTE	"'"

static void GenResChar(char ch) {
	char	*esc;

	write(fOutHand, CHAR_QUOTE, sizeof(CHAR_QUOTE)-1);
	if ((esc = strchr(CHAR_ESC_REAL, ch)) == NULL)
		write(fOutHand, &ch, sizeof(ch));
	else {
		write(fOutHand, CHAR_ESC_SLASH, 1);
		write(fOutHand, CHAR_ESC_SYMBOL+(unsigned)(esc-CHAR_ESC_REAL), 1);
		}
	write(fOutHand, CHAR_QUOTE, sizeof(CHAR_QUOTE)-1);
	}


#define FORE_ATTR	0x0F
#define BACK_ATTR	0xF0
#define MONO_ATTR	0x0F

#define COLON_SPACE	" : "

static void GenResColor(void) {
	Word		size;
	ResColor	*col_tab;
	Attrib		*attr;
	Byte		val;
	register short	num;
	static char	*resColor[] = {
				"black",	"blue",		"green",
				"cyan",		"red",		"magenta",
				"brown",	"lightgray",	"darkgray",
				"lightblue",	"lightgreen",	"lightcyan",
				"lightred",	"lightmagenta",	"yellow",
				"white"
				};
	static char	*resMono[] = {
				"inverse",	"line",		"inverse",
				"inverse",	"inverse",	"inverse",
				"inverse",	"low",		"highinverse",
				"highline",	"inverse",	"inverse",
				"inverse",	"inverse",	"inverse",
				"high"
				};

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((col_tab = (ResColor *) malloc(size)) == NULL)
		e_OutOfMemory("GenResColor");
	read(fInHand, col_tab, size);
	num = col_tab->num;
	attr = col_tab->attr;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		val = attr->color;
		if (val & BACK_ATTR) {
			write(fOutHand, resColor[val>>4], strlen(resColor[val>>4]));
			write(fOutHand, ASTERISK, sizeof(ASTERISK)-1);
			write(fOutHand, resColor[val & FORE_ATTR], strlen(resColor[val & FORE_ATTR]));
			}
		else
			write(fOutHand, resColor[val], strlen(resColor[val]));
		write(fOutHand, COLON_SPACE, sizeof(COLON_SPACE)-1);
		val = attr->mono & MONO_ATTR;
		write(fOutHand, resMono[val], strlen(resMono[val]));
		if (num) {
			write(fOutHand, COMMA, sizeof(COMMA)-1);
			attr++;
			}
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(col_tab);
	}


static void GenResCoord(void) {
	Word		size;
	ResCoord	*coord_tab;
	register short	num;
	char		buf[7];
	Point		*p;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((coord_tab = (ResCoord *) malloc(size)) == NULL)
		e_OutOfMemory("GenResCoord");
	read(fInHand, coord_tab, size);
	num = coord_tab->num;
	p = coord_tab->coord;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);

		itoa(p->V(), buf, 10);
		write(fOutHand, buf, strlen(buf));
		write(fOutHand, COLON, sizeof(COLON)-1);

		itoa(p->H(), buf, 10);
		write(fOutHand, buf, strlen(buf));

		if (num) {
			write(fOutHand, COMMA, sizeof(COMMA)-1);
			p++;
			}
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(coord_tab);
	}


enum ElemType { E_List, E_Popup, E_Prompt };

static void GenResDialog(void) {
	Word		size;
	ResElem		*elem;
	ResDialog	*dialog_tab;
	register short	num;
	char		buf[7];
	static char	*resElem[] = {
				"\tlist=",	"\tpopup=",	"\tprompt="
				};

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((dialog_tab = (ResDialog *) malloc(size)) == NULL)
		e_OutOfMemory("GenResDialog");
	read(fInHand, dialog_tab, size);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResId(dialog_tab->window_id);
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResId(dialog_tab->status_id);
	if (dialog_tab->status_id != NULL_ADR && dialog_tab->status_idx > 0) {
		write(fOutHand, ASTERISK, sizeof(ASTERISK)-1);
		itoa(dialog_tab->status_idx, buf, 10);
		write(fOutHand, buf, strlen(buf));
		}
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	elem = dialog_tab->elem;
	num = dialog_tab->num;
	while (num--) {
		write(fOutHand, resElem[elem->type], strlen(resElem[elem->type]));

		GenResId(elem->id);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

		itoa(elem->loc.V(), buf, 10);
		write(fOutHand, buf, strlen(buf));
		write(fOutHand, COLON, sizeof(COLON)-1);
		itoa(elem->loc.H(), buf, 10);
		write(fOutHand, buf, strlen(buf));
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

		GenResId(elem->help_id);
		if (num) {
			write(fOutHand, COMMA, sizeof(COMMA)-1);
			elem++;
			}
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(dialog_tab);
	}


static void GenResError(void) {
	Word		size;
	ResError	*err_tab;
	register short	num;
	char		*err_str;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((err_tab = (ResError *) malloc(size)) == NULL)
		e_OutOfMemory("GenResError");
	read(fInHand, err_tab, size);
	num = err_tab->num;
	err_str = err_tab->error;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(err_str);
		err_str = strchr(err_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(err_tab);
	}


static void GenResForm(void) {
	Word		size;
	ResForm		*form_tab;
	register short	num;
	char		*form_str;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((form_tab = (ResForm *) malloc(size)) == NULL)
		e_OutOfMemory("GenResForm");
	read(fInHand, form_tab, size);
	num = form_tab->num;
	form_str = form_tab->form;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(form_str);
		form_str = strchr(form_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(form_tab);
	}


static void GenResHelp(void) {
	Word		size;
	ResHelp		*help_tab;
	register short	num;
	char		*help_str;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((help_tab = (ResHelp *) malloc(size)) == NULL)
		e_OutOfMemory("GenResHelp");
	read(fInHand, help_tab, size);
	num = help_tab->num;
	help_str = help_tab->help;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(help_str);
		help_str = strchr(help_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(help_tab);
	}


// Generates the resource id.  If id is equal to NULL_ADR, the label
// NULL_ADR_STR is written.  Otherwise the id value is written.  This
// function is provided to generate id's for resources that reference other
// optional resources by their id.  The NULL id has the special meaning which
// indicates that no resource is referenced.  Attempting to define a resource
// with an id of NULL in Rez will generate a severe error.

static void GenResId(short id) {
	char	buf[7];

	if (id == NULL_ADR)
		write(fOutHand, NULL_ADR_STR, sizeof(NULL_ADR_STR)-1);
	else {
		itoa(id, buf, 10);
		write(fOutHand, buf, strlen(buf));
		}
	}


#define COMMENT_FORMAT		",\t\t\t// date format (usa, europe, japan)\r\n\t"
#define COMMENT_THOUSANDS	",\t\t\t// thousands separator\r\n\t"
#define COMMENT_DECIMAL		",\t\t\t// decimal separator\r\n\t"
#define COMMENT_DATE_SEP	",\t\t\t// date separator\r\n\t"
#define COMMENT_TIME_SEP	",\t\t\t// time separator\r\n\t\""
#define COMMENT_DATE_TEMPL	"\",\t// date template\r\n\t\""
#define COMMENT_TIME_TEMPL	"\",\t// time template\r\n\t\""
#define COMMENT_LONG_TIME_TEMPL	"\"\t// long time template\r\n"

static void GenResInternat(void) {
	Word		size;
	ResInternat	*internat_tab;
	char		*internat_str;
	static char	*resFormat[] = {
				"usa",		"europe",	"japan"
				};

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((internat_tab = (ResInternat *) malloc(size)) == NULL)
		e_OutOfMemory("GenResInternat");
	read(fInHand, internat_tab, size);
	write(fOutHand, INDENT, sizeof(INDENT)-1);

	write(fOutHand, resFormat[internat_tab->format], strlen(resFormat[internat_tab->format]));
	write(fOutHand, COMMENT_FORMAT, sizeof(COMMENT_FORMAT)-1);

	GenResChar(internat_tab->thousands);
	write(fOutHand, COMMENT_THOUSANDS, sizeof(COMMENT_THOUSANDS)-1);

	GenResChar(internat_tab->decimal);
	write(fOutHand, COMMENT_DECIMAL, sizeof(COMMENT_DECIMAL)-1);

	GenResChar(internat_tab->date);
	write(fOutHand, COMMENT_DATE_SEP, sizeof(COMMENT_DATE_SEP)-1);

	GenResChar(internat_tab->time);
	write(fOutHand, COMMENT_TIME_SEP, sizeof(COMMENT_TIME_SEP)-1);

	internat_str = internat_tab->data;

	write(fOutHand, internat_str, strlen(internat_str));
	write(fOutHand, COMMENT_DATE_TEMPL, sizeof(COMMENT_DATE_TEMPL)-1);
	internat_str = strchr(internat_str, '\0') + 1;

	write(fOutHand, internat_str, strlen(internat_str));
	write(fOutHand, COMMENT_TIME_TEMPL, sizeof(COMMENT_TIME_TEMPL)-1);
	internat_str = strchr(internat_str, '\0') + 1;

	write(fOutHand, internat_str, strlen(internat_str));
	write(fOutHand, COMMENT_LONG_TIME_TEMPL, sizeof(COMMENT_LONG_TIME_TEMPL)-1);

	free(internat_tab);
	}


static void GenResList(void) {
	Word		size;
	ResList		*list_tab;
	register short	num;
	char		*list_str, buf[7];

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((list_tab = (ResList *) malloc(size)) == NULL)
		e_OutOfMemory("GenResList");
	read(fInHand, list_tab, size);
	num = list_tab->num;

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	itoa(list_tab->visible, buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	list_str = list_tab->list;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(list_str);
		list_str = strchr(list_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(list_tab);
	}


#define ATTR_FLASH	128
#define ATTR_NOFLASH	0

#define FLASH_STR	"flash,\t"
#define NOFLASH_STR	"noflash,\t"

static void GenResMenu(void) {
	Word		size;
	ResMenu		*menu_tab;
	ResMenuItem	*menu_item_tab;
	register short	num;
	char		*menu_str;
	short		len;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((menu_tab = (ResMenu *) malloc(size)) == NULL)
		e_OutOfMemory("GenResMenu");
	read(fInHand, menu_tab, size);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	menu_str = (menu_tab->flash == ATTR_FLASH) ? "flash,\t" : "noflash,\t";
	write(fOutHand, menu_str, strlen(menu_str));

	GenResId(menu_tab->help_id);
	write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

	menu_str = menu_tab->title;
	len = strlen(menu_str);
	GenResASCIIZ(menu_str);
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	num = menu_tab->num;
	while (num--) {
		menu_item_tab = (ResMenuItem *)(menu_str + len + 1);

		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResId(menu_item_tab->popup_id);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

		GenResId(menu_item_tab->help_id);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

		menu_str = menu_item_tab->item_meta;
		len = strlen(menu_str);
		GenResASCIIZ(menu_str);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);
		menu_str += len + 1;

		len = strlen(menu_str);
		GenResASCIIZ(menu_str);
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(menu_tab);
	}


static void GenResPopup(void) {
	Word		size;
	ResPopup	*popup_tab;
	ResMenuItem	*menu_item_tab;
	register short	num;
	char		*menu_str;
	short		len;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((popup_tab = (ResPopup *) malloc(size)) == NULL)
		e_OutOfMemory("GenResPopup");
	read(fInHand, popup_tab, size);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResId(popup_tab->help_id);
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	menu_str = popup_tab->data;
	len = -1;

	num = popup_tab->num;
	while (num--) {
		menu_item_tab = (ResMenuItem *)(menu_str + len + 1);

		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResId(menu_item_tab->popup_id);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

		GenResId(menu_item_tab->help_id);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);

		menu_str = menu_item_tab->item_meta;
		len = strlen(menu_str);
		GenResASCIIZ(menu_str);
		write(fOutHand, COMMA_TAB, sizeof(COMMA_TAB)-1);
		menu_str += len + 1;

		len = strlen(menu_str);
		GenResASCIIZ(menu_str);
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(popup_tab);
	}


static void GenResPrompt(void) {
	Word		size;
	ResPrompt	*prompt_tab;
	char		*prompt_str, buf[7];

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((prompt_tab = (ResPrompt *) malloc(size)) == NULL)
		e_OutOfMemory("GenResPrompt");
	read(fInHand, prompt_tab, size);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	itoa(prompt_tab->loc.V(), buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COLON, sizeof(COLON)-1);
	itoa(prompt_tab->loc.H(), buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	prompt_str = prompt_tab->template;	// template string
	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResASCIIZ(prompt_str);
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	prompt_str += prompt_tab->len + 1;	// mask string
	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResASCIIZ(prompt_str);
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	prompt_str += prompt_tab->len + 1;	// default string
	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResASCIIZ(prompt_str);
	write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);

	free(prompt_tab);
	}


static void GenResStatus(void) {
	Word		size;
	ResStatus	*stat_tab;
	register short	num;
	char		*stat_str;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((stat_tab = (ResStatus *) malloc(size)) == NULL)
		e_OutOfMemory("GenResStatus");
	read(fInHand, stat_tab, size);
	num = stat_tab->num;
	stat_str = stat_tab->status;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(stat_str);
		stat_str = strchr(stat_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(stat_tab);
	}


static void GenResString(void) {
	Word		size;
	ResString	*str_tab;
	register short	num;
	char		*str_str;

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((str_tab = (ResString *) malloc(size)) == NULL)
		e_OutOfMemory("GenResString");
	read(fInHand, str_tab, size);
	num = str_tab->num;
	str_str = str_tab->string;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(str_str);
		str_str = strchr(str_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(str_tab);
	}


#define COMMENT_VERSION		",\t\t\t// version\r\n\t"
#define COMMENT_REVISION	",\t\t\t// revision\r\n\t"
#define COMMENT_STAGE		",\t\t\t// stage\r\n\t"
#define COMMENT_RELEASE		",\t\t\t// release / bug fix\r\n\t\""
#define COMMENT_VERSION_STRING	"\",\t\t\t// version string\r\n\t\""
#define COMMENT_COUNTRY		"\",\t\t// country\r\n\t\""
#define COMMENT_DATE		"\"\t// date of release\r\n"

static void GenResVersion(void) {
	Word		size;
	ResVersion	*version_tab;
	char		*data_str, buf[7];

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((version_tab = (ResVersion *) malloc(size)) == NULL)
		e_OutOfMemory("GenResVersion");
	read(fInHand, version_tab, size);
	write(fOutHand, INDENT, sizeof(INDENT)-1);

	itoa(version_tab->version, buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COMMENT_VERSION, sizeof(COMMENT_VERSION)-1);

	itoa(version_tab->revision, buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COMMENT_REVISION, sizeof(COMMENT_REVISION)-1);

	GenResChar(version_tab->stage);
	write(fOutHand, COMMENT_STAGE, sizeof(COMMENT_STAGE)-1);

	itoa(version_tab->release, buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COMMENT_RELEASE, sizeof(COMMENT_RELEASE)-1);

	data_str = version_tab->data;

	write(fOutHand, data_str, strlen(data_str));
	write(fOutHand, COMMENT_VERSION_STRING, sizeof(COMMENT_VERSION_STRING)-1);
	data_str = strchr(data_str, '\0') + 1;

	write(fOutHand, data_str, strlen(data_str));
	write(fOutHand, COMMENT_COUNTRY, sizeof(COMMENT_COUNTRY)-1);
	data_str = strchr(data_str, '\0') + 1;

	write(fOutHand, data_str, strlen(data_str));
	write(fOutHand, COMMENT_DATE, sizeof(COMMENT_DATE)-1);

	free(version_tab);
	}


static void GenResVideo(void) {
	Word		size;
	ResVideo	*video_tab;
	register short	num;
	char		*video_str, buf[7];

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((video_tab = (ResVideo *) malloc(size)) == NULL)
		e_OutOfMemory("GenResVideo");
	read(fInHand, video_tab, size);
	num = video_tab->num;
	itoa(video_tab->color_id, buf, 10);
	write(fOutHand, INDENT, sizeof(INDENT)-1);
	write(fOutHand, buf, strlen(buf));
	if (video_tab->color_idx > 0) {
		itoa(video_tab->color_idx, buf, 10);
		write(fOutHand, ASTERISK, sizeof(ASTERISK)-1);
		write(fOutHand, buf, strlen(buf));
		}
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);
	video_str = video_tab->video;
	while (num--) {
		write(fOutHand, INDENT, sizeof(INDENT)-1);
		GenResASCIIZ(video_str);
		video_str = strchr(video_str, '\0') + 1;
		if (num)
			write(fOutHand, COMMA, sizeof(COMMA)-1);
		write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
		}
	free(video_tab);
	}


static void GenResWindow(void) {
	Word		size;
	ResWindow	*window_tab;
	char		*window_str, buf[7];
	static char	*resFrame[] = {
				"wnone",	"w0000",	"w1111",
				"w2222",	"w2111"
				};

	lseek(fInHand, dir_tab->loc, SEEK_SET);
	read(fInHand, &size, sizeof(size));
	if ((window_tab = (ResWindow *) malloc(size)) == NULL)
		e_OutOfMemory("GenResWindow");
	read(fInHand, window_tab, size);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	itoa(window_tab->loc.Top(), buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COLON, sizeof(COLON)-1);
	itoa(window_tab->loc.Lef(), buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COLON, sizeof(COLON)-1);
	itoa(window_tab->loc.Bot(), buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COLON, sizeof(COLON)-1);
	itoa(window_tab->loc.Rig(), buf, 10);
	write(fOutHand, buf, strlen(buf));
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	itoa(window_tab->color_id, buf, 10);
	write(fOutHand, buf, strlen(buf));
	if (window_tab->color_idx > 0) {
		write(fOutHand, ASTERISK, sizeof(ASTERISK)-1);
		itoa(window_tab->color_idx, buf, 10);
		write(fOutHand, buf, strlen(buf));
		}
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	window_str = resFrame[window_tab->frame];
	write(fOutHand, window_str, strlen(window_str));
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResId(window_tab->video_id);
	if (window_tab->video_id != NULL_ADR) {
		write(fOutHand, COMMA_SPACE, sizeof(COMMA_SPACE)-1);
		itoa(window_tab->video_loc.V(), buf, 10);
		write(fOutHand, buf, strlen(buf));
		write(fOutHand, COLON, sizeof(COLON)-1);
		itoa(window_tab->video_loc.H(), buf, 10);
		write(fOutHand, buf, strlen(buf));
		}
	write(fOutHand, COMMA_NEWLINE, sizeof(COMMA_NEWLINE)-1);

	write(fOutHand, INDENT, sizeof(INDENT)-1);
	GenResASCIIZ(window_tab->title);

	write(fOutHand, NEWLINE, sizeof(NEWLINE)-1);
	free(window_tab);
	}
