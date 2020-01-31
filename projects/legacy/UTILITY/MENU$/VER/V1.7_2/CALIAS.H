// **********************************************
// * CAlias defines a simple symbol table class *
// **********************************************

class CAlias {
	enum { kMax=10 };

	char	*fKeyTable[kMax], *fDataTable[kMax];
	int	fNum;
public:
	CAlias();
	~CAlias();

	Boolean		Add(char *buff);
	const char	*Get(const char *key) const;
	};
