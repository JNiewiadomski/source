#define GIANT	9999

enum Dir_Type { Dir_Up, Dir_Down, Dir_Left, Dir_Right };
enum Win_Type { WNONE, W0000, W1111, W2222, W2111 };

struct CELL {
	char	ch;		// character
	char	attr;		// attribute
	};

extern Rect	Giant_Rect;
