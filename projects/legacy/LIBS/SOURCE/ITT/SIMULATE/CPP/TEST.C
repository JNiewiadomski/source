/* test.c -- February 5, 1990 */

/* Test Library */

#include <stdlib.h>

#include <..\seachest\seachest.h>
#include <..\seachest\event.h>

#include "led.h"
#include "button.h"
#include "dialog.h"
#include "display.h"
#include "state.h"
#include "test.h"

enum { TEST_LAMP, TEST_DISPLAY, TEST_NUM };

static char  *testList[] = { "Lamp Test",
                             "Display Test"
                             };
static char  *testHelp[] = { "Test of LED's and all display segments               ",
                             "Test for display readability               "
                             };
static short testPos;


static void TestMenuChoice(void);


void TestClear(void)
  {
    ShiftStatePop();
    DisplayPop();
    }


void TestDown(void)
  {
    testPos = testPos == 0 ? TEST_NUM-1 : testPos-1;
    TestMenuChoice();
    }


void TestLEDs(void)
  {
    EventRecord event;
    JIFF        timer;

    DisplayFill(DISP_ALL_ON);
    LEDSetAll(L_RED);
    LEDDrawAll();
    timer = EventJiffy() + 200L;
    while (!EventElapsed(timer) && !EventAvail(EVT_KEY | EVT_MOUSE_DOWN, &event))
      ;
    LEDSetAll(L_GREEN);
    LEDDrawAll();
    timer = EventJiffy() + 200L;
    while (!EventElapsed(timer) && !EventAvail(EVT_KEY | EVT_MOUSE_DOWN, &event))
      ;
    LEDSetAll(L_OFF);
    LEDDrawAll();
    EventFlush(EVT_EVERY, EVT_NULL);
    }


void TestMenu(void)
  {
    DisplayPush();
    ShiftStatePush(S_TEST);
    testPos = 0;
    TestMenuChoice();
    }


static void TestMenuChoice(void)
  {
    DisplayClear();
    DisplaySetLine(DISP_LINE1, 0, testList[testPos]);
    DisplayScrollLineOn(DISP_LINE2, testHelp[testPos]);
    }


void TestSelect(void)
  {
    EventDelay(10L);
    ButtonUp();
    switch (testPos) {
      case TEST_LAMP:
        TestLEDs();
        break;
      case TEST_DISPLAY:
        DisplayTestSegments();
        break;
      }
    TestClear();
    }


void TestUp(void)
  {
    testPos = (testPos + 1) % TEST_NUM;
    TestMenuChoice();
    }
