// The Error module provides a mechanism to issue warnings and errors to
// the user.  The individual occurences of warnings and errors are tallied,
// and may be displayed as statistics.  All messages are issued to stdout.
// This allows the caller to redirect stdout and capture all warnings and
// errors.

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <common\common.h>

#include "error.h"
#include "lex.h"

static short	severe=0, warnings=0;


// ErrorFatal issues a fatal error message and terminates program
// execution.  The argument list uses the same format as a call to printf.

void ErrorFatal(char *format, ...) {
	va_list	args;

	printf("Fatal Error: ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
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
	exit(ERR_PROG);
	}


// Return true is no severe errors were encountered.

Boolean ErrorNoSevere(void) {
	return ((Boolean)(severe == 0));
	}


// ErrorSevere issues a severe error message and increments the global
// severe.  The argument list uses the same format as a call to printf.

void ErrorSevere(char *format, ...) {
	va_list	args;

	printf("Severe Error %s(%d): ", yyname, yylineno);
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	severe++;
	}


// ErrorStats displays the total number of warnings and severe errors
// registered.  If any severe errors were registered, ErrorStats() returns
// true.

Boolean ErrorStats(void) {
	if (warnings == 0 && severe == 0)
		return (false);
	printf("\n%5d Warnings\n", warnings);
	printf("%5d Severe Errors\n", severe);
	return ((Boolean)(severe > 0));
	}


// ErrorWarning displays a warning message and increments the global
// warnings.  The argument list uses the same format as a call to printf.

void ErrorWarning(char *format, ...) {
	va_list	args;

	printf("Warning %s(%d): ", yyname, yylineno);
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
	warnings++;
	}
