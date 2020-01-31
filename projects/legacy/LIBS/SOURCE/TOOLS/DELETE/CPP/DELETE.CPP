// íchange: í
// íkeywords: delete.cpp 1.1 thu 03-18-93 09:03:40í
// ílgbí
// 1.0     JACKN     wed 03-17-93 13:41:09
//         Original program; 9006.07
// 1.1     JACKN     thu 03-18-93 09:03:40
//         Report if unable to delete a file or remove a directory; accept
//         multiple patterns; prompt to remove directory if verify requested.
// ílgeí
// ínokeywordsí

// Delete - delete files and removes directories.

// CAUTION:	Do not link in the WILDARGS.OBJ file.  Doing so will prevent
//		this tool from functioning correctly.

static char *pHelp[] = {
"The Delete command deletes files matching pattern.  Use the sweep and remove\n",
"options to remove empty subdirectories.\n\n",
"Syntax: Delete pattern... [/?] [/remove] [/sweep] [/verify]\n\n",
"Status: Alert can return the following status codes:\n",
"          0       No errors.\n",
"          1       Syntax error.\n",
"          2       Aborted by user.\n",
"          3       I/O error.\n\n",
"Options: /?       Displays this help text.\n",
"         /remove  removes empty subdirectories\n",
"         /sweep   deletes in current directory and subdirectories\n",
"         /verify  prompts before deleting files\n"
};

// Examples
//	The following will delete all files found with the extension .BAK.
//
//	Delete *.bak
//
//	The following will delete all files and remove all subdirectories.
//
//	Delete *.* /sweep /remove
//
//	The following will delete all files found with the extensions .OBJ
//	and .SYM, prompting before deleting.
//
//	Delete *.obj *.sym /verify

#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <iomanip.h>
#include <iostream.h>
#include <string.h>

#include <cbreak.h>
#include <ckeyin.h>
#include <ctool.h>

class CMyTool : public CTool {
	enum { kExitAbort=kExitSyntax+1, kExitIO };
	enum { L_REMOVE=L_BAD+1, L_SWEEP, L_VERIFY };

	static KeyWord pTable[];

	Boolean	fRemove, fSweep, fVerify, fAbort;
	short	fNumDeleted, fNumRemoved, fNumErrors;
	CBreak	fCtrlBreak;
	int	fExitStatus;

	Boolean	CheckIfBreakPressed();
	void	Delete(const char *src);
	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
	short	GetYesOrNo();
        void	Remove(const char *src);
	void	SweepDelete(char *src, const char *pattern);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "r",		L_REMOVE	},	// alias for remove
	{ "remove",	L_REMOVE	},
	{ "s",		L_SWEEP		},	// alias for sweep
	{ "sweep",	L_SWEEP		},
	{ "v",		L_VERIFY	},	// alias for verify
	{ "verify",	L_VERIFY	},
	};


CMyTool	tool("Delete", "1.1", "1990-1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fRemove = fSweep = fVerify = fAbort = false;
	fNumDeleted = fNumRemoved = fNumErrors = 0;
	fExitStatus = kExitOK;
	}


Boolean	CMyTool::DoOption(LToken option, const char * /*argument*/) {
	switch (option) {
		case L_REMOVE:
			fRemove = true;
			return (true);
		case L_SWEEP:
			fSweep = true;
			return (true);
		case L_VERIFY:
			fVerify = true;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	static char	srcPath[MAXPATH], patternName[MAXFILE+MAXEXT];
	static char	drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE], ext[MAXEXT];
	int		flags;

	if (argc == 0) {
		Help();
		return (kExitSyntax);
		}
	fCtrlBreak.Init();
	for (int i=0; !fAbort && i<argc; i++) {
		flags = fnsplit(argv[i], drive, dir, name, ext);
		fnmerge(srcPath, drive, dir, 0, 0);
		strupr(srcPath);
		if ((flags & (FILENAME | EXTENSION)) == 0)
			strcpy(patternName, "*.*");
		else
			fnmerge(patternName, 0, 0, name, ext);
		SweepDelete(srcPath, patternName);
		}
	cout << setw(5) << fNumDeleted << " file";
	if (fNumDeleted != 1)
		cout << 's';
	cout << " deleted.\n";
	if (fNumRemoved) {
		cout << setw(5) << fNumRemoved << " director";
		if (fNumRemoved == 1)
			cout << "y";
		else
			cout << "ies";
		cout << " removed.\n";
		}
	if (fNumErrors) {
		cout << setw(5) << fNumErrors << " error";
		if (fNumErrors > 1)
			cout << "s";
		cout << " encountered.\n";
		}
	fCtrlBreak.CleanUp();
	return (fExitStatus);
	}


