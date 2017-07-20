#pragma once

#include <stdnoreturn.h>

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

typedef enum _error_t {
	ERR_NOERR,
	ERR_PCOND,
	ERR_NOMEM,
	ERR_INVAL,
	ERR_AGAIN,
} error_t;

#define DIE_ON(expr) \
	do { \
		error_t code = (expr); \
		if (code != ERR_NOERR) { \
			error_exit(code, __FILE__, __LINE__, NULL); \
		} \
	} while (0)


/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

noreturn void error_exit(error_t err_code, const char *file, unsigned line, const char *info);

