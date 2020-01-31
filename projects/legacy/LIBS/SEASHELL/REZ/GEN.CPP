#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#include <common\common.h>
#include <common\ascii.h>
#include <common\colors.h>
#include <common\coord.h>
#include <common\system.h>

#include "error.h"
#include "gen.h"
#include "lex.h"
#include "parse.h"

#include <seashell\resclass.h>
#include <seashell\resource.h>

typedef struct b_tree {
	ResTable	r;			// resource table image
	struct b_tree	*left, *right;		// pointer to children
	} B_TREE;


static char	*outFileName=NULL, *outPath="", *outMemo=NULL;
static int	outFileHand=-1;
static long	outDirLocLoc, outDirLoc=0L, size_loc;
static B_TREE	*root=NULL;
static Word	numNode=0;
static L_TOKEN	colors[] = {
			L_BLACK,	L_BLUE,		L_GREEN,
			L_CYAN,		L_RED,		L_MAGENTA,
			L_BROWN,	L_LIGHTGRAY,	L_DARKGRAY,
			L_LIGHTBLUE,	L_LIGHTGREEN,	L_LIGHTCYAN,
			L_LIGHTRED,	L_LIGHTMAGENTA,	L_YELLOW,
			L_WHITE,	L_EOI
			};
static L_TOKEN	monos[] = {	// all but first L_INVERSE are place holders
			L_INVERSE,	L_LINE,		L_INVERSE,
			L_INVERSE,	L_INVERSE,	L_INVERSE,
			L_INVERSE,	L_LOW,		L_HIGHINVERSE,
			L_HIGHLINE,	L_INVERSE,	L_INVERSE,
			L_INVERSE,	L_INVERSE,	L_INVERSE,
			L_HIGH,		L_EOI
			};
static L_TOKEN	frames[] = {
			L_WNONE,	L_W0000,	L_W1111,
			L_W2222,	L_W2111,	L_EOI
			};
static L_TOKEN	date_format[] = {
			L_USA,		L_EUROPE,	L_JAPAN,
			L_EOI
			};
static L_TOKEN	elem_list[] = {
			L_LIST,		L_MENU,		L_PROMPT,
			L_EOI
			};


// The following static record is used to minimize parameter passing when
// doing a recursive search through the binary tree.

static ResTable	cur_res;


static void	GenASCIIZ(Word maxLen);
static long	GenBinFindTraverse(B_TREE *node);
static void	GenBinInsertTraverse(B_TREE **node);
static ResTable	*GenBinNumTraverse(B_TREE *node);
static void	GenBinSaveLocTraverse(B_TREE *node);
static void	GenPoint(void);
static void	GenRect(void);
static void	GenResourceSizeEnd(void);
static void	GenResourceSizeStart(void);
static void	GenVerifyResource(short type, Boolean option, Boolean index, char *desc);


// If the current token is ASCIIZ, the string is written out to the current
// resource file and the token is advanced.  If the length of the string is
// greater than maxLen then a diagnostic is issued.  If maxLen is 0, no check
// is performed.  If the current token is not ASCIIZ a diagnostic is issued.

static void GenASCIIZ(Word maxLen) {
	Word	len;

	if (LexMatch(L_ASCIIZ)) {
		len = strlen(yytext) + 1;
		if (maxLen > 0 && len > maxLen+1)	// take NULL into account
			ErrorSevere("Maximum length of string is %u", maxLen);
		write(outFileHand, yytext, len);
		LexAdvance();
		}
	else
		ErrorSevere("Expecting an ASCIIZ string");
	}


// GenBinFind() returns the resource location assigned to type_id.
// GEN_NOT_FOUND is returned if no match is made.

long GenBinFind(short type, short id) {
	cur_res.type_id = ShortShort(type, id);
	return (GenBinFindTraverse(root));
	}


// GenBinFindTraverse() does a recursive binary tree descent searching for
// a matching type_id.  The resource location is returned if a match is
// made, GEN_NOT_FOUND otherwise.

static long GenBinFindTraverse(B_TREE *node) {
	if (node == NULL)
		return (GEN_NOT_FOUND);
	if (cur_res.type_id == node->r.type_id)
		return (node->r.loc);
	else if (cur_res.type_id < node->r.type_id)
		return (GenBinFindTraverse(node->left));
	else
		return (GenBinFindTraverse(node->right));
	}


// GenBinInsert() inserts a node into the binary tree with type_id
// containing loc.

void GenBinInsert(short type, short id, Boolean preload, Boolean purgeable) {
	if (id == NULL_ADR)
		ErrorSevere("Illegal id of NULL (0)");
	else {
		cur_res.type_id = ShortShort(type, id);
		cur_res.loc = tell(outFileHand);
		cur_res.block = NULL;
		cur_res.preload = preload;
		cur_res.purgeable = purgeable;
		GenBinInsertTraverse(&root);
		}
	}


// GenBinInsertTraverse() recursively traverses the binary tree and
// inserts a new node into the tree.

static void GenBinInsertTraverse(B_TREE **node) {
	if (*node == NULL) {
		if ((*node = (B_TREE *) malloc(sizeof(B_TREE))) == NULL)
			e_OutOfMemory("GenBinInsertTraverse");
		memmove(&((*node)->r), &cur_res, sizeof(cur_res));
		(*node)->left = (*node)->right = NULL;
		numNode++;		// keep count of number of insertions
		}
	else if (cur_res.type_id == (*node)->r.type_id)
		ErrorSevere("Redefining resource type=%d id=%d", ShortHi(cur_res.type_id), ShortLo(cur_res.type_id));
	else if (cur_res.type_id < (*node)->r.type_id)
		GenBinInsertTraverse(&(*node)->left);
	else
		GenBinInsertTraverse(&(*node)->right);
	}


