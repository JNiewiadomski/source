#include <dir.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common\common.h>
#include <common\ttimer.h>

// ***************************
// * Common Utility Routines *
// ***************************

// AlertSound() turns on the speaker at a certain frequency for a short
// duration.

void AlertSound(void) {
	const long	kDelayTime=3L;
	TTimer		delay;

	sound(400);
	delay.Reset(kDelayTime);
	delay.Wait(NULL);
	sound(300);
	delay.Reset(kDelayTime);
	delay.Wait(NULL);
	nosound();
	}


char *TempName(void) {
	static Word	num=0;
	static char	*path, name[MAXPATH], drive[MAXDRIVE], dir[MAXDIR];

	if ((path = getenv("TMP")) == NULL && (path = getenv("TEMP")) == NULL)
		path = "";
	fnsplit(path, drive, dir, NULL, NULL);
	do {
		sprintf(name, "%s%sTMP%05u.$$$", drive, dir, num++);
		if (num == 0)
			return (NULL);		// unique names exhausted
		} while (FileExists(name));
	if ((path = new char[strlen(name)+1]) == 0)
		return (NULL);			// out of memory
	strcpy(path, name);
	return (path);
	}
