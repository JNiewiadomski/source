#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <mem.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>
#include <common\old\keyboard.h>
#include <common\old\mouse.h>
#include <common\old\system.h>
#include <common\ttimer.h>

#include <seashell\seashell.rh>
#include <seashell\color.h>
#include <seashell\dialog.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\list.h>
#include <seashell\resource.h>
#include <seashell\status.h>
#include <seashell\string.h>
#include <seashell\video.h>
#include <seashell\window.h>

#define MAX_CHOICE	4
#define MAX_PROMPTS	10

#define LABEL_LEN	13
#define PATH_LEN	45

static short	dlgLow, dlgHigh, dlgInvLow;
static short	dlgChoiceLen;
static char	choices[MAX_CHOICE], curPath[MAXPATH], thePath[MAXPATH];


static void	DialogChangeCurPath(char *s);
static int	DialogDirCompare(const void *elem1, const void *elem2);
static short	DialogDirCount(int fattr, char *format, ...);
static void	DialogDisposeDirList(char **list, short count);
static void	DialogGetDir(int fattr, char **list, char *format, ...);
static short	DialogNewDirList(char ***list, Boolean new_opt, short filter_id);


short DialogPromptChoice(char *prompt, char *choice) {
	char	valid[MAX_PROMPTS+1], *s, vsnum, ans[2];
	short	num, len, h1, h2;
	Boolean	done;
	Rect	r;
	short	ch;

	for (num=0, s=prompt; num<MAX_PROMPTS && *s; s++)
		if (*s == '{')
			valid[num++] = *++s;
	if (num < 1)
		return (kK_ESCAPE);
	valid[num] = '\0';
	r.Set(10, 12, 14, 67);
	vsnum = VirtualNew(3, 54, W2222, r, NULL, dlgLow);
	len = HiliteStrLen(prompt);
	h1 = (r.Rig()-r.Lef()+1)/2 - (len+2)/2 - 1;
	h2 = h1 + len + 1;
	VirtualWrite2(vsnum, 1, h1, GIANT, prompt, dlgHigh, dlgInvLow);
	done = False;
	while (!done) {
		sprintf(ans, "%c", *choice);
		switch (ch = DialogStringEditor(vsnum, 1, h2, ans, 1)) {
			case kK_ESCAPE:
				done = True;
				break;
			case kK_RETURN:
				if ((s = strstr(valid, strupr(ans))) != NULL) {
					*choice = *s;
					done = True;
					}
				break;
			}
		}
	VirtualDispose(vsnum);
	return (ch);
	}


void DialogClearPrompts(short vsnum, void *dialogPtr) {
	DialogStruct	*dp;
	Rect		r;

	dp = (DialogStruct *)dialogPtr;
	while (dp) {
		r.Set(dp->top, dp->lef, dp->top, (dp->lef)+(dp->len)-1);
		VirtualFill(vsnum, r, ' ', dlgLow);
		dp = (DialogStruct *)((*dp).next);
		}
	ScreenSynch();
	}


short DialogEditor(short vsnum, void *dialogPtr) {
	DialogStruct	*dp;
	Boolean		done;
	short		ch;

	dp = (DialogStruct *)dialogPtr;
	done = False;
	while (!done) {
		ch = DialogStringEditor(vsnum, dp->top, dp->lef, dp->ans, dp->len);
		switch (ch) {
			case kK_RETURN:
				dp = (DialogStruct *)(dp->next);
				if (dp == NULL)
					done = True;
				break;
			case kK_ESCAPE:
				done = True;
				break;
			case kK_UP:
			case kK_BACKTAB:
				dp = (DialogStruct *)(dp->prev);
				break;
			case kK_DOWN:
			case kK_TAB:
				dp = (DialogStruct *)(dp->next);
				if (dp == NULL)
					dp = (DialogStruct *)dialogPtr;
				break;
			}
		}
	return (ch);
	}


