// CCITT - Generates a CCITT CRC for the input stream.

static char *pHelp[] = {
"The CCITT command generates a CCITT CRC for the input stream.  If no file\n",
"name is specified, standard input is used.  Standard input is read in text\n",
"mode and all specified files are read in binary mode.\n\n",
"Syntax: CCITT [filename...] [/?]\n\n",
"Status: CCITT can return the following status codes:\n",
"          0       No errors.\n",
"          1       Syntax error.\n\n",
"Options: /?       Displays this help text.\n",
};

// Examples
//	The following example generates a CRC for the file CHECK.BIN.
//
//	CCITT check.bin
//
// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0.1  9411.11  jn      Correct printout of CRC.
// 1.0    9307.01  jn      Original program.

#include <fstream.h>
#include <iomanip.h>
#include <iostream.h>

#include <base\include\cbreak.h>
#include <base\include\ccrc.h>
#include <base\include\ctool.h>

class CMyTool : public CTool {
//	enum { kExitAbort=kExitSyntax+1 };
//	enum { L_SILENT=L_BAD+1 };

//	static KeyWord pTable[];

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


//KeyWord CMyTool::pTable[] = {
//	{ "silent",	L_SILENT	},
//	};


CMyTool	tool("CCITT", "1.0.1", "1993, 1994");


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
	CBreak	ctrlBreak;
	CCRC	crc;
	Byte	ch;

	//
	// setup common output stream characteristics
	//
	cout.setf(ios::uppercase);
	cout << hex;
	//
	// If no parameters specified, use stdin.
	//
	cout << setfill('0');
	if (argc == 0) {
		while (cin.get(ch))
			crc.Update(ch);
		cout << "0x" << setw(4) << crc.Get() << "\n";
		}
	else {
		while (argc--) {
			const char	*name=*argv++;
			ifstream	in(name, ios::in | ios::binary);

			if (!in) {
				cerr << "Unable to open file '" << name << "'.\n";
				continue;
				}
			crc.Init();
			while (in.get(ch))
				crc.Update(ch);
			cout << name << ": 0x" << setw(4) << crc.Get() << "\n";
			}
		}

	return (kExitOK);
	}
