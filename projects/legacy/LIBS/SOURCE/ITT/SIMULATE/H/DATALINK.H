/* datalink.h -- November 30, 1989 */

/* Data Link Layer of Serial Communication */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   30-NOV-1989
 * ----------------------------------------------------------------------- */

#define DLINK_NONE     0
#define DLINK_ERROR   -1

#define DLINK_BUF_LEN 255

void    DataLinkAck(void);
void    DataLinkCleanUp(void);
BOOLEAN DataLinkInit(int port, int baud, BYTE node);
short   DataLinkReceive(char *data);
void    DataLinkTransmit(short len, char *data);
