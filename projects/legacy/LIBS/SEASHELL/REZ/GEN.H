#define GEN_NOT_FOUND	-1

long	GenBinFind(short type, short id);
void	GenBinInsert(short type, short id, Boolean preload, Boolean purgeable);
Byte	GenBinNumGet(short type, short id);
void	GenBinNumSet(short type, short id, Byte num);
void	GenDelete(void);
void	GenOutFile(char *name);
void	GenOutMemo(char *memo);
void	GenOutPath(char *path);
void	GenRead(void);
Byte	GenResourceBar(void);
Byte	GenResourceColor(void);
Byte	GenResourceCoord(void);
Byte	GenResourceDialog(void);
Byte	GenResourceError(void);
Byte	GenResourceForm(void);
Byte	GenResourceHelp(void);
Byte	GenResourceInternat(void);
Byte	GenResourceList(void);
Byte	GenResourceMenu(void);
Byte	GenResourcePrompt(void);
Byte	GenResourceStatus(void);
Byte	GenResourceString(void);
Byte	GenResourceVersion(void);
Byte	GenResourceVideo(void);
Byte	GenResourceWindow(void);
void	GenStart(void);
void	GenStop(void);
