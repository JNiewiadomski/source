#ifndef _TVIDEO_H
#define _TVIDEO_H

#ifndef __CONIO_H
#include <conio.h>
#endif

// ***************
// * Video Class *
// ***************

class TVideo {
	struct text_info	fTI;
	char			*fScreen;
public:
	TVideo(void) { fScreen = 0; }
	~TVideo();

	void	Save(void);
	};

extern TVideo	gVideo;

#endif
