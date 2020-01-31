struct VideoRecord {
	Byte	mode;
	Byte	rows;
	Byte	cols;
	Word	size;
	Boolean	colorFlag;
	Word	dispOff;
	Word	dispSeg;
	};

extern VideoRecord	video;

void VideoGetCursor(short *row, short *col);
void VideoGetCursorType(short *start, short *end);
void VideoGetInfo(void);
void VideoSetCursor(short row, short col);
void VideoSetCursorType(short start, short end);
void VideoSetMode(Byte mode);
void VideoShowCursor(Boolean on);
