// ***************
// * TTool Class *
// ***************

#include <dos.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\ttool.h>

// The TTool class implements the user interface to take care of the DOS
// command line and providing a path to the application and data files.

// *************
// * Constants *
// *************

const short	kOptDone=-1;	// returned by GetOption() when finished
const short	kOptErr='?';	// returned by GetOption() when erroneous option encountered

const char	kVersion[]	= "Version";
const char	kCopyright[]	= "Copyright (c)";
const char	kCompany[]	= "Vorne Industries Incorporated";

// ***********************
// * TTool Class Methods *
// ***********************

// The TTool constructor sets fProgPath to the location of the running
// program and fProgName to the name of the running program.  If the current
// version of DOS is less then 3, then fProgPath is set to the current
// directory and progName is set to name.  If a DOS environment variable is
// found under fProgName, fDataPath is set to its value.  Otherwise fDataPath
// is set to the fProgPath.

TTool::TTool(const char *name) {
	char	*saveEnv;

	fOptSw = "/-";
	fOptOff = "//";
	fOptList = "";
	fCurInd = 1;
	fHelpNum = 0;
	if (_osmajor >= 3) {
		char	dir[MAXDIR];
		fnsplit(*_argv, fProgPath, dir, fProgName, NULL);
		strcat(fProgPath, dir);
		}
	else {
		getcwd(fProgPath, sizeof(fProgPath));
		if (*(fProgPath+strlen(fProgPath)-1) != '\\')
			strcat(fProgPath, "\\");
		strcpy(fProgName, name);
		}
	if ((saveEnv = strupr(getenv(fProgName))) != NULL) {
		int	saveDrive;
		char	savePath[MAXPATH];
		saveDrive = getdisk();
		if (strlen(saveEnv)>1 && saveEnv[1] == ':')
			setdisk(*saveEnv - 'A');
		getcwd(savePath, sizeof(savePath));
		strcpy(fDataPath, saveEnv);
		if (*(fDataPath+strlen(fDataPath)-1) == '\\')
			strcat(fDataPath, ".");
		chdir(fDataPath);
		getcwd(fDataPath, sizeof(fDataPath)-1);
		chdir(savePath);
		setdisk(saveDrive);
		}
	else if (getcwd(fDataPath, sizeof(fDataPath)) == NULL)
		strcpy(fDataPath, fProgPath);
	if (*(fDataPath+strlen(fDataPath)-1) != '\\')
		strcat(fDataPath, "\\");
	}


// Copyright displays a copyright notice which includes the running program's
// name, version number, and year.

void TTool::Copyright(const char *version, const char *year) {
	cerr << fProgName << "  " << kVersion << " " << version << "  " << kCopyright << " " << year << " " << kCompany << "\n";
	}


// GetOption() takes command lines and sensibly parses options and arguments.
// Command lines parsed must be of the form:
//
//		<command name>  [<options>]  [<data>, ...]
//
// Options are introduced by fOptSw.
//
// GetOption() receives a string of variables that is a list of allowable
// option flags.  Single letters (uppercase and lowercase letters are unique)
// and single-digit numbers are acceptable option letters.  If a valid option
// letter in the list is followed by a colon, GetOption() expects to find a
// following option argument.
//
// In addition to the parameter passed, GetOption() involves two global
// variables.  The fCurInd variable (option index) is an integer that is
// initialized to 1; it keeps track of which option is currently being
// processed.  A character pointer, fCurArg, is set to NULL unless the option
// being processed takes an argument, in which case fCurArg points to what
// should be the required argument.

short TTool::GetOption(void) {
	static short	sp=1;		// index into current option
	register short	c;		// current option character
	register char	*cp;

	if (sp == 1)
		if (fCurInd >= _argc || strchr(fOptSw, _argv[fCurInd][0]) == NULL || _argv[fCurInd][1] == '\0')
			return (kOptDone);
		else if (strcmp(_argv[fCurInd], fOptOff) == NULL) {
			fCurInd++;
			return (kOptDone);
			}
	fCurOpt = c = _argv[fCurInd][sp];
	if (c == '?') {
		Help(kErrParam);
		return (kOptErr);
		}
	if (c == ':' || (cp=strchr(fOptList, c)) == NULL) {
		if (_argv[fCurInd][++sp] == '\0') {
			fCurInd++;
			sp = 1;
			}
		Help(kErrOpt);
		return (kOptErr);
		}
	if (*++cp == ':') {
		if (_argv[fCurInd][sp+1] != '\0')
			fCurArg = &_argv[fCurInd++][sp+1];
		else if (++fCurInd >= _argc) {
			sp = 1;
			Help(kErrArg);
			return (kOptErr);
			}
		else
			fCurArg = _argv[fCurInd++];
		sp = 1;
		}
	else {
		if (_argv[fCurInd][++sp] == '\0') {
			sp = 1;
			fCurInd++;
			}
		fCurArg = NULL;
		}
	return (c);
	}


void TTool::Help(short error_code) {
	switch (error_code) {
		case kErrOpt:
			cerr << "Unrecognized option " << *fOptSw << (char)fCurOpt << "\n";
			break;
		case kErrArg:
			cerr << "Option " << *fOptSw << (char)fCurOpt << " requires an argument\n";
			break;
		}
	while (fHelpNum--)
		cerr << *fHelpText++;
	}


int TTool::Run(void) {
	Boolean	done=False;

	while (!done)
		switch (GetOption()) {
			case kOptDone:
				done = True;
				break;
			case kOptErr:
				return (kExitSyntax);
			default:
				if (!DoOption(fCurOpt, fCurArg))
					return (kExitSyntax);
				break;
			}
	return (DoWork(_argc-fCurInd, (const char **)_argv+fCurInd));
	}
