// **********************************************
// * CAlias defines a simple symbol table class *
// **********************************************

#include <string.h>

#include <cbase.h>

#include "h\calias.h"

CAlias::CAlias() {
	fNum = 0;
	for (int i=0; i<kMax; i++) {
		fKeyTable[i] = 0;
		fDataTable[i] = 0;
		}
	}


CAlias::~CAlias() {
	for (int i=0; i<kMax; i++) {
		delete[] fKeyTable[i];
		delete[] fDataTable[i];
		}
	}


Boolean CAlias::Add(char *buff) {
	if (fNum < kMax) {
		char *key = strtok(buff, "=");
		char *data = strtok(0, "");
		if (!key || !data)
			return (false);
		fKeyTable[fNum] = new char [strlen(key)+1];
		fDataTable[fNum] = new char [strlen(data)+1];
		if (fKeyTable[fNum] && fDataTable[fNum]) {
			strcpy(fKeyTable[fNum], key);
			strcpy(fDataTable[fNum], data);
			fNum++;
			return (true);
			}
		else {
			delete[] fKeyTable[fNum];
			delete[] fDataTable[fNum];
			fKeyTable[fNum] = 0;
			fDataTable[fNum] = 0;
			}
		}
	return (false);
	}


const char *CAlias::Get(const char *key) const {
	for (int i=0; i<fNum; i++)
		if (strcmpi(key, fKeyTable[i]) == 0)
			return (fDataTable[i]);
	return (0);
	}
