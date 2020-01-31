// Fnt2Asm - Binary to assembly font converter.

static char *pHelp[] = {
"Reads in a binary font file and generates an equivallent assembler listing.\n\n",
"Syntax: Fnt2Asm source [destination] [/?] [/column] [/def] [/row]\n\n",
"Status: Fnt2Asm can return the following status codes:\n",
"          0         No errors.\n",
"          1         Syntax error.\n",
"          2         Illegal FNT file format.\n",
"          3         I/O or program error.\n",
"          4         Out of memory.\n\n",
"Options: /?         Displays this help text.\n",
"         /column    Generates character bit image in column order.\n",
"         /def name  File name with sections specifying the [header], [public],\n",
"                    [extern] and [footer] definitions.\n",
"         /meta ch   Specify metachatracter (default=%).\n",
"         /row       Generates character bit image in row order (default).\n"
};

// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.5    9604.17  djs     remove 'h' from output of height,width, and size.
// 1.4    9403.01  jn      Generate font table directly.
// 1.3    9402.22  jn      Conform to BASE library; output for any assembler.
// 1.2    9105.31  jn      Conform to new COMMON library.
// 1.1    9102.26  jn      Ability to generate bit map in column order.
// 1.0             jn      Original program.

#include <dir.h>
#include <fstream.h>
#include <iomanip.h>
#include <string.h>
#include <strstrea.h>

#include <newbase\include\cbase.h>
#include <newbase\include\ctool.h>

const char	pAsmExt[] = ".ASM";
const char	pDefExt[] = ".DEF";
const char	pFntExt[] = ".FNT";
const char	pIncExt[] = ".INC";

class CMyTool : public CTool {
	enum { kExitFormat=kExitSyntax+1, kExitIO, kExitMemory };
	enum { L_COLUMN=L_BAD+1, L_DEF, L_META, L_ROW };

	static KeyWord pTable[];

	Boolean	fRow;
	char	fMeta;
	char	fBuffer[512];
	char	fHeader[512];
	char	fPublic[512];
	char	fExtern[512];
	char	fFooter[512];
	char	fSrc[MAXPATH];
	char	fDst[MAXPATH];
//	CBreak	fCtrlBreak;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
	int	GenFontColumn(istream &in, ostream &out);
	int	GenFontRow(istream &in, ostream &out);
	void	GenSection(ostream &out, const char *section, const char *var);
	Boolean	ReadDescription(const char *name);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "c",		L_COLUMN	},
	{ "column",	L_COLUMN	},
	{ "d",		-L_DEF		},
	{ "def",	-L_DEF		},
	{ "m",		-L_META		},
	{ "meta",	-L_META		},
	{ "r",		L_ROW		},
	{ "row",	L_ROW		},
	};


CMyTool	tool("Fnt2Asm", "1.4", "1990-1994");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fRow = true;
	fMeta = '%';
	strcpy(fHeader, "CHAR_SET\tSEGMENT\tCODE\n\n\tRSEG\tCHAR_SET");
	strcpy(fPublic, "\tPUBLIC\t%");
	strcpy(fExtern, "\tEXTRN\tCODE(%)");
	strcpy(fFooter, "\tEND");
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	switch (option) {
		case L_COLUMN:
			fRow = false;
			return (true);
		case L_DEF:
			return (ReadDescription(argument));
		case L_META:
			fMeta = *argument;
			return (true);
		case L_ROW:
			fRow = true;
			return (true);
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	char	drive[MAXDRIVE], dir[MAXDIR], name[MAXFILE];

	if (argc < 1) {
		Help();
		return (kExitSyntax);
		}
	Merge(fSrc, argv[0], pFntExt);
	if (argc < 2) {
		fnsplit(fSrc, drive, dir, name, 0);
		fnmerge(fDst, drive, dir, name, pAsmExt);
		}
	else
		Merge(fDst, argv[1], pAsmExt);

	ifstream	in(fSrc, ios::in | ios::binary);
	if (!in) {
		cerr << "Unable to open input file " << fSrc << ".\n";
		return (kExitIO);
		}

	ofstream	out(fDst);
	if (!out) {
		cerr << "Unable to open output file " << fDst << ".\n";
		return (kExitIO);
		}

	if (fRow)
		return (GenFontRow(in, out));
	else
		return (GenFontColumn(in, out));
	}


