// The Color module provides routines to return color attributes defined in
// the resource file using the Resource manager.  The Video managers data
// structures provide the needed information to determine whether or not
// we are currently running on a system that supports color.

#include <common\common.h>
#include <common\old\colors.h>
#include <common\old\coord.h>

#include <seashell\color.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>
#include <seashell\video.h>


// ColorGet() returns the color attribute requested from the resource file.
// The appropriate color/mono attribute is returned based on the type of
// system we are running on.
//
// NOTE: VideoGetInfo() must be called to initialize the video structures.

Byte ColorGet(short id, short index) {
	ResColor	*r_color;
	Attrib		*r_attrib;

	if ((r_color = (ResColor *) ResLoad(R_Color, id)) == NULL)
		return (WHT);
	if (index >= r_color->num)
		return (WHT);
	for (r_attrib=r_color->attr; index; r_attrib++, index--)
		;
	return (video.colorFlag ? r_attrib->color : r_attrib->mono);
	}
