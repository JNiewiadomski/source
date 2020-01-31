/* about.c -- March 8, 1990 */

/* About Dialog */

#include <graphics.h>
#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>
#include <..\seachest\graphics.h>
#include <..\seachest\mouse.h>

#include "about.h"

#define ABOUT_A  "ITT v0.1d19"
#define ABOUT_B  "Technologic Pump Controller"
#define ABOUT_C  "by"
#define ABOUT_D  "Vorne Industries Incorporated"
#define ABOUT_E  ""
#define ABOUT_F  "March 8, 1990"


void DoAbout(void)
  {
    GrafPort    *old, new;
    short       height, width, top, lef, center;
    Rect        r;
    char        *buff;
    EventRecord myEvent;

    old = GraphicsGetPort();
    GraphicsNewPort(&new);
    GraphicsForeColor(YELLOW);
    GraphicsFillColor(BLUE);
    GraphicsTextSize(6);
    GraphicsTextJustify(CENTER_TEXT, TOP_TEXT);
    height = (textheight("!y")+4) * 7;
    width = textwidth(ABOUT_D) + 10;
    center = getmaxx() / 2;
    top = (getmaxy()/2) - (height / 2);
    lef = center - (width / 2);
    RASG(&r, top, lef, top+height, lef+width);
    if ((buff = (char *) malloc(RSIZE(&r))) != NULL) {
      mouseHideCursor();
      getimage(r.p.lef, r.p.top, r.p.rig, r.p.bot, buff);
      bar3d(r.p.lef, r.p.top, r.p.rig, r.p.bot, 0, 0);
      height = textheight("!y") + 4;
      top = r.p.top + 8;
      outtextxy(center, top, ABOUT_A);
      top += height;
      outtextxy(center, top, ABOUT_B);
      top += height;
      outtextxy(center, top, ABOUT_C);
      top += height;
      outtextxy(center, top, ABOUT_D);
      top += height;
      outtextxy(center, top, ABOUT_E);
      top += height;
      outtextxy(center, top, ABOUT_F);
      mouseShowCursor();
      while (!EventAvail(EVT_EVERY, &myEvent))
        ;
      mouseHideCursor();
      putimage(r.p.lef, r.p.top, buff, COPY_PUT);
      mouseShowCursor();
      free(buff);
      }
    GraphicsSetPort(old);
    EventFlush(EVT_EVERY, EVT_NULL);
    }