void DialogInit(const char *path) {
	char	*sPtr;
	short	i;

	dlgLow = ColorGet(_DialogCOLOR, _COLOR_Low);
	dlgHigh = ColorGet(_DialogCOLOR, _COLOR_High);
	dlgInvLow = ColorGet(_DialogCOLOR, _COLOR_InvLow);
	sPtr = StringGet(_DialogSTRING, _DialogSTRING_OMDC);
	dlgChoiceLen = HiliteStrLen(sPtr);
	i = 0;
	while (i < MAX_CHOICE) {
		while (*sPtr != '{')
			sPtr++;
		sPtr++;
		choices[i++] = *sPtr++;
		}
	strcpy(curPath, path);
	}


Dialog DialogOkModifyDeleteCancel(short vsnum, short v, short h, Dialog dlg) {
	Boolean	done;
	char	ans[2];

	VirtualWrite2(vsnum, v, h, GIANT, StringGet(_DialogSTRING, _DialogSTRING_OMDC), dlgLow, dlgHigh);
	sprintf(ans, "%c", choices[dlg]);
	done = False;
	while (!done) {
		switch (DialogStringEditor(vsnum, v, h+dlgChoiceLen, ans, 1)) {
			case kK_ESCAPE:
				dlg = D_Cancel;
				done = True;
				break;
			case kK_RETURN:
				*ans = toupper(*ans);
				done = True;
				if (*ans == choices[D_Ok])
					dlg = D_Ok;
				else if (*ans == choices[D_Modify])
					dlg = D_Modify;
				else if (*ans == choices[D_Delete])
					dlg = D_Delete;
				else if (*ans == choices[D_Cancel])
					dlg = D_Cancel;
				else {
					sprintf(ans, "%c", choices[dlg]);
					done = False;
					}
				break;
			}
		}
	return (dlg);
	}


void DialogShowPrompts(short vsnum, void *dialogPtr) {
	DialogStruct	*dp;

	dp = (DialogStruct *)dialogPtr;
	while (dp) {
		VirtualWrite(vsnum, dp->top, dp->lef, GIANT, dp->ans, dlgHigh);
		dp = (DialogStruct *)(dp->next);
		}
	ScreenSynch();
	}


short DialogStringEditor(short vsnum, short v, short h, char *s, short max) {
	Rect		r;
	short		len, pos, oldLen, i;
	Boolean		done, first;
	C_Event		event;
	short		ch;

	r.Set(v, h, v, h+max-1);
	VirtualShowCursor(vsnum, True);
	len = strlen(s);
	if (len > max) {
		len = max;
		s[len] = '\0';
		}
	pos = len;
	oldLen = -1;
	first = True;
	done = False;
	while (!done) {
		if (oldLen != len) {
			VirtualFill(vsnum, r, '_', dlgHigh);
			oldLen = len;
			}
		if (first)
			VirtualWrite(vsnum, v, h, GIANT, s, dlgInvLow);
		else
			VirtualWrite(vsnum, v, h, GIANT, s, dlgHigh);
		VirtualSetCursor(vsnum, v, h+pos);
		ScreenSynch();
		while (!event.GetNext(EVT_EVERY))
			;
		if (event.What() == EVT_KEY) {
			switch (event.Message()) {
				case kK_ESCAPE:
				case kK_RETURN:
				case kK_UP:
				case kK_PGUP:
				case kK_DOWN:
				case kK_PGDN:
				case kK_TAB:
				case kK_BACKTAB:
					done = True;
					break;
				case kK_LEFT:
					if (pos > 0)
						pos--;
					break;
				case kK_RIGHT:
					if (pos < len)
						pos++;
					break;
				case kK_CLEFT:
					if (pos > 0)
						pos--;
					while (pos > 0 && s[pos-1] != ' ')
						pos--;
					break;
				case kK_CRIGHT:
					if (pos < len)
						pos++;
					while (pos < len && s[pos-1] != ' ')
						pos++;
					break;
				case kK_HOME:
					pos = 0;
					break;
				case kK_END:
					pos = len;
					break;
				case kK_BACKSPACE:
					if (pos > 0) {
						pos--;
						for (i=pos; i<len; i++)
							*(s+i) = *(s+i+1);
						len--;
						}
					break;
				case kK_DEL:
					if (first) {
						*s = '\0';
						len = pos = 0;
						}
					else if (pos<len) {
						for (i=pos; i<len; i++)
							*(s+i) = *(s+i+1);
						len--;
						}
					break;
				default:
					ch = event.Message();
					if (ch >= ' ') {
						if (first) {
							*s = '\0';
							len = pos = 0;
							}
						if (pos < max) {
							movmem(s+pos, s+pos+1, (len<max) ? len-pos+1 : max-pos-1);
							s[pos] = ch;
							pos++;
							if (len < max)
								len++;
							}
						}
					else
						done = True;
					break;
				}
			first = False;
			}
		}
	VirtualFill(vsnum, r, ' ', dlgLow);
	VirtualWrite(vsnum, v, h, GIANT, s, dlgHigh);
	VirtualShowCursor(vsnum, False);
	return (event.Message());
	}


