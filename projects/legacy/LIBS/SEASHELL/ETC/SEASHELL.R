// SeaShell system resource file common to all languages.

//	<color>:
//		black		blue		green		cyan
//		red		magenta		brown		lightgray
//		darkdray	lightblue	lightgreen	lightcyan
//		lightred	lightmagenta	yellow		white
//	<mono>:
//		line		low		highline	high
//		inverse		highinverse

resource color (_ErrorCOLOR, preload, purgeable) {
	red : low,			// low
	white : high,			// high
	red*white : inverse		// inverse
	};

resource color (_EventCOLOR, preload, purgeable) {
	green : low,			// low
	white : high			// high
	};

resource color (_StatusCOLOR, preload, purgeable) {
	cyan : low,
	white : high,
	cyan*black : inverse,
	cyan*white : highinverse,
	lightcyan : highline
	};

resource color (_PrinterCOLOR, preload, purgeable) {
	green : low,
	white : high
	};

resource color (_HelpCOLOR, preload, purgeable) {
	cyan : low,
	white : high,
	cyan*black : inverse,
	cyan*white : highinverse,
	lightcyan : highline
	};

resource color (_MenuCOLOR, preload, purgeable) {
	cyan : low,
	white : high,
	cyan*black : inverse,
	cyan*white : highinverse,
	lightcyan : highline
	};

resource color (_DialogCOLOR, preload, purgeable) {
	magenta : low,
	white : high,
	magenta*white : inverse
	};

resource window (_ErrorWINDOW) {
	9:12:15:67,			// top, left, bottom, right
	_ErrorCOLOR*_COLOR_Low,		// color
	w1111,				// frame
	NULL,				// video id, top, left
	""				// title
	};

resource window (_StatusWINDOW) {
	24:0:24:79,			// top, left, bottom, right
	_StatusCOLOR*_COLOR_InvLow,	// color
	wnone,				// frame
	NULL,				// video id, top, left
	""				// title
	};

resource window (_PrinterWINDOW) {
	10:22:14:58,			// top, left, bottom, right
	_PrinterCOLOR*_COLOR_Low,	// color
	w2222,				// frame
	NULL,				// video id, top, left
	""				// title
	};

resource window (_HelpWINDOW) {
	0:0:23:79,			// top, left, bottom, right
	_HelpCOLOR*_COLOR_Low,		// color
	w2222,				// frame
	NULL,				// video id, top, left
	""				// title
	};

resource window (_HelpPeekWINDOW) {
	19:0:23:79,			// top, left, bottom, right
	_HelpCOLOR*_COLOR_Low,		// color
	w2222,				// frame
	NULL,				// video id, top, left
	""				// title
	};
