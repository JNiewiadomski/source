#include <stdarg.h>
#include <stdio.h>

#include <common\common.h>
#include <common\tfile.h>


// ***********************
// * TFile Class Methods *
// ***********************

Boolean TFile::Print(const char *format, ...) {
	va_list	arg_ptr;
	char	buff[BUFSIZ];
	Word	count;

	va_start(arg_ptr, format);
	count = vsprintf(buff, format, arg_ptr);
	va_end(arg_ptr);
	return (Write(buff, count));
	}


Boolean TFile::ReadLine(void *buf, Word len, Byte nl) {
	Byte	ch, *bp;

	bp = (Byte *) buf;
	while (len) {
		if (read(fHandle, &ch, sizeof(ch)) != sizeof(ch))
			break;
		*bp++ = ch;
		len--;
		if (ch == nl && len) {
			*bp = '\0';
			return (True);
			}
		}
	return (False);
	}