struct CFontHeader {
	short	fHeight;
	short	fWidth;
	short	fSets;
	};

class CFont {
	char		fName[127];
	CFontHeader	fHeader;
	short		fSize;
	long		fLocTab[128];
public:
	short		GetHeight() { return (fHeader.fHeight); }
	long		GetLoc(short set) { return (fLocTab[set]); }
	const char	*GetName() { return (fName); }
	short		GetSets() { return (fHeader.fSets); }
	short		GetSize() { return (fSize); }
	short		GetWidth() { return (fHeader.fWidth); }
	Boolean		Read(istream &in);
	};

Boolean CFont::Read(istream &in) {
	if (!in.getline(fName, sizeof(fName), '\0'))
		return (false);
	if (!in.read((char *)&fHeader, sizeof(fHeader)))
		return (false);
	fSize = (fHeader.fHeight * fHeader.fWidth) / 8;
	if ((fHeader.fHeight * fHeader.fWidth) % 8)		// calculate character size
		fSize++;
	if (!in.read((char *)&fLocTab, sizeof(fLocTab[0]) * fHeader.fSets))
		return (false);
	return (true);
	}


int CMyTool::GenFontRow(istream &in, ostream &out) {
	CFont	font;
	short	cur_set, count, count_hi, cur_size;

	if (!font.Read(in))
		return (kExitIO);

	GenSection(out, fHeader, "");
	out << "\n";
	for (cur_set=0; cur_set<font.GetSets(); cur_set++) {
		ostrstream	os(fBuffer, sizeof(fBuffer));

		os << font.GetName() << '_' << cur_set << "_FONT" << ends;
		GenSection(out, fPublic, fBuffer);
		}
	out.setf(ios::uppercase);
	out << setfill('0');

	for (cur_set=0; cur_set<font.GetSets(); cur_set++) {
		in.seekg(font.GetLoc(cur_set));
		out << '\n' << font.GetName() << '_' << cur_set << "_FONT:\n";
		out << "\tDB\t";
		out << setw(3) << font.GetHeight() << ",";
		out << setw(3) << font.GetWidth() << ",";
		out << setw(3) << font.GetSize() << "\t; height, width, size\n";
		out << hex;
		count = cur_set ? 128 : 0;
		for (count_hi=count+128; count<count_hi; count++) {
			out << "\tDB\t";
			for (cur_size=font.GetSize(); cur_size; cur_size--) {
				out << setw(3) << in.get() << 'H';
				if (cur_size > 1)
					out << ',';
				}
			out << "\t; " << setw(3) << count << 'h';
			if ((count >= 32 && count < 127) || (count > 127))
				out << " ('" << (char)count << "')";
			out << '\n';
			}
		out << dec;
		}
	out << '\n';
	GenSection(out, fFooter, "");
	return (kExitOK);
	}


#define BYTE_SIZE	8