char *DialogOpen(short filter_id, Boolean new_file) {
	short		vsnum, attr;
	Rect		r;
	Boolean		done1=False, ok, done2;
	char		volume[MAXPATH], **dirList, *s;
	struct ffblk	ff;
	ListStruct	ls;

	HelpPush(_DialogHELP_Open, StringGet(_DialogSTRING, _DialogSTRING_Open));
	StatusPush(_DialogSTATUS, _DialogSTATUS_Open);
	vsnum = VirtualGetNew(_DialogWINDOW_Open, &r);
	attr = ColorGet(_DialogCOLOR, _COLOR_High);
	while (!done1) {
		sprintf(volume, "%s*.*", curPath);
		ok = (Boolean)(findfirst(volume, &ff, FA_LABEL) != -1);
		VirtualWrite(vsnum, 0, 16, LABEL_LEN, (ok) ? ff.ff_name : " ", attr);
		VirtualWrite(vsnum, 2, 2, PATH_LEN, curPath, attr);
		if (strlen(curPath) > PATH_LEN)
			VirtualWrite(vsnum, 2, PATH_LEN-1, 3, "...", attr);
		ls.vsnum = vsnum;
		ls.count = DialogNewDirList(&dirList, new_file, filter_id);
		ls.list = dirList;
		ls.top = 4;
		ls.lef = 3;
		ls.height = 10;
		ls.width = 12;
		ls.loc = 0;
		ls.pos = 0;
		done2 = False;
		while (!done2) {
			switch (ListScroll(&ls)) {
				case kK_ESCAPE:
					ok = False;
					done2 = done1 = True;
					break;
				case kK_RETURN:
					s = dirList[ls.pos];
					switch (*s) {
						case '<':
							strcpy(thePath, curPath);
							ok = done2 = done1 = True;
							break;
						case '[':
							DialogChangeCurPath(dirList[ls.pos]);
							done2 = True;
							break;
						default:
							sprintf(thePath, "%s%s", curPath, dirList[ls.pos]);
							ok = done2 = done1 = True;
							break;
						}
					break;
				}
			}
		DialogDisposeDirList(dirList, ls.count);
		}
	VirtualDispose(vsnum);
	StatusPop();
	HelpPop();
	return (ok ? thePath : NULL);
	}


