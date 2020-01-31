// BinLink - Combines binary files into one file at defined offsets.

static char *pHelp[] = {
"Combines binary files into one file at specified offsets.\n\n",
"Syntax: BinLink outfile [offset file ...] [/boundary] [/?]\n\n",
"Status: BinLink can return the following status codes:\n",
"          0           No errors.\n",
"          1           Syntax error.\n",
"          2           I/O error.\n\n",
"Options: /?           Displays this help text.\n",
"         /boundary #  Fill with FF's to nearest # byte boundary.\n"
};

// Examples
//	The following example combines files PROGRAM.BIN at offset 0 and
//	MESSAGE.BIN at offset 65536 into file BINARY.BIN.
//
//	BinLink BINARY 0 PROGRAM 65536 MESSAGE
//
// History:	jn	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.1    9408.23  jn      Option to fill with FF's to nearest 128 bytes.
// 1.0    9307.06  jn      Original program.

#include <fstream.h>
#include <iostream.h>
#include <stdlib.h>

#include <base\include\ctool.h>

class CMyTool : public CTool {
	enum { kExitIO=kExitSyntax+1 };
	enum { L_BOUNDARY=L_BAD+1 };

	static KeyWord pTable[];

	int	fBoundary;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "b",		-L_BOUNDARY	},
	{ "boundary",	-L_BOUNDARY	},
	};


CMyTool	tool("BinLink", "1.1", "1993, 1994");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));
	fBoundary = 0;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_BOUNDARY:
			fBoundary = atoi(argument);
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	if (argc < 1) {
		Help();
		return (kExitSyntax);
		}

	const char	*oname=*argv++, *iname;
	ofstream	out(oname, ios::out | ios::binary);
	long		offset, count;
	ifstream	in;
	int		status=kExitOK;

	if (!out) {
		cerr << "Unable to open file " << oname << " for output.\n";
		return (kExitIO);
		}
	argc--;
	while (argc) {
		if (argc < 2) {
			cerr << "Expecting file name associated with offset " << *argv << ".\n\n";
			Help();
			return (kExitSyntax);
			}
		offset = atol(*argv++);
		iname = *argv++;
		argc -= 2;
		count = offset - out.tellp();
		if (count < 0) {
			cerr << "Invalid offset " << offset << " ignored.\n";
			status = kExitSyntax;
			}
		else
			while (count--)
				out << '\0';
		in.open(iname, ios::in | ios::binary);
		if (!in) {
			cerr << "Unable to open file " << iname << " for input.\n";
			status = kExitIO;
			continue;
			}
		cerr << iname << ":\n";
		// copy contents of input stream to output stream
		out << in.rdbuf();
		in.close();
		}

	if (fBoundary) {
		offset = out.tellp();
		count = (offset + (fBoundary-1)) / fBoundary;
		count = (count * fBoundary) - offset;
		while (count--)
			out << (char)0xFF;
		}

	return (status);
	}
