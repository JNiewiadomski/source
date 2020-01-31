// The String module provides routines to return pointers to strings defined
// as resources in the resource file using the Resource manager and
// information about those strings.

short	StringCount(short id);
char	*StringGet(short id, short index);
short	StringHiliteLen(char *s);
