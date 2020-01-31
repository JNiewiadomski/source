/* dialog.h -- January 11, 1990 */

/* Dialog Library */

typedef void (*dialogFunc)(short ch);

void  DialogClear(void);
void  DialogDo(short ch);
char *DialogGetVal(void);
void  DialogStartCode(short row, short col, short len, dialogFunc func, WORD lo, WORD hi);
void  DialogStartList(short row, short col, dialogFunc func, WORD dflt, short num, char **list);
void  DialogStartNum(short row, short col, short len, dialogFunc func, WORD dflt, WORD lo, WORD hi);
void  DialogStop(void);
