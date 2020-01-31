// íkeywords: once.cpp 1.1 mon 03-22-93 09:56:39í
// ílgbí
// 1.0     JACKN     mon 03-22-93 09:55:18
//         Original program; 9302.12
// 1.1     JACKN     mon 03-22-93 09:56:39
//         Fix 'per week' algorithm; 9302.15
// ílgeí
// ínokeywordsí

#include <fstream.h>
#include <string.h>
#include <time.h>

#include <ctool.h>

static char *pHelp[] = {
"Returns the status of 0 if the specified control file has not been accessed\n",
"at least once for the specified term.  Otherwise returns the status of 1.\n\n",
"Syntax:  Once [/?] [/per] [path]\n\n",
"Status:  Once may return the following status codes:\n",
"          0   No errors.\n",
"          1   Syntax error.\n",
"          2   I/O error.\n",
"          3   Already performed once.\n\n",
"Options: /?   Displays this help text.\n",
"         /per day | week | month\n"
};

class CMyTool : public CTool {
	enum	{ kExitIO=kExitSyntax+1, kExitOnce };
	enum	{ L_PER=L_BAD+1 };
	enum	{ kPerDay, kPerWeek, kPerMonth };

	static KeyWord pTable[];

	int	fPer;

	Boolean	DoOption(LToken option, const char *argument);
	int	DoWork(short argc, const char **argv);
public:
	CMyTool(const char *name, const char *version, const char *year);
	};


KeyWord CMyTool::pTable[] = {
	{ "per",	-L_PER	},
	};


CMyTool	tool("Once", "1.0.1", "1993");


main() {
	return (tool.Run());
	}


CMyTool::CMyTool(const char *name, const char *version, const char *year) : CTool(name, version, year) {
	fHelpText = pHelp;
	fHelpNum = NUMELE(pHelp);
	fOptions.Init(pTable, NUMELE(pTable));

	fPer = kPerDay;
	}


Boolean	CMyTool::DoOption(LToken option, const char *argument) {
	enum	{ L_DAY, L_WEEK, L_MONTH, L_UNDEFINED };
	CToken	per;
	KeyWord	per_list[] = {
			{ "day",	L_DAY	},
			{ "month",	L_MONTH	},
			{ "week",	L_WEEK	},
			};

	switch (option) {
		case L_PER:
			per.Init(per_list, NUMELE(per_list));
			switch (per.Check(argument, L_UNDEFINED, true)) {
				case L_DAY:
                                	fPer = kPerDay;
					return (true);
				case L_WEEK:
					fPer = kPerWeek;
					return (true);
				case L_MONTH:
					fPer = kPerMonth;
					return (true);
				}
		}
	return (false);
	}


int CMyTool::DoWork(short argc, const char **argv) {
	if (argc != 1) {
		Help();
		return (kExitSyntax);
		}

	ifstream	fIn(*argv);
	char		old_time_str[26], cur_time_str[26];
	time_t		t;

	t = time(0);
	strcpy(cur_time_str, ctime(&t));
	if (!fIn == 0) {			// if file exists...
		fIn.get(old_time_str, sizeof(old_time_str), 0);
		switch (fPer) {
			case kPerDay:
				if (strncmp(old_time_str, cur_time_str, 3) == 0)
					return (kExitOnce);
				break;
			case kPerWeek:
				if (strncmp(old_time_str, cur_time_str, 3) != 0)
					return (kExitOnce);
				if (strncmp(old_time_str+8, cur_time_str+8, 2) == 0)
					return (kExitOnce);
				break;
			case kPerMonth:
				if (strncmp(old_time_str+4, cur_time_str+4, 3) == 0)
					return (kExitOnce);
				break;
			}
		}
	fIn.close();
	ofstream fOut(*argv, ios::trunc);
	if (!fOut) {
		cout << "Unable to create/modify file " << *argv << ".\n";
		return (kExitIO);
		}
	fOut << cur_time_str;
	return (kExitOK);
	}
