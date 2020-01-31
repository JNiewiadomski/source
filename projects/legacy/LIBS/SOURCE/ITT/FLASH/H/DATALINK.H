/* datalink.h -- April 2, 1990 */

/* Data Link Layer of Serial Communication */

#define DLINK_NONE     0
#define DLINK_ERROR   -1
#define DLINK_ACK     -2

#define ERR_RESET         0xC0     /* reset done */
#define ERR_INDEX         0xC1     /* index error */
#define ERR_FLASH         0xC2     /* flash failed */
#define ERR_DONE          0xC8     /* instruction done */

#define PACK_STX          0x00
#define PACK_LEN          0x01
#define PACK_NODE         0x02
#define PACK_IDX          0x03
#define PACK_DATA         0x04

#define DLINK_BUF_LEN 255

void    DataLinkCleanUp(void);
Boolean DataLinkInit(short port);
short   DataLinkReceive(char *data);
void    DataLinkReset(void);
void    DataLinkTransmit(short len, char *data);
