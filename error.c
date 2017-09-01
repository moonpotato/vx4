#include "error.h"

#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

noreturn void error_exit(error_t err_code, const char *file, unsigned line, const char *info)
{
    #ifdef NDEBUG
    (void)file;
    (void)line;
    #endif // NDEBUG

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
            msg = "An error occurred in an external function or file";
            break;

        case ERR_FILE:
            msg = "An error occurred reading or writing one or more files";
            break;

        case ERR_PORT:
            msg = "An error occurred trying to acquire a port";
            break;

        default:
            msg = "An unspecified error occurred";
            break;
    }
    #endif // NDEBUG

    fprintf(stderr, "Error code %d, terminating...\n", err_code);

    #ifndef NDEBUG
    fprintf(stderr, "Note: In %s, on line %u\n", file, line);
    fprintf(stderr, "Note: %s.\n", msg);
    #endif // NDEBUG

    if (info != NULL) {
        fprintf(stderr, "%s.\n", info);
    }

    graphics_end(); // Called to avoid leaking graphics resources
    exit(err_code);
}

