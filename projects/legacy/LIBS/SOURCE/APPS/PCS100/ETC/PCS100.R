// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

resource bar (PCS100_BAR, preload, purgeable) {
	0:0:0:79,			// menu bar location
	HelpMENU,			// menu bar list...
	MachineMENU,
	JobMENU,
	ShiftMENU,
	GraphMENU,
	ReportMENU
	};

resource color (WindowCOLOR, preload) {
	cyan : low,
	white : high
	};

resource color (DialogCOLOR, preload) {
	magenta : low,
	white : high
	};

resource color (ScreenCOLOR, preload) {
	brown : low,
	yellow : high,
	brown*yellow : inverse
	};

resource color (DOS_COLOR, preload) {
	lightgray : low
	};

resource window (ScreenWINDOW, purgeable) {
	1:0:23:79,
	WindowCOLOR*_COLOR_Low,
	w1111,
	NULL,
	""
	};

resource window (DOS_WINDOW, purgeable) {
	0:0:24:79,
	DOS_COLOR*_COLOR_Low,
	wnone,
	NULL,
	""
	};