static short DialogNewDirList(char ***list, Boolean new_opt, short filter_id) {
	short	count, pos, i, num;
	char	*s;

	count = equip.drives;
	count += DialogDirCount(FA_DIREC, "%s*.*", curPath);
	for (num=StringCount(filter_id), i=0; i<num; i++)
		count += DialogDirCount(0, "%s%s", curPath, StringGet(filter_id, i));
	*list = (char **) r_malloc(sizeof(char *) * (count+2));
	pos = 0;
	if (new_opt) {
		count++;
		s = StringGet(_DialogSTRING, _DialogSTRING_NewFile);
		(*list)[pos] = (char *) r_malloc(strlen(s)+3);
		sprintf((*list)[pos++], "<%s>", s);
		}
	for (i=0; i<equip.drives; i++) {
		(*list)[pos] = (char *) r_malloc(5);
		sprintf((*list)[pos], "[%c:]", 'a'+i);
		pos++;
		}
	(*list)[pos] = NULL;
	DialogGetDir(FA_DIREC, *list, "%s*.*", curPath);
	for (num=StringCount(filter_id), i=0; i<num; i++)
		DialogGetDir(0, *list, "%s%s", curPath, StringGet(filter_id, i));
	if (new_opt)
		qsort(&(*list)[1], count-1, sizeof(char *), DialogDirCompare);
	else
		qsort(*list, count, sizeof(char *), DialogDirCompare);
	return (count);
	}


static void DialogGetDir(int fattr, char **list, char *format, ...) {
	va_list		ptr;
	char		name[MAXPATH], dir[MAXDIR];
	short		index, i;
	struct ffblk	ff;

	va_start(ptr, format);
	vsprintf(name, format, ptr);
	va_end(ptr);
	index = 0;
	while (list[index] != NULL)
		index++;
	i = findfirst(name, &ff, fattr);
	//
	// Novell networks do not display [..] directory, so take care of it
	//
	if ((i != -1) && (fattr & FA_DIREC) && (strcmp(ff.ff_name, ".") != 0)) {
		fnsplit(name, NULL, dir, NULL, NULL);
		if (strlen(dir) > 1) {
			list[index] = (char *) r_malloc(strlen("[..]")+1);
			strcpy(list[index], "[..]");
			index++;
			}
		}
	while (i != -1) {
		if ((ff.ff_attrib & ~FA_ARCH) == fattr && strcmp(ff.ff_name, ".") != 0) {
			if (fattr == FA_DIREC) {
				list[index] = (char *) r_malloc(strlen(ff.ff_name)+3);
				sprintf(list[index], "[%s]", ff.ff_name);
				}
			else {
				list[index] = (char *) r_malloc(strlen(ff.ff_name)+1);
				strcpy(list[index], ff.ff_name);
				}
			index++;
			}
		i = findnext(&ff);
		}
	list[index] = NULL;
	}


static int DialogDirCompare(const void *elem1, const void *elem2) {
	char	**s1, **s2;

	s1 = (char **)elem1;
	s2 = (char **)elem2;
	return (strcmp(*s1, *s2));
	}


static void DialogChangeCurPath(char *s) {
	char	*p;

	if (*(s+1) == '.') {
		p = curPath+strlen(curPath)-1;
		while (*(p-1) != '\\')
			p--;
		*p = '\0';
		}
	else if (*(s+2) == ':')
		sprintf(curPath, "%c:\\", 'A'+*(s+1)-'a');
	else {
		strcat(curPath, s+1);
		*(curPath+strlen(curPath)-1) = '\\';
		}
	}


static short DialogDirCount(int fattr, char *format, ...) {
	va_list		ptr;
	char		name[MAXPATH], dir[MAXDIR];
	struct ffblk	ff;
	short		count=0, i;

	va_start(ptr, format);
	vsprintf(name, format, ptr);
	va_end(ptr);
	i = findfirst(name, &ff, fattr);
	//
	// Novell networks do not display [..] directory, so take care of it
	//
	if ((i != -1) && (fattr & FA_DIREC) && (strcmp(ff.ff_name, ".") != 0)) {
		fnsplit(name, NULL, dir, NULL, NULL);
		if (strlen(dir) > 1)
			count++;
		}
	while (i != -1) {
		if ((ff.ff_attrib & ~FA_ARCH) == fattr && strcmp(ff.ff_name, ".") != 0)
			count++;
		i = findnext(&ff);
		}
	return (count);
	}


