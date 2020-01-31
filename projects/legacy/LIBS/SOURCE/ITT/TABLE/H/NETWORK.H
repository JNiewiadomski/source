/* network.h -- December 12, 1989 */

/* Network Layer for Serial Communication */

/* ----------------------------------------------------------------------- *
 * MODIFICATIONS
 *  Jack Niewiadomski   12-DEC-1989
 * ----------------------------------------------------------------------- */

typedef enum { NET_WRITE, NET_READ, NET_NONE } NET_CMD;

typedef struct {
  BYTE len;
  WORD adr;
  char data[DLINK_BUF_LEN];
  } NetworkRecord;

void    NetworkCleanUp(void);
void    NetworkInit(short port);
NET_CMD NetworkReceive(NetworkRecord *packet);
void    NetworkReOpen(void);
void    NetworkRespond(NET_CMD cmd, NetworkRecord *packet);
