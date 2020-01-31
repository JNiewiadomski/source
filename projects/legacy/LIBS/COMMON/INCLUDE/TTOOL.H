#ifndef _TTOOL_H
#define _TTOOL_H

#ifndef __DIR_H
#include <dir.h>
#endif

// ***************
// * TTool Class *
// ***************

// The TTool class takes care of the DOS command line, processing options,
// and providing a few more pieces of information useful to most programs.

class TTool {
	short	fCurInd;		// index of which option is being processed
	short	fCurOpt;		// current option character
	char	*fCurArg;		// pointer to argument of current option
	char	*fOptSw;		// option switch character list
	char	*fOptOff;		// turns off option parsing

	short	GetOption(void);
protected:
	enum { kErrOpt, kErrArg, kErrParam };
	enum { kExitOK, kExitSyntax };

	char	*fOptList;		// option list used by GetOption()
	short	fHelpNum;		// number of lines of help text
	const char **fHelpText;		// help text
	char	fProgName[MAXFILE];	// name of running program
	char	fProgPath[MAXPATH];	// location of running program
	char	fDataPath[MAXPATH];	// path set by DOS SET or current directory

	virtual Boolean	DoOption(char /*option*/, const char * /*argument*/) { return (False); }
	virtual int	DoWork(short /*argc*/, const char ** /*argv*/) { return (kExitOK); }
	virtual void	Help(short error_code);
public:
	TTool(const char *name);

	void	Copyright(const char *version, const char *year);
	int	Run(void);
	};

#endif
