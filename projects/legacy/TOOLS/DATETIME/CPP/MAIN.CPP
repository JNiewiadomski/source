// DateTime - outputs the current date and time.

// History:	JN	Jack Niewiadomski
//
// ver    date     author  description
// --------------------------------------------------------------------------
// 1.0    9403.03  jn      Original program.

#include <iostream.h>
#include <string.h>
#include <time.h>

main() {
	char	buff[30];

	time_t t = time(0);
	strcpy(buff, ctime(&t));
	buff[strlen(buff)-1] = ' ';
	cout << buff;
	return (0);
	}
