// default resource file extension
#define RES_OUT_EXT	".001"

enum ResType {
	R_Bar,		R_Color,	R_Coord,	R_Dialog,
	R_Error,	R_Form,		R_Help,		R_Internat,
	R_List,		R_Menu,		R_Prompt,	R_Status,
	R_String,	R_Version,	R_Video,	R_Window
	};

Boolean	ResAvail(void);
void	ResInit(char *format, ...);
void	*ResLoad(ResType type, short id);
void	ResSetPurge(ResType type, short id, Boolean purgeable);
void	ResUnLoad(ResType type, short id);

// Memory allocation routines.

#if !defined(__RESOURCE__)
#define r_malloc(size)	res_alloc((size), __FILE__, __LINE__)
#define r_free(block)	res_free((block), __FILE__, __LINE__)
#endif

void 	*res_alloc(Word size, char *file, Word line);
void	res_free(void *block, char *file, Word line);
