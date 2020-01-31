// *********************
// * Coordinate System *
// *********************

#include <common\common.h>
#include <common\old\coord.h>


// Intersect() intersects the source rectangle src with the rectangle
// extending from (0, 0) to (maxrow, maxcol).  The resulting rectangle is
// assigned to the current rectangle.  The values true or false are returned
// depending on whether the intersection is empty.
//		maxrow  -  max row of clipping rectangle
//		maxcol  -  max column of clipping rectangle

Boolean Rect::Intersect(Rect &src, short maxrow, short maxcol) {
	top = max(0, src.Top());
	lef = max(0, src.Lef());
	bot = min(maxrow, src.Bot());
	rig = min(maxcol, src.Rig());
	return ((Boolean)(top <= bot && lef <= rig));
	}


// Union() sets the current rectangle to the smallest rectangle that
// completely encloses the rectangles src1 and src2.

void Rect::Union(Rect &src1, Rect &src2) {
	top = min(src1.Top(), src2.Top());
	lef = min(src1.Lef(), src2.Lef());
	bot = max(src1.Bot(), src2.Bot());
	rig = max(src1.Rig(), src2.Rig());
	}
