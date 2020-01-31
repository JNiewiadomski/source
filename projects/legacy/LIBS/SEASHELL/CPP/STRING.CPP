// The String module provides routines to return pointers to strings defined
// as resources in the resource file using the Resource manager.

#include <string.h>

#include <common\common.h>
#include <common\old\coord.h>

#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\string.h>


// StringCount returns the number of strings defined for the specified id.
// If resource id does not exist, 0 is returned.

short StringCount(short id) {
	ResString	*str;

	str = (ResString *) ResLoad(R_String, id);
	return (str ? str->num : 0);
	}


// StringGet() returns a pointer to the requested string from the current
// resource file.  If id or index do not reference a string, a pointer to
// the string "undefined" is returned.
//
// CAUTION: all pointers reference unique strings in memory, so no
// contentions should occur when referencing multiple strings.  However,
// the pointer may be invalidated whenever the resource manager needs to
// read in a new resource and memory needs to be purged.  If a string
// resource is marked as purgeable, always use StringGet() as a pointer to
// the resource.  If the resource is not marked as purgeable, it is correct
// programming practice to assign the pointer returned to a character
// pointer and reference the string by the character pointer.

char *StringGet(short id, short index) {
	ResString	*str;
	register char	*s;

	str = (ResString *) ResLoad(R_String, id);
	if (str && index < str->num) {
		s = str->string;
		while (index--)
			s = strchr(s, '\0') + 1;	// scan for next string
		}
	else
		s = "undefined";
	return (s);
	}


// StringHiliteLen() returns the length of a string using the dual attribute
// encoding scheme ('{' for begin highlight and '}' for end highlight).

short StringHiliteLen(char *s) {
	register short	i;

	i = 0;
	while (*s) {
		if (*s!='{' && *s!='}')
			i++;
		s++;
		}
	return (i);
	}
