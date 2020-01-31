#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>

#include "error.h"


// ErrorFatal issues a fatal error message and terminates program
// execution.  The argument list uses the same format as a call to printf.

void ErrorFatal(char *format, ...) {
	va_list	args;

	printf("Fatal Error: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	fcloseall();
	exit(ERR_PROG);
	}


// ErrorFatalList issues a fatal error message and terminates program
// execution.  The argument index subscripts the array of strings.  This
// allows several calling routines to issue a consistent error message.
// If the index is out of range, a generic error message is issued.

void ErrorFatalList(short index, char *msg) {
	static char *list[] = {
		"Out of Memory",
		"Internal Error",
		};

	if (index < NUMELE(list))
		printf("%s: %s\n", list[index], msg);
	else
		printf("Sorry, an internal error occurred (%s)\n", msg);
	fcloseall();
	exit(ERR_PROG);
	}
