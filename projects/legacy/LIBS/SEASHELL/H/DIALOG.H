enum Dialog { D_Ok, D_Modify, D_Delete, D_Cancel };

struct DialogStruct {
	Byte	len;
	Byte	top;
	Byte	lef;
	char	*ans;
	void	*prev;
	void	*next;
	};

void	DialogClearPrompts(short vsnum, void *dialogPtr);
short	DialogEditor(short vsnum, void *dialogPtr);
void	DialogInit(const char *path);
Dialog	DialogOkModifyDeleteCancel(short vsnum, short v, short h, Dialog dlg);
short	DialogPromptChoice(char *prompt, char *choice);
void	DialogShowPrompts(short vsnum, void *dialogPtr);
short	DialogStringEditor(short vsnum, short v, short h, char *s, short max);
char	*DialogOpen(short filter_id, Boolean new_file);
char	*DialogSave(short filter_id, char *name, char *prompt);
