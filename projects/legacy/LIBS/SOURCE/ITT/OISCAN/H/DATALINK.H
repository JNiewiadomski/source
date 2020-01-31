// Data Link Layer of Serial Communication

#define DLINK_NONE	0
#define DLINK_ERROR	-1

#define DLINK_BUF_LEN	255

void	DataLinkCleanUp(void);
Boolean	DataLinkInit(int port, int baud, Byte node);
short	DataLinkReceive(void);
