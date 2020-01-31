// The Help Manager handles the process of displaying multiple pages of
// help text found in the programs resource file.

void	HelpGet(void);
int	HelpGetKey(void);
Boolean	HelpGetPeek(void);
void	HelpInit(int key);
void	HelpPop(void);
void	HelpPush(Word num, char *title);
void	HelpSet(Word num, char *title);
void	HelpSetKey(int key);
void	HelpSetPeek(Boolean on);