// GenBinNumGet() returns the number of items that the specified resource
// contains.  The number of items was stored in ResTable.block by
// GenBinNumSet().  This double use of the variable is ok at this point
// because block is not used by the compiler at all and it is set back to
// NULL when the table is written out to disk.
//
// The number of items within a resource is needed when one resource
// references another and specifies an offset into the resource.  We want to
// do as much error checking at compile time as we can, so we return the
// number of items within the resource so that an out of bounds index error
// can be detected.

Byte GenBinNumGet(short type, short id) {
	cur_res.type_id = ShortShort(type, id);
	return ((Byte)(GenBinNumTraverse(root)->block));
	}


// GenBinNumSet() temporarily uses ResTable.block to hold the number of items
// within the specified resource.  This should prevent the compiler from
// slowing down when the number of items is needed via the GenBinNumGet()
// function.

void GenBinNumSet(short type, short id, Byte num) {
	cur_res.type_id = ShortShort(type, id);
	GenBinNumTraverse(root)->block = (void *)(num);
	}


// GenBinNumTraverse() does a recursive binary tree descent searching for
// a matching type_id.  The resource address of the block pointer is returned
// if a match is made, NULL otherwise.

static ResTable *GenBinNumTraverse(B_TREE *node) {
	if (node == NULL)
		return (NULL);
	if (cur_res.type_id == node->r.type_id)
		return (&(node->r));
	else if (cur_res.type_id < node->r.type_id)
		return (GenBinNumTraverse(node->left));
	else
		return (GenBinNumTraverse(node->right));
	}


// GenBinSaveTraverseLoc() recursively traverses the binary tree saving the
// nodes to a file pointed to by cur_fp.

static void GenBinSaveLocTraverse(B_TREE *node) {
	if (node == NULL)
		return;
	GenBinSaveLocTraverse(node->left);
	node->r.block = NULL;
	write(outFileHand, &(node->r), sizeof(ResTable));
	GenBinSaveLocTraverse(node->right);
	}


// GenDelete() deletes the output resource file just created.  This is done
// when any severe errors have been issued.

void GenDelete(void) {
	unlink(outFileName);
	}


// Sets outFileName to the name of the file to be generated.  If the pointer
// is not NULL, the space is first deallocated before we get room for the
// new string.

void GenOutFile(char *name) {
	if (outFileName)
		free(outFileName);
	if ((outFileName = (char *) malloc(strlen(outPath)+strlen(name)+1)) == NULL)
		e_OutOfMemory("GenOutName");
	strcpy(outFileName, outPath);
	strcat(outFileName, name);
	}


// Registers path as the location where the output file will be saved.

void GenOutPath(char *path) {
	outPath = path;
	}


// Sets outMemo to the string seen when the file is TYPE'd.  If the pointer
// is not NULL, the space is first deallocated before we get room for the
// new string.

void GenOutMemo(char *memo) {
	if (outMemo)
		free(outMemo);
	if ((outMemo = strdup(memo)) == NULL)
		e_OutOfMemory("GenOutName");
	}


// Expects next three tokens to specify a Point using the format top:lef.
// The values are checked to make sure that they express a Point within the
// video screen space.

#define SCREEN_HEIGHT	50
#define SCREEN_WIDTH	80

static void GenPoint(void) {
	short	top, lef;
	Point	p;

	top = ParseValue();
	if (top < 0 || top > SCREEN_HEIGHT-1)
		ErrorWarning("Vertical coordinate must be between 0 and %d", SCREEN_HEIGHT-1);
	ParseMatchAdvance(L_COLON, ":");
	lef = ParseValue();
	if (top < 0 || lef > SCREEN_WIDTH-1)
		ErrorWarning("Horizontal coordinate must be between 0 and %d", SCREEN_WIDTH-1);
	p.Set(top, lef);
	write(outFileHand, &p, sizeof(p));
	}


// Expects next seven tokens to specify a Rect using the format
// top:lef:bot:rig.  The values are checked to make sure that they express a
// valid Rect within the video screen space.

static void GenRect(void) {
	short	top, lef, bot, rig;
	Rect	r;

	top = ParseValue();
	if (top < 0 || top > SCREEN_HEIGHT-1)
		ErrorWarning("Vertical coordinate must be between 0 and %d", SCREEN_HEIGHT-1);
	ParseMatchAdvance(L_COLON, ":");
	lef = ParseValue();
	if (lef < 0 || lef > SCREEN_WIDTH-1)
		ErrorWarning("Horizontal coordinate must be between 0 and %d", SCREEN_WIDTH-1);
	ParseMatchAdvance(L_COLON, ":");
	bot = ParseValue();
	if (bot < 0 || bot > SCREEN_HEIGHT-1)
		ErrorWarning("Vertical coordinate must be between 0 and %d", SCREEN_HEIGHT-1);
	ParseMatchAdvance(L_COLON, ":");
	rig = ParseValue();
	if (rig < 0 || rig > SCREEN_WIDTH-1)
		ErrorWarning("Horizontal coordinate must be between 0 and %d", SCREEN_WIDTH-1);
	if (top > bot || lef > rig)
		ErrorWarning("Imaginary rectangle specified");
	r.Set(top, lef, bot, rig);
	write(outFileHand, &r, sizeof(r));
	}


