#include <alloc.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <seashell\seashell.h>
#include <seashell\clock.h>
#include <seashell\dialog.h>
#include <seashell\error.h>
#include <seashell\event.h>
#include <seashell\help.h>
#include <seashell\iu.h>
#include <seashell\menu.h>
#include <seashell\message.h>
#include <seashell\printer.h>
#include <seashell\resource.h>
#include <seashell\window.h>

#include <common\misc.h>

#include "table.sym"
#include "table.h"
#include "hardware.h"

#define PROGRAM_NAME  "TABLE"

#define AboutITEM    1
#define HelpITEM     3
#define QuitITEM     5

short         ErrorMenuID, HelpMenuID, SystemMenuID;
EquipStruct   equip;
char          *progName;

static BOOLEAN       doneFlag;
static VersionRecord tableVer;


static void    doCleanUp(void);
static void    doCommand(short menuID, short item);
static void    doHelpMenu(short item);
static void    doMainLoop(void);
static BOOLEAN initProgName(void);
static void    installMenus(void);
static void    systemTask(void);
static void    updateMenus(void);


main()
  {
    E_VAL err;

    if (!initProgName()) {
      printf("Insufficient memory to run program %s.\n\a", PROGRAM_NAME);
      return (EXIT_FAILURE);
      }
    if ((err = ResInit(progName)) != ERR_RES_OK) {
      printf("%s.\n\a", ResErrDesc(err));
      return (EXIT_FAILURE);
      }
    if (VirtualInit() != ERR_WIN_OK) {
      printf(ResGetErr(tableWinNoMemERROR, FALSE));
      ResCleanUp();
      return (EXIT_FAILURE);
      }
    ErrorInit(doCleanUp);
    EventInit(systemTask);
    PrinterInit(K_CTRLP);
    MessageInit();
    HelpInit(K_CTRLH);
    MenuInit();
    DialogInit();
    IUInit();
    ClockInit();
    ResGetVer(TABLE_Version, &tableVer);
    equipCheck(&equip);
    HardwareInit(1);
    MessagePush(progName);
    MenuPushBar(0, 0, SCREEN_WIDTH-1);
    installMenus();
    HelpPush(HelpItemAbout, PROGRAM_NAME);
    doMainLoop();
    HelpPop();
    doCleanUp();
    return (EXIT_SUCCESS);
    }


static void doCleanUp(void)
  {
    HardwareCleanUp();
    ClockCleanUp();
    EventCleanUp();
    VirtualCleanUp();
    ResCleanUp();
    }


static void doCommand(short menuID, short item)
  {
    if (menuID == ErrorMenuID)
      MenuDoError(item);
    else if (menuID == HelpMenuID)
      doHelpMenu(item);
    else if (menuID == SystemMenuID)
      HardwareDoMenu(item);
    }


static void doHelpMenu(short item)
  {
    char          msgStr[80];
    short         vsnum, attr, height, width;

    switch (item) {
      case AboutITEM:
        vsnum = VirtualGetNew(AboutWINDOW, &height, &width, &attr);
        HelpPush(HelpItemAbout, MenuGetItem(HelpMenuID, AboutITEM));
        VirtualPrint(vsnum, 0, 1, GIANT, attr, "%s %s", PROGRAM_NAME, tableVer.verStr);
        VirtualWrite(vsnum, 0, width-strlen(tableVer.language)-1, GIANT, tableVer.language, attr);
        VirtualWrite(vsnum, 4, 1, GIANT, tableVer.date, attr);
        sprintf(msgStr, ResGetStr(_errFreeSTRING), coreleft());
        VirtualWrite(vsnum, 4, (width/2)-(strlen(msgStr)/2), GIANT, msgStr, attr);
        MessagePush(ResGetStr(_gPressAnyKeySTRING));
        EventWaitKey();
        MessagePop();
        HelpPop();
        VirtualDispose(vsnum);
        break;
      case HelpITEM:
        HelpGet();
        break;
      case QuitITEM:
        doneFlag = TRUE;
        break;
      }
    }


static void doMainLoop(void)
  {
    int         ch;
    short       menuID, item;
    EventRecord myEvent;

    doneFlag = FALSE;
    while (!doneFlag) {
      while (!EventGetNext(EVT_EVERY, &myEvent))
        ;
      switch (myEvent.what) {
        case EVT_KEY:
          menuID = NO_MENU;
          item = 0;
          ch = myEvent.message;
          if (MenuIsAlt(ch)) {
            updateMenus();
            MenuSelect(ch, &menuID, &item);
            }
          else if (MenuIsCtrl(ch) || MenuIsFunc(ch)) {
            updateMenus();
            MenuKey(ch, &menuID, &item);
            }
          else if ((ch>='A' && ch<='Z') || (ch>='a' && ch<='z')) {
            updateMenus();
            MenuSelect(toupper(ch), &menuID, &item);
            }
          else
            HardwareProcessChar(ch);
          if (menuID != NO_MENU) {
            doCommand(menuID, item);
            MenuHilite(NO_MENU);
            }
          break;
        }
      }
    }


static BOOLEAN initProgName(void)
  {
    char pPath[MAXPATH], drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE];

    if (_osmajor < 3) {
      getcwd(pPath, MAXPATH-1);
      if (pPath[strlen(pPath)-1] != '\\')
        strncat(pPath, "\\", MAXPATH-1);
      strncat(pPath, PROGRAM_NAME, MAXPATH-1);
      }
    else {
      fnsplit(parsePath(*_argv), drive, dir, name, NULL);
      fnmerge(pPath, drive, dir, name, "");
      }
    if ((progName = (char *) malloc(strlen(pPath)+1)) == NULL)
      return (FALSE);
    strcpy(progName, pPath);
    return (TRUE);
    }


static void installMenus(void)
  {
    ErrorMenuID = MenuInstallError();
    if (equip.serial < 1)
      MenuPostError("Communications Disabled.", TRUE);
    HelpMenuID = MenuGet(HelpMENU);
    MenuInsert(HelpMenuID, 0);
    SystemMenuID = MenuGet(SystemMENU);
    MenuInsert(SystemMenuID, 0);
    MenuDrawBar();
    MessageSet("{^H}-Help  {^P}-Print  {^Q}-Quit  {<Alt><1st letter>}-Open Menu");
    }


static void systemTask(void)
  {
    HardwareCommunicate();
    if (clockChanged) {
      MessagePrint(SCREEN_WIDTH-6, clockStr);
      clockChanged = FALSE;
      }
    ScreenSynch();
    }


static void updateMenus(void)
  {
    HardwareUpdateMenu();
    }
