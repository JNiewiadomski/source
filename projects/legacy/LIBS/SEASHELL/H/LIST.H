struct ListStruct {
	short	vsnum;
	char	**list;
	short	top;
	short	lef;
	short	height;
	short	width;
	short	count;
	short	loc;
	short	pos;
	};

void	ListDisplay(ListStruct *ls);
void	ListInit(void);
short	ListScroll(ListStruct *ls);