// GenRead() attempts to open up the resource file specified by the
// current lexical token and copy all of the resources found to the current
// resource file.

#define RES_BUF_SIZE	512

void GenRead(void) {
	char		drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	char		src_name[MAXPATH], ch, sig[sizeof(RES_SIG)], buf[RES_BUF_SIZE];
	char		*pp;
	int		fInHand;
	long		resDirLoc;
	Word		size, dir_num, cur_size;
	ResTable	*dir_tab, *dir_pos;
	short		type, id;
	Boolean		preload, purgeable;

	// build source file name
	strcpy(src_name, yytext);
	LexAdvance();
	if (!(fnsplit(src_name, drive, dir, name, ext) & EXTENSION)) {
		strcat(src_name, RES_OUT_EXT);
		strcpy(ext, RES_OUT_EXT);
		}
	strupr(src_name);
	pp = searchpath(src_name);
	if (pp == NULL || (fInHand = open(pp, O_RDONLY | O_BINARY)) == -1) {
		ErrorSevere("Unable to open file '%s'", src_name);
		return;
		}

	// read past dos type info
	while (read(fInHand, &ch, sizeof(ch)) == sizeof(ch) && ch != A_CTRLZ)
		;

	// check signiature
	read(fInHand, sig, sizeof(RES_SIG));
	if (memcmp(RES_SIG, sig, sizeof(RES_SIG)) != 0) {
		ErrorSevere("Signiature mismatch in resource file '%s'", src_name);
		return;
		}

	// read in directory table
	read(fInHand, &resDirLoc, sizeof(resDirLoc));
	lseek(fInHand, resDirLoc, SEEK_SET);
	read(fInHand, &dir_num, sizeof(dir_num));
	if (dir_num > 0) {
		size = dir_num * sizeof(ResTable);
		if ((dir_tab = (ResTable *) malloc(size)) == NULL)
			e_OutOfMemory("ParseRead");
		read(fInHand, dir_tab, size);

		for (dir_pos=dir_tab; dir_num; dir_num--, dir_pos++) {
			type = ShortHi(dir_pos->type_id);
			id = ShortLo(dir_pos->type_id);
			preload = (Boolean)dir_pos->preload;
			purgeable = (Boolean)dir_pos->purgeable;
			GenBinInsert(type, id, preload, purgeable);
			lseek(fInHand, dir_pos->loc, SEEK_SET);
			read(fInHand, &size, sizeof(size));
			write(outFileHand, &size, sizeof(size));
			while (size) {
				cur_size = min(size, RES_BUF_SIZE);
				read(fInHand, buf, cur_size);
				write(outFileHand, buf, cur_size);
				size -= cur_size;
				}
			}

		free(dir_tab);
		}

	// clean up after ourselves
	close(fInHand);
	}


// GenResourceBar() parses the input and generates the bar resource data.
//
// Syntax:
//		resource bar (128, preload, purgeable) {
//			0:0:0:79,
//			HelpMENU, MachineMENU, JobMENU, ShiftMENU, GraphMENU
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of colors in list
//		(Rect)<loc>			// top, left, bottom, right
//		(short)<id_1>			// 1st resource id
//		(short)<id_2>			// 2nd resource id
//		...
//		(short)<id_n>			// nth resource id
//
// Structure:
//		struct ResBar {
//			Byte	num;		// number of colors in table
//			Rect	loc;		// top, left, bottom, right
//			short	id[1];		// dummy label to id table
//			};

Byte GenResourceBar(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Boolean	done=false;
	short	id;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));

	GenRect();
	ParseMatchAdvance(L_COMMA, ",");

	while (!done) {
		if ((id = ParseValue()) == NULL_ADR)
			ErrorSevere("Bar id's in list must not be NULL");
		write(outFileHand, &id, sizeof(id));
		LexAdvance();
		num++;
		if (num == 0)
			ErrorSevere("Maximum number of attribute pairs in list is 255");
		if (LexMatch(L_COMMA))
			LexAdvance();
		else
			done = true;
		}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceColor() parses the input and generates the color resource data.
//
// Syntax:
//		resource color (128, preload, purgeable) {
//			cyan:low,		// color : monochrome pairs
//			lightcyan:highline,
//			cyan*black:inverse,	// background*foreground
//			cyan*white:highinverse
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of colors in list
//		(Byte)<color_1>(Byte)<mono_1>	// first color:mono pair
//		(Byte)<color_2>(Byte)<mono_2>	// second color:mono pair
//		...
//		(Byte)<color_n>(Byte)<mono_n>	// nth color:mono pair
//
// Structure:
//		struct Attrib {
//			Byte	color;		// color attribute
//			Byte	mono;		// mono attribute
//			};
//
//		struct ResColor {
//			Byte	num;		// number of colors in table
//			Attrib	attr[1];	// attribute pair table -- dummy label
//			};

#define MONO_ATTR	0x70

