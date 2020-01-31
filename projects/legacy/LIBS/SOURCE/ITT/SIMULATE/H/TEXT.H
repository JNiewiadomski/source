/* text.h -- December 6, 1989 */

/* Text Object Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   06-DEC-1989
 * ----------------------------------------------------------------------- */


#define TEXT_BAD  -1

short TextCreate(Point *pos, short color, short size, char *text);
void  TextDraw(short index);

