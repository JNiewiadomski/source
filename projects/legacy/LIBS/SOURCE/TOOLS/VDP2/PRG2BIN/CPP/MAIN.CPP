// Prg2Bin - Converts VDP2 .PRG files to .BIN files by stripping the 128 byte
// header.

static char *pHelp[] = {
"The Prg2Bin command converts .PRG files to .BIN files by stripping the 128\n",
"byte header.\n\n",
"Syntax: Prg2Bin infile [/?]\n\n",
"Status: Prg2Bin can return the following status codes:\n",
"          0       No errors.\n",
"          1       Syntax error.\n",
"          2       I/O error.\n\n",
"Options: /?       Displays this help text.\n"
};

// Examples
//	The following example converts the test file TEST32K.PRG to a .BIN
//	file.
//
//	Prg2Bin test32k
//
// History:	jn	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9306.25  jn      Original program.

#include <dir.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>

#include <ctool.h>

const char	pBinExt[] = ".BIN";
const char	pPrgExt[] = ".PRG";

class CMyTool : public CTool {
	enum { kExitIO=kExitSyntax+1 };
//	enum { L_SILENT=L_BAD+1 };

//	static KeyWord pTable[];

	char	fPath[MAXPATH];

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
	void	Merge(char *path, const char *name, const char *ext);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


//KeyWord CMyTool::pTable[] = {
//	{ "silent",	L_SILENT	},
//	};


CMyTool	tool("Prg2Bin", "1.0", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
//	fOptions.Init(pTable, NUMELE(pTable));
	}


Boolean	CMyTool::DoOption(LToken /*option*/, const char * /*argument*/) {
//	switch (option) {
//		case L_SILENT:
//			fSilent = true;
//			return (true);
//		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	if (argc != 1) {
		Help();
		return (kExitSyntax);
		}

	ifstream	in;
	ofstream	out;

	// Open up an input stream.
	Merge(fPath, argv[0], pPrgExt);
	in.open(fPath, ios::in | ios::binary);
	if (!in) {
		cout << "Unable to open file " << fPath << " for input.\n";
		return (kExitIO);
		}

	// Open up an output stream.
	Merge(fPath, argv[0], pBinExt);
	out.open(fPath, ios::out | ios::binary);
	if (!out) {
		cout << "Unable to open file " << fPath << " for output.\n";
		return (kExitIO);
		}

	char	ch;

	// read in header and dispose of it
	for (int i=0; i<128; i++)
		in.get(ch);

	// copy rest of input stream to output stream
	out << in.rdbuf();

	return (kExitOK);
	}


void CMyTool::Merge(char *path, const char *name, const char *ext) {
	strcpy(path, name);
	if (!(fnsplit(path, 0, 0, 0, 0) & EXTENSION))
		strcat(path, ext);
	}