Byte GenResourceColor(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Boolean	done=false;
	Attrib	attr;
	short	val;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done) {
		if ((val = LexInList(colors)) == -1) {
			ErrorSevere("Expecting a color attribute");
			done = true;
			continue;		// skip rest of tests
			}
		LexAdvance();
		attr.color = val;
		if (LexMatch(L_ASTERISK)) {
			LexAdvance();
			if ((val = LexInList(colors)) == -1) {
				ErrorSevere("Expecting a color attribute");
				done = true;
				continue;	// skip rest of tests
				}
			LexAdvance();
			attr.color = (attr.color << 4) | val;
			}
		ParseMatchAdvance(L_COLON, ":");
		if ((val = LexInList(monos)) == -1) {
			ErrorSevere("Expecting a mono attribute");
			done = true;
			continue;		// skip rest of tests
			}
		LexAdvance();
		attr.mono = (val & ~BRIGHT) ? val : val | MONO_ATTR;
		write(outFileHand, &attr, sizeof(attr));
		num++;
		if (num == 0)
			ErrorSevere("Maximum number of attribute pairs in list is 255");
		if (LexMatch(L_COMMA))
			LexAdvance();
		else
			done = true;
		}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceCoord() parses the input and generates the coord resource data.
//
// Syntax:
//		resource coord (128, preload, purgeable) {
//			0:1,			// local coordinate list
//			0:15,
//			0:32,
//			0:51
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<num>			// number of coords in list
//		(Point)<coord_1>		// 1st coord
//		(Point)<coord_2>		// 2nd coord
//		...
//		(Point)<coord_n>		// nth coord
//
// Structure:
//		struct ResCoord {
//			Byte	num;		// number of colors in table
//			Point	coord[1];	// dummy label to coordinate list
//			};

Byte GenResourceCoord(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done) {
		GenPoint();
		if (++num == 0)
			ErrorSevere("Maximum number of coordinates in list is 255");
		if (LexMatch(L_COMMA))
			LexAdvance();
		else
			done = true;
		}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceDialog() parses the input and generates the dialog resource data.
//
// Syntax:
//		resource dialog (128, preload, purgeable) {
//			PreferencesWINDOW,	// id of window resource
//			PreferencesSTATUS*1,	// status id and index
//			prompt=DelayPROMPT,	1:1,	PromptHELP,
//			list=LanguageLIST,	2:1,	ListHELP,
//			menu=DefaultMENU,	3:1,	MenuHELP
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of video lines
//		(short)<window_id>		// id of window resource
//		(short)<st_id>(Byte)<st_idx>	// status resource id and index (NULL == none)
//		(short)<type_1>(short)<id_1>	// type and id of 1st resource element
//		(Point)<loc_1>			// coordinate of 1st element
//		(short)<help_1>			// id of help resource of 1st element
//
// Structure:
//		struct ResElem {
//			short	type;		// type of dialog resource element
//			short	id;		// id of dialog resource element
//			Point	loc;		// location of element
//			short	help_id;	// id of help resource
//			};			// menu item data follows menu title
//
//		struct ResDialog {
//			Byte	num;		// number of elements in table
//			short	window_id;	// id of window resource
//			short	status_id;	// id of status resource (NULL == none)
//			Byte	status_idx;	// index into status resource
//			ResElem	elem[1];	// table of elements
//			};

enum E_Type { E_List, E_Menu, E_Prompt };

Byte GenResourceDialog(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	short	top, lef, type;
	Boolean	done=false;
	Point	item_loc;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));

	GenVerifyResource(R_Window, false, false, "window");
	ParseMatchAdvance(L_COMMA, ",");

	GenVerifyResource(R_Status, true, true, "status");
	ParseMatchAdvance(L_COMMA, ",");

	while (!done) {
		if ((type = LexInList(elem_list)) == -1) {
			ErrorSevere("Expecting a dialog element type");
			done = true;
			continue;		// skip rest of tests
			}
		LexAdvance();
		write(outFileHand, &type, sizeof(type));
		ParseMatchAdvance(L_EQUALS, "=");
		switch (type) {
			case E_List:
				GenVerifyResource(R_List, false, false, "list");
				break;
			case E_Menu:
				GenVerifyResource(R_Menu, false, false, "menu");
				break;
			case E_Prompt:
				GenVerifyResource(R_Prompt, false, false, "prompt");
				break;
			}
		ParseMatchAdvance(L_COMMA, ",");

		top = ParseValue();
		ParseMatchAdvance(L_COLON, ":");
		lef = ParseValue();
		item_loc.Set(top, lef);
		write(outFileHand, &item_loc, sizeof(item_loc));

		ParseMatchAdvance(L_COMMA, ",");
		GenVerifyResource(R_Help, true, false, "help");

		num++;
		if (num == 0)
			ErrorSevere("Maximum number of dialog items is 255");
		if (LexMatch(L_COMMA))
			LexAdvance();
		else
			done = true;
		}

	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceError() parses the input and generates the error resource data.
//
// Syntax:
//		resource error (128, preload, purgeable) {
//			"Cannot find file '%s'",
//			"Cannot open file '%s'"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of error strings in list
//		(char[size-1])			// error strings
//
// Structure:
//		struct ResError {
//			Byte	num;		// number of error strings in table
//			char	error[1];	// error strings list
//			};

