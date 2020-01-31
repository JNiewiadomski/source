/* text.c -- February 5, 1989 */

/* Text Object Library */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   06-DEC-1989
 * ----------------------------------------------------------------------- */

#include <graphics.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\graphics.h>

#include "text.h"

#define MAX_TEXT   50

typedef struct {
  Point pos;
  short color;
  short size;
  char  *text;
  } TEXT_OBJECT;

static TEXT_OBJECT   *listText[MAX_TEXT];
static short         numText=0;


short TextCreate(Point *pos, short color, short size, char *text)
  {
    TEXT_OBJECT *new;

    if (numText >= MAX_TEXT)
      return (TEXT_BAD);
    if ((new = (TEXT_OBJECT *) malloc(sizeof(TEXT_OBJECT))) == NULL)
      return (TEXT_BAD);
    PCPY(&(new->pos), pos);
    new->color = color;
    new->size = size;
    new->text = text;
    listText[numText] = new;
    return (numText++);
    }


void TextDraw(short index)
  {
    TEXT_OBJECT *text;
    char        *s, *t, buff[255];
    int         top, h;

    if (index < 0 || index >= numText)
      return;
    text = listText[index];
    setcolor(text->color);
    settextstyle(G_FONT, HORIZ_DIR, text->size);
    h = textheight("!y") + 1;
    t = text->text;
    top = text->pos.v;
    while (*t) {
      s = buff;
      while (*t && *t != 0x0D)
        *s++ = *t++;
      if (*t)
        t++;
      *s = '\0';
      outtextxy(text->pos.h, top, buff);
      top += h;
      }
    }