static void DialogDisposeDirList(char **list, short count) {
	short	i;

	for (i=0; i<count; i++)
		r_free(list[i]);
	r_free(list);
	}


char *DialogSave(short filter_id, char *save_name, char *prompt) {
	short		vsnum, attr, handle;
	Rect		r;
	Boolean		done1=False, saveMode=True, ok, done2;
	struct ffblk	ff;
	char		volume[MAXPATH], **dirList, name[MAXPATH], ext[MAXEXT];
	char		ans[2], yes='Y', no='N', *s;
	ListStruct	ls;

	HelpPush(_DialogHELP_Save, StringGet(_DialogSTRING, _DialogSTRING_Save));
	StatusPush(_DialogSTATUS, _DialogSTATUS_Save);
	vsnum = VirtualGetNew(_DialogWINDOW_Save, &r);
	attr = ColorGet(_DialogCOLOR, _COLOR_High);
	fnsplit(save_name, NULL, NULL, name, ext);
	strcat(name, ext);
	while (!done1) {
		sprintf(volume, "%s*.*", curPath);
		ok = (Boolean)(findfirst(volume, &ff, FA_LABEL) != -1);
		VirtualWrite(vsnum, 0, 16, LABEL_LEN, (ok) ? ff.ff_name : " ", attr);
		VirtualWrite(vsnum, 2, 2, PATH_LEN, curPath, attr);
		if (strlen(curPath) > PATH_LEN)
			VirtualWrite(vsnum, 2, PATH_LEN-1, 3, "...", attr);
		VirtualWrite(vsnum, 11, 2, GIANT, prompt, attr);
		ls.vsnum = vsnum;
		ls.count = DialogNewDirList(&dirList, False, filter_id);
		ls.list = dirList;
		ls.top = 4;
		ls.lef = 3;
		ls.height = 5;
		ls.width = 12;
		ls.loc = 0;
		ls.pos = 0;
		ListDisplay(&ls);
		done2 = False;
		while (!done2) {
			if (saveMode) {
				switch (DialogStringEditor(vsnum, 13, 4, name, 12)) {
					case kK_RETURN:
						sprintf(thePath, "%s%s", curPath, name);
						ok = done2 = done1 = True;
						if ((handle = open(thePath, O_RDONLY)) != -1) {
							close(handle);
							ErrorSound();
							s = StringGet(_DialogSTRING, _DialogSTRING_Overwrite);
							while (*s && *s++ != '{')
								;
							if (*s)
								yes = *s;
							while (*s && *s++ != '{')
								;
							if (*s)
								no = *s;
							sprintf(ans, "%c", yes);
							if (DialogPromptChoice(StringGet(_DialogSTRING, _DialogSTRING_Overwrite), ans) == kK_ESCAPE)
								*ans = 'N';
							if (toupper(*ans) == no)
								done2 = done1 = False;
							}
						break;
					case kK_TAB:
					case kK_BACKTAB:
						saveMode = False;
						break;
					case kK_ESCAPE:
						done1 = done2 = True;
						ok = False;
						break;
					}
				}
			else {
				switch (ListScroll(&ls)) {
					case kK_ESCAPE:
						done2 = done1 = True;
						ok = False;
						break;
					case kK_RETURN:
						switch (*dirList[ls.pos]) {
							case '[':
								DialogChangeCurPath(dirList[ls.pos]);
								done2 = True;
								break;
							default:
								fnsplit(dirList[ls.pos], NULL, NULL, name, NULL);
								saveMode = True;
							}
						break;
					case kK_TAB:
					case kK_BACKTAB:
						saveMode = True;
						break;
					}
				}
			}
		DialogDisposeDirList(dirList, ls.count);
		}
	VirtualDispose(vsnum);
	StatusPop();
	HelpPop();
	return (ok ? thePath : NULL);
	}