// ********************
// * Support Routines *
// ********************

Boolean CMyTool::CheckIfBreakPressed() {
	CKeyIn	keyboard;

	while (keyboard.Avail())
		if (keyboard.Read() == kCtrlBrk) {
			fAbort = true;
			fExitStatus = kExitAbort;
			return (true);
			}
	return (false);
	}


// Delete file src.  The user is prompted whether or not to delete the file
// if fVerify if true.

void CMyTool::Delete(const char *src) {
	if (fVerify) {
		cout << "Delete file " << src << " (y/n)? ";
		if (GetYesOrNo() != 'Y')
			return;
		}
	if (unlink(src) == 0) {
		fNumDeleted++;
		cout << src << "\n";
		}
	else {
		switch (errno) {
			case ENOENT:
				cout << "Path or file " << src << " not found.\n";
				break;
			case EACCES:
				cout << "Permission to delete file " << src << " denied.\n";
				break;
			default:
				cout << "Unknown error " << errno << " while deleting file " << src << ".\n";
				break;
			}
		fNumErrors++;
		fExitStatus = kExitIO;
		}
	}


// GetYesOrNo flushes the keyboard buffer and waits until either the keys
// 'y', 'Y', 'n', 'N' or kCtrlBrk are pressed.  The key is echoed to the
// screen and the value is returned.

short CMyTool::GetYesOrNo() {
	CKeyIn	keyboard;
	short	ch='\0';

	keyboard.Flush();
	while (!ch) {
		ch = keyboard.Read();
		switch (ch) {
			case 'y': case 'Y': case 'n': case 'N':
				ch = toupper(ch);
				cout << (char)ch << '\n';
				break;
			case kCtrlBrk: case kCtrlC:
				cout << "^C\n";
				fAbort = true;
				fExitStatus = kExitAbort;
				break;
			default:
				ch = '\0';
				break;
			}
		}
	return (ch);
	}


void CMyTool::Remove(const char *src) {
	if (fVerify) {
		cout << "Remove directory " << src << " (y/n)? ";
		if (GetYesOrNo() != 'Y')
			return;
		}
	if (rmdir(src) == 0) {
		fNumRemoved++;
		cout << src << "\n";
		}
	else {
		switch (errno) {
			case ENOENT:
				cout << "Path or file " << src << " not found.\n";
				break;
			case EACCES:
				cout << "Permission to remove directory " << src << " denied.\n";
				break;
			default:
				cout << "Unknown error " << errno << " while removing directory " << src << ".\n";
				break;
			}
		fNumErrors++;
		fExitStatus = kExitIO;
		}
	}


// SweepDelete deletes all files that match pattern.  If fSweep is true, all
// subdirectories are searched as well.  The pattern is temporarily appended
// to the src to minimize the number of buffer copies made.  Subdirectories
// are taken care of first so that the src can be removed if it is empty.

void CMyTool::SweepDelete(char *src, const char *pattern) {
	char		*tempPath;
	int		success;
	struct ffblk	ffblk;

	tempPath = src + strlen(src);		// find tail end of paths
	strcpy(tempPath, "*.*");
	success = findfirst(src, &ffblk, FA_DIREC);
	*tempPath = '\0';
	if (fSweep && success == 0) {
		while (success == 0) {
			CheckIfBreakPressed();
			if (fAbort)
				return;
			if (ffblk.ff_attrib == FA_DIREC && *(ffblk.ff_name) != '.') {
				strcpy(tempPath, ffblk.ff_name);
				strcat(tempPath, "\\");
				SweepDelete(src, pattern);
				*tempPath = '\0';
				}
			success = findnext(&ffblk);
			}
		}
	strcpy(tempPath, pattern);
	success = findfirst(src, &ffblk, 0);
	*tempPath = '\0';
	while (success == 0) {
		CheckIfBreakPressed();
		if (fAbort)
			return;
		strcpy(tempPath, ffblk.ff_name);
		Delete(src);
		*tempPath = '\0';
		success = findnext(&ffblk);
		}
	if (fRemove && *src && *src != '.') {
		*--tempPath = '\0';
		Remove(src);
		*tempPath = '\\';
		}
	}
