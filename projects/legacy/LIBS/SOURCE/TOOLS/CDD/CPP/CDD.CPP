// CDD - Change current drive and directory.

static char *pHelp[] = {
"Outputs the current drive and directory or changes to the specified drive\n",
"and directory.\n\n",
"Syntax:  CDD [/?] [path]\n\n",
"Status:  CDD may return the following status codes:\n",
"          0         No errors.\n",
"          1         Syntax error.\n",
"          2         I/O error.\n\n",
"Options: /?         Displays this help text.\n",
"         /footer x  Specify header to appear before path.\n",
"         /header x  Specify footer to appear after path.\n"
};

// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.2    9409.22  Update help displayed with "/?".
// 1.1    9306.24  Allow user to set header and footer for path.
// 1.0.2  9302.15  Handle directories containing extensions.
// 1.0.1  9302.12  Display error if invalid drive/directory.
// 1.0    9209.24  Original program.

#include <ctype.h>
#include <dir.h>
#include <iostream.h>
#include <string.h>

#include <base\include\ctool.h>

class CMyTool : public CTool {
	enum	{ kExitIO=kExitSyntax+1 };
	enum	{ L_FOOTER=L_BAD+1, L_HEADER };

	static KeyWord pTable[];

	char	*fHeader;
	char	*fFooter;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "f",		-L_FOOTER	},
	{ "footer",	-L_FOOTER	},
	{ "h",		-L_HEADER	},
	{ "header",	-L_HEADER	},
	};


CMyTool	tool("CDD", "1.2", "1992-1994");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fHeader = 0;
	fFooter = 0;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_FOOTER:
			fFooter = new char[strlen(argument)+1];
			if (fFooter)
				strcpy(fFooter, argument);
			return (true);
		case L_HEADER:
			fHeader = new char[strlen(argument)+1];
			if (fHeader)
				strcpy(fHeader, argument);
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	if (argc == 0) {
		static char	path[MAXPATH];

		getcwd(path, sizeof(path));
		if (fHeader)
			cout << fHeader << ' ';
		cout << path;
		if (fFooter) {
			if (path[strlen(path)-1] != '\\')
				cout << '\\';
			cout << fFooter;
			}
		cout << '\n';
		}
	else {
		static char	drive[MAXDRIVE], dir[MAXDIR+MAXFILE], file[MAXFILE], ext[MAXEXT];

		int old_disk = getdisk();
		fnsplit(*argv, drive, dir, file, ext);
		strcat(dir, file);
		strcat(dir, ext);
		setdisk(toupper(*drive)-'A');
		if (chdir(dir) != 0) {
			setdisk(old_disk);
			cout << "Invalid drive or directory.\n";
			return (kExitIO);
			}
		}
	return (kExitOK);
	}
