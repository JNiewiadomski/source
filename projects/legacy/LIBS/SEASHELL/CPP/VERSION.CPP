#include <common\common.h>
#include <common\old\coord.h>

#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\version.h>

// VersionGet() returns a pointer to the requested version data structure
// from the current resource file.  If id does not reference a version
// resource, a NULL pointer is returned.

void *VersionGet(short id) {
	ResVersion	*r_ver;
	register char	*s;

	if ((r_ver = (ResVersion *) ResLoad(R_Version, id)) == NULL)
		return (NULL);
	s = r_ver->data;
	r_ver->ver_str = s;
	while (*s++)
		;
	r_ver->country = s;
	while (*s++)
		;
	r_ver->date = s;
	return (r_ver);
	}
