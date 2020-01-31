// ***********************************************************************
// *                                                                     *
// *                      PCS100 Network Supervisor                      *
// *                                                                     *
// *       Copyright (c) 1987-1991  Vorne Industries Incorporated.       *
// *                         All rights reserved.                        *
// *                                                                     *
// ***********************************************************************

// If the symbol DEMO is #defined, the PCS100 software will only allow 1
// machine to be installed.

#define DEMO			"DEMO"

// signiatures
#define PCS100_SIG		"PCS100 by Vorne Industries Incorporated\x1A"

struct PrefsRecord {
	DWord	autoSave;		// delay between autosaves
	DWord	screenSave;		// delay before screen saver kicks in
	Byte	exportField;		// export field delimeter
	Byte	exportRecord;		// export record delimeter
	DWord	netPubSub;		// publish/subscribe timing
	DWord	netSubRetry;		// subscribe retry timing
	};

struct QueryRecord {
	Word	bell		: 1;	// sound bell on errors?
	Word	err		: 1;	// should retrieveQuery post errors?
	Word	arch		: 1;	// should we retrieve archival information?
	Word	off		: 1;	// should we display offline trace?
	Word	enable		: 1;	// querying enabled?
	Word	logPrint	: 1;	// log errors to printer?
	int	port;
	DWord	minDelay;		// minimum delay for query response
	DWord	maxDelay;		// maximum delay for query response
	DWord	hiPri;			// high priority delay
	DWord	loPri;			// low priority delay
	DWord	retries;		// maximum delay of offline tolerance
	Byte	eStat;			// statistical error tolerance
	};

struct CommRecord {
	Byte	printer;
	int	port;			// serial printer port
	int	baud;			// serial printer Baud rate
	int	parity;			// serial printer parity
	int	data;			// serial printer data bits
	int	stop;			// serial printer stop bits
	};

struct DataStruct {
	char		rExt[5];	// extension of language resource file
	PrefsRecord	prefs;
	QueryRecord	query;
	CommRecord	comm;
	short		cSum;		// verify data integrity
	};

extern Boolean		doneFlag, restart, host;
extern DataStruct	data;
extern C_Timer		nextAutoSave;
