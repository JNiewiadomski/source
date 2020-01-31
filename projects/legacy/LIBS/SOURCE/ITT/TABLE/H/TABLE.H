/* table.h -- January 9, 1990 */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   09-JAN-1990
 * ----------------------------------------------------------------------- */

#define S_RXLEN   MIN_BUF_SIZE
#define S_TXLEN   256
#define S_MODE    (ASOUT | BINARY | NORMALRX)
#define S_BAUD    300
#define S_PARITY  P_NONE
#define S_DATA    8
#define S_STOP    1

extern short       ErrorMenuID, HelpMenuID, SystemMenuID;
extern EquipStruct equip;
extern char        *progName;