int CMyTool::GenFontColumn(istream &in, ostream &out) {
	CFont	font;
	char	*buff;
	short	cur_set, count, count_hi, cur_size;
	short	curCol, curRow, curBit, i, j, *image, build_image, bit_mask;

	if (!font.Read(in))
		return (kExitIO);

	buff = new char[font.GetSize()];
	image = new short[font.GetSize() * sizeof(short)];
	if (!buff || !image) {
		delete[] buff;
		delete[] image;
		return (kExitMemory);
		}

	GenSection(out, fHeader, "");
	out << "\n";
	for (cur_set=0; cur_set<font.GetSets(); cur_set++) {
		ostrstream	os(fBuffer, sizeof(fBuffer));

		os << font.GetName() << '_' << cur_set << "_FONT" << ends;
		GenSection(out, fPublic, fBuffer);
		}
	out.setf(ios::uppercase);
	out << setfill('0');

	for (cur_set=0; cur_set<font.GetSets(); cur_set++) {
		in.seekg(font.GetLoc(cur_set));
		out << '\n' << font.GetName() << '_' << cur_set << "_FONT:\n";
		out << "\tDB\t";
		out << setw(3) << font.GetHeight() << ",";
		out << setw(3) << font.GetWidth() << ",";
		out << setw(3) << font.GetSize() << "\t; height, width, size\n";
		out << hex;
		count = cur_set ? 128 : 0;
		for (count_hi=count+128; count<count_hi; count++) {
			out << "\tDB\t";
			in.read(buff, font.GetSize());
			build_image = 0x0000;
			bit_mask = 0x0080;
			cur_size = 0;
			for (curCol=0; curCol<font.GetWidth(); curCol++) {
				for (curRow=0, curBit=curCol; curRow<font.GetHeight(); curRow++, curBit+=font.GetWidth()) {
					i = curBit / BYTE_SIZE;
					j = curBit % BYTE_SIZE;
					if (buff[i] & (0x80 >> j))
						build_image |= bit_mask;
					bit_mask >>= 1;
					if (!bit_mask) {
						bit_mask = 0x0080;
						image[cur_size++] = build_image;
						build_image = 0x0000;
						}
					}
				}
			if (cur_size < font.GetSize())
				image[cur_size++] = build_image;
			cur_size = 0;
			out << setw(3) << image[cur_size++] << 'H';
			while (cur_size < font.GetSize())
				out << ',' << setw(3) << image[cur_size++] << 'H';
			out << "\t; " << setw(3) << count << 'h';
			if ((count >= 32 && count < 127) || (count > 127))
				out << " ('" << (char)count << "')";
			out << '\n';
			}
		out << dec;
		}
	out << '\n';
	GenSection(out, fFooter, "");
	delete[] buff;
	delete[] image;
	return (kExitOK);
	}


void CMyTool::GenSection(ostream &out, const char *section, const char *var) {
	for ( ; *section; section++) {
		if (*section == fMeta)
			out << var;
		else
			out << *section;
		}
	}


Boolean	CMyTool::ReadDescription(const char *name) {
	enum { kNone, kHeader, kPublic, kExtern, kFooter };

	ifstream	in;
	int		state=kNone;
	char		*s;

	Merge(fBuffer, name, pDefExt);
	in.open(fBuffer);
	if (!in) {
		cerr << "Unable to open description file " << fBuffer << '\n';
		return (false);
		}
        strcpy(fHeader, "");
	strcpy(fPublic, "");
	strcpy(fExtern, "");
	strcpy(fFooter, "");
	for (;;) {
		if (in.eof())
			break;
		if (!in.getline(fBuffer, sizeof(fBuffer)))
			break;
		if (*fBuffer == '[') {
			if (stricmp(fBuffer, "[header]") == 0)
				state = kHeader;
			else if (strcmp(fBuffer, "[public]") == 0)
				state = kPublic;
			else if (strcmp(fBuffer, "[extern]") == 0)
				state = kExtern;
			else if (strcmp(fBuffer, "[footer]") == 0)
				state = kFooter;
			else {
				cerr << "Invalid descriptor: " << fBuffer << '\n';
				return (false);
				}
			}
		else {
			switch (state) {
				case kHeader:	s=fHeader;	break;
				case kPublic:	s=fPublic;	break;
				case kExtern:	s=fExtern;	break;
				case kFooter:	s=fFooter;	break;
				default:
					cerr << "Line '" << fBuffer << "' ignored.\n";
					break;
				}
			strcat(s, fBuffer);
			strcat(s, "\n");
			}
		}
	return (true);
	}
