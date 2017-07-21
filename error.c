#include "error.h"

#include <stdlib.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

noreturn void error_exit(error_t err_code, const char *file, unsigned line, const char *info)
{
	#ifndef NDEBUG
	const char *msg;

	switch (err_code) {
		case ERR_NOERR:
			msg = "No error occurred";
			break;

		case ERR_PCOND:
			msg = "A function's precondition was violated";
			break;

		case ERR_NOMEM:
			msg = "A memory allocation failed";
			break;

		case ERR_INVAL:
			msg = "A function's argument was invalid";
			break;

		case ERR_AGAIN:
			msg = "A request could not be fulfilled at the required time";
			break;

		case ERR_EXTERN:
			msg = "An error occurred in an external function";
			break;

		default:
			msg = "An unspecified error occurred";
			break;
	}
	#endif

	fprintf(stderr, "Error code %d, terminating...\n", err_code);

	#ifndef NDEBUG
	fprintf(stderr, "Note: In %s, on line %d\n", file, line);
	fprintf(stderr, "Note: %s.\n", msg);
	#endif

	if (info != NULL) {
		fprintf(stderr, "Note: %s.\n", info);
	}

	_Exit(err_code);
}