Byte GenResourceError(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Word	len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			len = strlen(yytext) + 1;
			if (len > RES_ERR_LEN+1)
				ErrorSevere("Maximum error string length is %d", RES_ERR_LEN);
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of errors in list is 255");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceForm() parses the input and generates the form resource data.
//
// Syntax:
//		resource form (128, preload, purgeable) {
//			"Job Number  Machine Number  Pieces Required",
//			"-------------------------------------------"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of form strings in list
//		(char[size-1])			// form strings
//
// Structure:
//		struct ResForm {
//			Byte	num;		// number of form strings in table
//			char	form[1];	// form strings list
//			};

Byte GenResourceForm(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Word	len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			len = strlen(yytext) + 1;
			if (len > RES_FORM_LEN+1)
				ErrorSevere("Maximum form string length is %d", RES_FORM_LEN);
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of forms in list is 255");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceHelp() parses the input and generates the help resource data.
//
// Syntax:
//		resource help (128, preload, purgeable) {
//			"PCS100 Production Control Software",
//			"by Vorne Industries Incorporated"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of help strings in list
//		(char[size-1])			// help strings
//
// Structure:
//		struct ResHelp {
//			Byte	num;		// number of help strings in table
//			char	help[1];	// help strings list
//			};

Byte GenResourceHelp(void) {
	long	loc;			// location of start of resource
	Word	num=0, len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			len = strlen(yytext) + 1;
			if (len > RES_HELP_LEN+1)
				ErrorSevere("Maximum help string length is %d", RES_HELP_LEN);
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of help strings in list is 65535");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceInternat() parses the input and generates the international resource data.
//
// Syntax:
//		resource internat (128, preload, purgeable) {
//			usa,			// date format (usa, europe, japan)
//			',',			// thousands separator
//			'.',			// decimal separator
//			'-',			// date separator
//			':',			// time separator
//			"<MM-DD-YY>",		// date prompt
//			"<HH:MM:SS>",		// time prompt
//			"%02d-%02d-%04d",	// date template
//			"%02d:%02d:%02d",	// time template
//			"%4d:%02d:%02d"		// long time template
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<format>			// date format (usa, europe, japan)
//		(char)<thousands>		// thousands separator
//		(char)<decimal>			// decimal separator
//		(char)<date>			// date separator
//		(char)<time>			// time separator
//		(char *)<date prompt ptr>
//		(char *)<time prompt ptr>
//		(char *)<date template ptr>
//		(char *)<time template ptr>
//		(char *)<long time template ptr>
//		(char)<date prompt string>
//		(char)<time prompt string>
//		(char)<date template string>
//		(char)<time template string>
//		(char)<long time template string>
//
// Structure:
//		struct ResInternat {
//			Byte	format;		// date format (usa, europe, japan)
//			char	thousands;	// thousands separator
//			char	decimal;	// decimal separator
//			char	date;		// date separator
//			char	time;		// time separator
//			char	*datePrompt;	// pointers to data strings
//			char	*timePrompt;
//			char	*dateTemplate;
//			char	*timeTemplate;
//			char	*ltimeTemplate;
//			char	data[1];	// date, time and long time template strings
//			};

Byte GenResourceInternat(void) {
	short	val;
	Byte	format;
	char	ch, *s;

	GenResourceSizeStart();

	if ((val = LexInList(date_format)) == -1)
		ErrorSevere("Expecting a date format attribute");
	LexAdvance();
	format = val;
	write(outFileHand, &format, sizeof(format));

	ParseMatchAdvance(L_COMMA, ",");
	ch = ParseChar();			// thousands separator
	LexAdvance();
	write(outFileHand, &ch, sizeof(ch));

	ParseMatchAdvance(L_COMMA, ",");
	ch = ParseChar();			// decimal separator
	LexAdvance();
	write(outFileHand, &ch, sizeof(ch));

	ParseMatchAdvance(L_COMMA, ",");
	ch = ParseChar();			// date separator
	LexAdvance();
	write(outFileHand, &ch, sizeof(ch));

	ParseMatchAdvance(L_COMMA, ",");
	ch = ParseChar();			// time separator
	LexAdvance();
	write(outFileHand, &ch, sizeof(ch));

	s = NULL;
	write(outFileHand, s, sizeof(s));	// date prompt pointer
	write(outFileHand, s, sizeof(s));	// time prompt pointer
	write(outFileHand, s, sizeof(s));	// date template pointer
	write(outFileHand, s, sizeof(s));	// time template pointer
	write(outFileHand, s, sizeof(s));	// long time template pointer

	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// date prompt string

	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// time prompt string

	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// date template string

	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// time template string

	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// long time template string

	GenResourceSizeEnd();
	return (1);
	}


// GenResourceList() parses the input and generates the list resource data.
//
// Syntax:
//		resource list (128, preload, purgeable) {
//			2,			// number if items visible
//			"Running",
//			"Offline",
//			"Idle   ",
//			"Update "
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of list strings
//		(Byte)<len>			// length of each list string
//		(Byte)<visible>			// number of items visible
//		(**char)<list>			// pointer to list of strings
//		(char[size-3])			// string data
//
// Structure:
//		struct ResList {
//			Byte	num;		// number of list strings in table
//			Byte	len;		// length of each list string
//			Byte	visible;	// number of visible items
//			char	**list;		// pointer to list of strings
//			char	data[1];	// string data
//			};

Byte GenResourceList(void) {
	long	loc;			// location of start of resource
	Byte	num=0, maxLen=0, visible;
	char	**list;
	Word	len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	write(outFileHand, &maxLen, sizeof(maxLen));

	visible = ParseValue();
	write(outFileHand, &visible, sizeof(visible));
	ParseMatchAdvance(L_COMMA, ",");
	list = NULL;
	write(outFileHand, list, sizeof(list));
	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			if (maxLen == 0) {
				len = maxLen = strlen(yytext) + 1;
				if (len > RES_LIST_LEN+1)
					ErrorSevere("Maximum list length is %d", RES_LIST_LEN);
				}
			else if (maxLen != strlen(yytext)+1)
				ErrorSevere("All list items must be the same length");
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of list items is 255");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	write(outFileHand, &maxLen, sizeof(maxLen));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceMenu() parses the input and generates the menu resource data.
//
// Syntax:
//		resource menu (128, preload, purgeable) {
//			noflash,	AboutHELP,  "Help",
//			NULL,		AboutHELP,  "About PCS100...",	"!/A(=A",
//			LanguageMENU,	AboutHELP,  "Language...",	"=L",
//			NULL,		AboutHELP,  "Quit",		"/Q=Q"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of video lines
//		(Boolean)<flash>		// menu name flash/noflash flag
//		(short)<help>			// id of help resource (NULL == none)
//		(char *)<title>			// dummy label to menu title
//						// menu item data follows menu title
//		(short)<menu>			// id of menu resource (NULL == none)
//		(short><help>			// id of help resource (NULL == none)
//		(char *)<item_meta>		// menu item and meta string dummy label
//
// Structure:
//		struct ResMenu {
//			Byte	num;		// number of video lines
//			Boolean	flash;		// menu name flash/noflash flag
//			short	help;		// id of help resource (NULL == none)
//			char	title[1];	// dummy label to menu title
//			};			// menu item data follows menu title
//
//		struct ResMenuItem {
//			short	menu;		// id of menu resource (NULL == none)
//			short	help;		// id of help resource (NULL == none)
//			char	item_meta[1];	// menu item and meta string dummy label
//			};

#define MAX_MENU_ITEMS	31

Byte GenResourceMenu(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Boolean	done=false, flash=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));

	switch (LexLook()) {
		case L_FLASH:
			flash = true;
			LexAdvance();
			break;
		case L_NOFLASH:			// this is the default
			LexAdvance();
			break;
		default:
			ErrorSevere("Expecting flash or noflash keyword");
			break;
		}
	write(outFileHand, &flash, sizeof(flash));
	ParseMatchAdvance(L_COMMA, ",");

	GenVerifyResource(R_Help, true, false, "help");
	ParseMatchAdvance(L_COMMA, ",");

	GenASCIIZ(RES_MENU_NAME);
	ParseMatchAdvance(L_COMMA, ",");

	while (!done) {
		GenVerifyResource(R_Menu, true, false, "menu");
		ParseMatchAdvance(L_COMMA, ",");

		GenVerifyResource(R_Help, true, false, "help");
		ParseMatchAdvance(L_COMMA, ",");

		GenASCIIZ(RES_MENU_ITEM);
		ParseMatchAdvance(L_COMMA, ",");

		GenASCIIZ(RES_MENU_META);

		num++;
		if (num > MAX_MENU_ITEMS)
			ErrorSevere("Maximum number of menu items is %d", MAX_MENU_ITEMS);
		if (LexMatch(L_COMMA))
			LexAdvance();
		else
			done = true;
		}

	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourcePrompt() parses the input and generates the prompt resource data.
//
// Syntax:
//		resource prompt (128, preload, purgeable) {
//			0:16,			// top left local coordinate of prompt entry
//			"__ __ ____",		// data entry template
//			"MM-DD-YYYY",		// data entry mask
//			"01-01-1990"		// default value
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<len>			// length of prompt
//		(Point)<loc>			// local coordinate of prompt
//		(char *)<template>		// data entry template
//		(char *)<mask>			// data entry mask
//		(char *)<default>		// default value
//
// Structure:
//		struct ResVersion {
//			Byte	len;		// length of prompt
//			Point	loc;		// local coordinate of prompt
//			char	template[1];	// dummy label to template string
//			};			// followed by mask and default strings

Byte GenResourcePrompt(void) {
	long	loc;			// location of start of resource
	Byte	len=0;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &len, sizeof(len));

	GenPoint();
	ParseMatchAdvance(L_COMMA, ",");

	LexLook();				// force yytext to next token
	len = strlen(yytext);
	GenASCIIZ(RES_PROMPT_LEN);		// template string
	ParseMatchAdvance(L_COMMA, ",");

	LexLook();				// force yytext to next token
	if (strlen(yytext) != len)
		ErrorSevere("Template and mask lengths differ");
	GenASCIIZ(RES_PROMPT_LEN);		// mask string
	ParseMatchAdvance(L_COMMA, ",");

	LexLook();				// force yytext to next token
	if (strlen(yytext) > len)
		ErrorSevere("Default length greater than template");
	GenASCIIZ(RES_PROMPT_LEN);		// mask string

	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &len, sizeof(len));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (1);
	}


static void GenResourceSizeEnd(void) {
	long	actual_size;
	Word	size;

	actual_size = tell(outFileHand) - size_loc;
	if (actual_size == 0)
		ErrorSevere("Resource cannot have a size of 0");
	else if (actual_size > 65535L)
		ErrorSevere("Maximum size of resource is 65535 bytes");
	size =  (Word)actual_size;
	lseek(outFileHand, size_loc, SEEK_SET);
	write(outFileHand, &size, sizeof(size));
	lseek(outFileHand, 0L, SEEK_END);
	}


// Together with the GenResourceSizeEnd function, this pair keeps track of
// how big the generated resource actually is.  GenResourceSizeStart should
// be called before the actual resource contents are written, and
// GenResourceSizeEnd should be called when the resource has been generated.

static void GenResourceSizeStart(void) {
	Word	size=0;

	size_loc = tell(outFileHand);
	write(outFileHand, &size, sizeof(size));
	}


// GenResourceStatus() parses the input and generates the status resource data.
//
// Syntax:
//		resource status (128, preload, purgeable) {
//			"<^H>-Help  <^P>-Print  <^Q>-Quit",
//			"<^H>-Help  <^P>-Print  <Esc>-Exit"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of status strings
//		(char[size-1])			// status strings
//
// Structure:
//		struct ResStatus {
//			Byte	num;		// number of status strings in table
//			char	status[1];	// status strings
//			};

Byte GenResourceStatus(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Word	len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			len = strlen(yytext) + 1;
			if (len > RES_STAT_LEN+1)
				ErrorSevere("Maximum status length is %d", RES_STAT_LEN);
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of status items in list is 255");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceString() parses the input and generates the string resource data.
//
// Syntax:
//		resource string (128, preload, purgeable) {
//			"Nothing to print",
//			"Printing file '%s'"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of strings in list
//		(char[size-1])			// strings list
//
// Structure:
//		struct ResString {
//			Byte	num;		// number of strings in table
//			char	string[1];	// strings list
//			};

Byte GenResourceString(void) {
	long	loc;			// location of start of resource
	Byte	num=0;
	Word	len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			len = strlen(yytext) + 1;
			if (len > RES_STR_LEN+1)
				ErrorSevere("Maximum string length is %d", RES_STR_LEN);
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of strings in list is 255");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceVersion() parses the input and generates the version resource data.
//
// Syntax:
//		resource version (128, preload, purgeable) {
//			1,			// version
//			0,			// revision
//			'.',			// stage
//			0,			// release / bug fix
//			"v1.0",			// version string
//			"U.S.A.",		// country
//			"August 21, 1990"	// date of release
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<version>
//		(Byte)<revision>
//		(char)<stage>
//		(Byte)<release>
//		(char *)<version ptr>
//		(char *)<country ptr>
//		(char *)<date ptr>
//		(char)<version string>
//		(char)<country string>
//		(char)<date of release string>
//
// Structure:
//		struct ResVersion {
//			Byte	version;	// version
//			Byte	revision;	// revision
//			char	stage;		// stage
//			Byte	release;	// release / bug fix
//			char	*ver_str;	// version string pointer
//			char	*country;	// country string pointer\
//			char	*date;		// date string pointer
//			char	data[1];	// version, country and date strings
//			};

Byte GenResourceVersion(void) {
	Byte	version, revision, release;
	char	stage, *s;

	GenResourceSizeStart();
	version = ParseValue();
	write(outFileHand, &version, sizeof(version));
	ParseMatchAdvance(L_COMMA, ",");
	revision = ParseValue();
	write(outFileHand, &revision, sizeof(revision));
	ParseMatchAdvance(L_COMMA, ",");
	stage = ParseChar();
	write(outFileHand, &stage, sizeof(stage));
	if (stage != 'ë' && stage != 'à' && stage != 'á' && stage != '.')
		ErrorSevere("Version stage must be 'ë', 'à', 'á' or '.'");
	ParseMatchAdvance(L_COMMA, ",");
	release = ParseValue();
	write(outFileHand, &release, sizeof(release));

	s = NULL;
	write(outFileHand, s, sizeof(s));	// version string pointer
	write(outFileHand, s, sizeof(s));	// country string pointer
	write(outFileHand, s, sizeof(s));	// date string pointer

	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// version string
	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// country string
	ParseMatchAdvance(L_COMMA, ",");
	GenASCIIZ(0);				// date of release string

	GenResourceSizeEnd();
	return (1);
	}


// GenResourceVideo() parses the input and generates the video resource data.
//
// Syntax:
//		resource video (128, preload, purgeable) {
//			VideoCOLOR*1,		// id of color resource and index
//			"Welcome to PCS100  ",
//			"by Vorne Industries"
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Byte)<n>			// number of video lines
//		(Byte)<len>			// length of each video line
//		(short)<color_id>(Byte)<index>	// id of color resource and index
//		(char[size-2])			// video lines
//
// Structure:
//		struct ResVideo {
//			Byte	num;		// number of video lines
//			Byte	len;		// length of each video line
//			short	color_id;	// id of color resource
//			Byte	color_idx;	// index into color resource
//			char	video[1];	// video list
//			};

Byte GenResourceVideo(void) {
	long	loc;			// location of start of resource
	Byte	num=0, maxLen=0;
	Word	len;
	Boolean	done=false;

	GenResourceSizeStart();
	loc = tell(outFileHand);
	write(outFileHand, &num, sizeof(num));
	write(outFileHand, &maxLen, sizeof(maxLen));

	GenVerifyResource(R_Color, false, true, "color");
	ParseMatchAdvance(L_COMMA, ",");

	while (!done)
		if (LexMatch(L_ASCIIZ)) {
			if (maxLen == 0) {
				len = maxLen = strlen(yytext) + 1;
				if (len > RES_VID_LEN+1)
					ErrorSevere("Maximum video length is %d", RES_VID_LEN);
				}
			else if (maxLen != strlen(yytext)+1)
				ErrorSevere("All video items must be the same length");
			write(outFileHand, yytext, len);
			num++;
			if (num == 0)
				ErrorSevere("Maximum number of video items is 255");
			LexAdvance();
			if (LexMatch(L_COMMA))
				LexAdvance();
			else
				done = true;
			}
		else {
			ErrorSevere("Expecting an ASCIIZ string");
			done = true;
			}
	lseek(outFileHand, loc, SEEK_SET);
	write(outFileHand, &num, sizeof(num));
	write(outFileHand, &maxLen, sizeof(maxLen));
	lseek(outFileHand, 0L, SEEK_END);
	GenResourceSizeEnd();
	return (num);
	}


// GenResourceWindow() parses the input and generates the window resource data.
//
// Syntax:
//		resource window (128, preload, purgeable) {
//			9:4:15:75,		// top, left, bottom, right
//			WindowCOLOR*2,		// color resource id and index
//			W1111,			// frame
//			AboutVIDEO, 0:1,	// video id, vertical, horizontal
//			"About"			// title
//			};
//
// Format:
//		(Word)<size>			// number of bytes in resource
//		(Rect)<loc>			// top, left, bottom, right of window
//		(short)<color_id>(Byte)<index>	// id and index of color resource
//		(Byte)<frame>			// type of window frame
//		(short)<video_id>		// id of video resource (NULL == none)
//		(Point)<video_loc>		// local vertical/horizontal of video placement
//		(char *)<title>			// window title
//
// Structure:
//		struct ResWindow {
//			Rect	loc;		// top, left, bottom, right of window
//			short	color_id;	// id of color resource
//			Byte	color_idx;	// index into color resource
//			Byte	frame;		// type of window frame
//			short	video_id;	// id of video resource (NULL == none)
//			Point	video_loc;	// local vertical/horizontal of video placement
//			char	title[1];	// dummy label to window title
//			};

Byte GenResourceWindow(void) {
	short		top, lef, frame;
	ResWindow	rWin;

	GenResourceSizeStart();

	GenRect();
	ParseMatchAdvance(L_COMMA, ",");

	GenVerifyResource(R_Color, false, true, "color");
	ParseMatchAdvance(L_COMMA, ",");

	if ((frame = LexInList(frames)) == -1)
		ErrorSevere("Expecting a window frame attribute");
	else {
		LexAdvance();
		rWin.frame = frame;
		}
	ParseMatchAdvance(L_COMMA, ",");
	write(outFileHand, &rWin.frame, sizeof(rWin.frame));

	GenVerifyResource(R_Video, true, false, "video");
	ParseMatchAdvance(L_COMMA, ",");

	if (LexMatch(L_VALUE)) {		// parse coordinates
		top = ParseValue();
		ParseMatchAdvance(L_COLON, ":");
		lef = ParseValue();
		ParseMatchAdvance(L_COMMA, ",");
		rWin.video_loc.Set(top, lef);
		}
	else					// no coordinates specified
		rWin.video_loc.Set(0, 0);
	write(outFileHand, &rWin.video_loc, sizeof(rWin.video_loc));

	GenASCIIZ(RES_WIN_LEN);

	GenResourceSizeEnd();
	return (1);
	}


// GenStart() opens up an output resource file, writes the header info,
// and notes the location of the directory and preload data pointers.

void GenStart(void) {
	char	ch;

	if (!outFileName)
		e_Internal("GenStart - must define output file name");
	if ((outFileHand = open(outFileName, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE)) == -1)
		ErrorFatal("Unable to create file '%s'", outFileName);
	if (!outMemo)
		e_Internal("GenStart - must define memo");
	write(outFileHand, outMemo, strlen(outMemo));
	ch = A_CTRLM;
	write(outFileHand, &ch, sizeof(ch));
	ch = A_CTRLJ;
	write(outFileHand, &ch, sizeof(ch));
	ch = A_CTRLZ;
	write(outFileHand, &ch, sizeof(ch));
	write(outFileHand, RES_SIG, sizeof(RES_SIG));
	outDirLocLoc = tell(outFileHand);
	write(outFileHand, &outDirLoc, sizeof(outDirLoc));
	}


// GenStop() writes out the resource directory and preload table to the
// output file, updates the reference pointers, and closes the file.

void GenStop(void) {
	outDirLoc = tell(outFileHand);
	write(outFileHand, &numNode, sizeof(numNode));
	GenBinSaveLocTraverse(root);
	lseek(outFileHand, outDirLocLoc, SEEK_SET);
	write(outFileHand, &outDirLoc, sizeof(outDirLoc));
	close(outFileHand);
	}


// GenVerifyResource() issues a diagnostic if an invalid resource is
// referenced or an out of bounds index is used.  If the id is NULL_ADR,
// no error checking is done.  If option is false, an id is required and a
// diagnostic will be issued if a NULL is seen.  If index is true, an
// optional index into the resource will be parsed if specified.

static void GenVerifyResource(short type, Boolean option, Boolean index, char *desc) {
	short	id;
	Byte	idx=0;

	if ((id = ParseValue()) == NULL_ADR && !option)
		ErrorSevere("Non-NULL %s resource id expected", desc);
	write(outFileHand, &id, sizeof(id));
	if (id != NULL_ADR && GenBinFind(type, id) == GEN_NOT_FOUND)
		ErrorSevere("Referencing undefined %s resource (%d)", desc, id);
	if (!index)
		return;
	if (id != NULL_ADR && LexMatch(L_ASTERISK)) {
		LexAdvance();
		idx = ParseValue();
		if (GenBinNumGet(type, id) < idx)
			ErrorSevere("Out of bounds index into %s resource (%u)", desc, idx);
		}
	write(outFileHand, &idx, sizeof(idx));
	}
