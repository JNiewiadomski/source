// Cleans up ASCII text files.

static char *pHelp[] = {
"Cleans up ASCII text files by stripping all NUL's and converting lone\n",
"CR or LF is turned into a CRLF pair.\n\n",
"Syntax:  Clean [/?] [filename...]\n\n",
"Input:   If no source files specified, reads in from stdin.\n\n",
"Output:  If no source files specified, writes out to stdout.\n\n",
"Status:  Clean may return the following status codes:\n",
"          0      No errors.\n",
"          1      Syntax error.\n",
"          2      I/O error.\n\n",
"Options: /?      Displays this help text.\n",
"         /mac    Convert from Macintosh format to IBM PC format.\n",
"         /nul    Converts NUL's into spaces instead of removing them.\n"
};

// Examples
//	The following example cleans the file EXAMPLE.TXT.
//
//	Clean Example.TXT
//
// History:	jn	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.3    9410.14  jn      Convert NUL's to spaces and CR or LF into CRLF.
//                         Remove all other functionality (hinderences).
// 1.2    9405.05  jn      Update for BASE library.
// 1.1    9202.26  jn      Option to remove all control characters.
// 1.0.1  9111.12  jn      Convert illegal characters to \x##.
// 1.0    9103.20  jn      Original program.

#include <dir.h>
#include <fstream.h>
#include <iomanip.h>
#include <iostream.h>
#include <stdio.h>

#include <base\include\cascii.h>
#include <base\include\ctool.h>


const char	pClnExt[] = ".CLN";


class CMyTool : public CTool {
	enum { kExitIO=kExitSyntax+1 };
	enum { L_MAC=L_BAD+1, L_NUL };

	static KeyWord pTable[];

	ios	*fIn, *fOut;
	Boolean	fMac, fNul;

	void	Clean(istream &in, ostream &out);
	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "m",		L_MAC		},
	{ "mac",	L_MAC		},
	{ "n",		L_NUL		},
	{ "nul",	L_NUL		},
	};


CMyTool	tool("CLEAN", "1.3", "1991-1994");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fIn = &cin;
	fOut = &cout;
	fMac = false;
	fNul = false;
	}


Boolean	CMyTool::DoOption(LToken option, const char * /*argument*/) {
	switch (option) {
		case L_MAC:
			fMac = true;
			return (true);
		case L_NUL:
			fNul = true;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	const char	*inFileName;
	char		outFileName[MAXPATH];
	ifstream	in;
	ofstream	out;

	if (argc < 1)
		Clean(cin, cout);
	else {
		while (argc--) {
			inFileName = *argv++;
			cout << inFileName << ':' << endl;
			Merge(outFileName, inFileName, pClnExt);
			in.open(inFileName, ios::in | ios::binary);
			out.open(outFileName);
			if (!in)
				cout << "Unable to open file " << inFileName << ".\n";
			else if (!out)
				cout << "Unable to create file " << outFileName << ".\n";
			else
				Clean(in, out);
			in.close();
			out.close();
			}
		}
	return (kExitOK);
	}


void CMyTool::Clean(istream &in, ostream &out) {
	Byte	ch;
	Boolean	cr=false;
	short	line=1;

	while (!in.eof()) {
		if (!in.get(ch))
			break;
		if (ch == kA_LF) {
			if (cr)
				cr = false;
			else {
				out << endl;
				line++;
				}
			}
		else if (ch == kA_CR) {
			out << endl;
			line++;
			cr = true;
			}
		else if (ch == kA_NUL) {
			if (fNul)
				out << ' ';
			}
		else {
			if (fMac)
				switch (ch) {
					case 210:
					case 211:
						out << '"';
						break;
					case 213:
						out << "'";
						break;
					case 209:
						out << "--";
						break;
					case 201:
						out << "...";
						break;
					default:
						out << ch;
						break;
					}
			else
				out << ch;
			cr = false;
			}
		}
	}
