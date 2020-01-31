#include <stdio.h>

#include <common\common.h>
#include <common\old\coord.h>

#include <seashell\seashell.rh>
#include <seashell\iu.h>
#include <seashell\resclass.h>
#include <seashell\resource.h>

enum { usa, europe, japan };

#define BASE_YEAR	1980

static ResInternat	defaultIU = {
				usa,			// date format
				',',			// thousands separator
				'.',			// decimal separator
				'-',			// date separator
				':',			// time separator
				"<MM-DD-YY>",		// date prompt
				"<HH:MM:SS>",		// time prompt
				"%02d-%02d-%04d",	// date template
				"%02d:%02d:%02d",	// time template
				"%4d:%02d:%02d"		// long time template
				};
static ResInternat	*Intl0=&defaultIU;


// IUGet returns a pointer to the currently loaded international resource.

void *IUGet(void) {
	return (Intl0);
	}


// IUGet reads in the international resource id and returns a pointer to it.
// The string pointers are stuffed with the correct values.

void *IUGet(short id) {
	ResInternat	*r_iu;
	char		*s;

	if ((r_iu = (ResInternat *) ResLoad(R_Internat, id)) == NULL)
		return (&defaultIU);
	s = r_iu->data;
	r_iu->datePrompt = s;
	while (*s++)
		;
	r_iu->timePrompt = s;
	while (*s++)
		;
	r_iu->dateTemplate = s;
	while (*s++)
		;
	r_iu->timeTemplate = s;
	while (*s++)
		;
	r_iu->ltimeTemplate = s;
	return (r_iu);
	}


// IUGetDatePrompt -- returns a pointer to a date prompt in international
// format.

char *IUGetDatePrompt(void) {
	return (Intl0->datePrompt);
	}


// IUGetDecSep -- returns the international decimal separator.

char IUGetDecimal(void) {
	return (Intl0->decimal);
	}


// IUGetTimePrompt -- returns a pointer to a time prompt in international
// format.

char *IUGetTimePrompt(void) {
	return (Intl0->timePrompt);
	}


// IUHrs2LTime -- converts hours to long time.

DWord IUHrs2LTime(float htime) {
	DWord	hour, minute, sec;

	hour = (DWord)htime;
	htime = (htime - (float)hour) * 60;
	minute = (DWord)htime;
	htime = (htime - (float)minute) * 60;
	sec = (DWord)htime;
	return ((hour<<11) + (minute<<5) + (sec/2));
	}


// IUInit -- reads in the local international resource.

void IUInit(void) {
	Intl0 = (ResInternat *) IUGet(_LocalINTERNAT);
	}


// IUInt2LTime -- converts an integer of hours, minutes and seconds to
// a long time notation.

void IUInt2LTime(Word hour, Word min, Word sec, DWord *ltime) {
	*ltime = ((DWord)hour << 11) + ((DWord)min << 5) + ((DWord)sec / 2);
	}


// IUIDate2Str -- converts integer date to a string.

void IUIDate2Str(Word idate, char *sdate) {
	Word	year, month, day;

	day = idate & 0x001F;
	month = (idate >> 5) & 0x000F;
	year = (idate >> 9) + BASE_YEAR;
	switch (Intl0->format) {
		case usa:
			sprintf(sdate, Intl0->dateTemplate, month, day, year);
			break;
		case europe:
			sprintf(sdate, Intl0->dateTemplate, day, month, year);
			break;
		case japan:
			sprintf(sdate, Intl0->dateTemplate, year, month, day);
			break;
		}
	}


// IUITime2Str -- converts an integer time to a string.

void IUITime2Str(Word itime, char *stime) {
	Word	hour, minute, sec;

	sec = (itime & 0x001F) * 2;
	minute = (itime >> 5) & 0x003F;
	hour = itime >> 11;
	sprintf(stime, Intl0->timeTemplate, hour, minute, sec);
	}


// IULTime2Hrs -- converts a long time to hours.

float IULTime2Hrs(DWord ltime) {
	float	hour, minute, sec;

	sec = (float)(ltime&0x001F)*2;
	minute = (float)((ltime>>5)&0x003F);
	hour = (float)(ltime>>11);
	return (hour + ((minute + (sec / 60)) / 60));
	}


// IULTime2Str -- converts a long time to a string.

void IULTime2Str(DWord ltime, char *stime) {
	Word	hour, min, sec;

	sec = (Word)((ltime & 0x001F) * 2);
	min = (Word)((ltime >> 5) & 0x003F);
	hour = (Word)(ltime >> 11);
	sprintf(stime, Intl0->ltimeTemplate, hour, min, sec);
	}


// IULTimeAdd -- adds two long times together.

DWord IULTimeAdd(DWord ltime1, DWord ltime2) {
	DWord	hour, minute, sec;

	sec = ((ltime1&0x001F)*2) + ((ltime2&0x001F)*2);
	minute = ((ltime1>>5)&0x003F) + ((ltime2>>5)&0x003F);
	hour = (ltime1>>11) + (ltime2>>11);
	if (sec > 59) {
		sec -= 60;
		minute++;
		}
	if (minute > 59) {
		minute -= 60;
		hour++;
		}
	return ((hour<<11) + (minute<<5) + (sec/2));
	}


// IUPacked2IDate -- converts a six character representation of the date
// in month, day and year notation to an integer date notation.

Word IUPacked2IDate(char *sdate) {
	Word	year, month, day;

	sscanf(sdate, "%2u%2u%2u", &month, &day, &year);
	if (year < 80)
		return (((year+20)<<9) + (month<<5) + day);
	else
		return (((year-80)<<9) + (month<<5) + day);
	}


// IUPacked2ITime -- converts a six character representation of the time
// in hours, minutes and seconds to an integer time notation.

Word IUPacked2ITime(char *stime) {
	Word	hour, min, sec;

	sscanf(stime, "%2u%2u%2u", &hour, &min, &sec);
	return ((hour<<11) + (min<<5) + (sec/2));
	}


// IUPacked2LTime -- converts an eight character representation of the
// time in hours, minutes and seconds to a long integer time notation.

DWord IUPacked2LTime(char *stime) {
	DWord	hour, min, sec;

	sscanf(stime, "%4lu%2lu%2lu", &hour, &min, &sec);
	return ((hour<<11) + (min<<5) + (sec/2));
	}


// IUStr2ITime -- converts a character string representing the time to
// an integer time notation.

int IUStr2ITime(char *stime, Word *itime) {
	Word	hour, minute, sec;
	int	items;

	items = sscanf(stime, Intl0->timeTemplate, &hour, &minute, &sec);
	if (items != 3 || hour > 23 || minute > 59 || sec > 59)
		return (ERR_INTL_TIME);
	*itime = (hour << 11) + (minute << 5) + (sec / 2);
	return (ERR_INTL_OK);
	}


// IUStr2IDate -- converts a character string representing the
// international date to an integer date notation.

int IUStr2IDate(char *sdate, Word *idate) {
	Word	year, month, day;
	int	items;

	switch (Intl0->format) {
		case usa:
			items = sscanf(sdate, Intl0->dateTemplate, &month, &day, &year);
			break;
		case europe:
			items = sscanf(sdate, Intl0->dateTemplate, &day, &month, &year);
			break;
		case japan:
			items = sscanf(sdate, Intl0->dateTemplate, &year, &month, &day);
			break;
		}
	if (items != 3 || year < BASE_YEAR || year > BASE_YEAR+127)
		return (ERR_INTL_DATE);
	*idate = ((year - BASE_YEAR) << 9) + (month << 5) + day;
	return (ERR_INTL_OK);
	}
