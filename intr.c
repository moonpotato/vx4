#include "intr.h"

#include "error.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifdef __MINGW32__
#include "winshim.h"
#else
#include <strings.h> // ffs(3) is in this header on modern POSIX systems
#endif // __MINGW32__

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define IS_VALID_INTR(intr) ((intr) < INTR_NUM_INTRS)

// How many bits fit in each array element?
#define INTRS_IN_ELEM (8 * sizeof (unsigned))
#define INTR_BUFFER_SIZE (INTR_NUM_INTRS / INTRS_IN_ELEM)

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Each interrupt being raised or not is represented as a single bit.
 */
static unsigned intr_buffer[INTR_BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

/*
 * For anyone unclear on bitwise techniques, x |= 1 << y sets the yth bit of
 * x and x &= ~(1 << y) clears ONLY the yth bit of x (x &= 1 << y would clear
 * all EXCEPT the yth bit of x).
 */

error_t interrupt_raise(intr_id which)
{
	if (!IS_VALID_INTR(which)) {
		return ERR_INVAL;
	}

	intr_buffer[which / INTRS_IN_ELEM] |= 1u << (which % INTRS_IN_ELEM);
	return ERR_NOERR;
}

error_t interrupt_clear(intr_id which)
{
	if (!IS_VALID_INTR(which)) {
		return ERR_INVAL;
	}

	intr_buffer[which / INTRS_IN_ELEM] &= ~(1u << (which % INTRS_IN_ELEM));
	return ERR_NOERR;
}

void interrupt_clear_all()
{
	memset(intr_buffer, 0, INTR_BUFFER_SIZE * sizeof (unsigned));
}

intr_id interrupt_which()
{
    for (size_t i = 0; i < INTR_BUFFER_SIZE; ++i) {
		int pos //...
		#ifdef __MINGW32__
            = ffs_shim(intr_buffer[i]);
        #else
        // ffs(3) (Find First Set) returns a 1-based index and 0 for none
		// We have to correct for that
            = ffs(intr_buffer[i]) - 1;
        #endif // __MINGW32__

        if (pos != -1) {
			// We clear the interrupt first so it doesn't fire infinitely
            intr_buffer[i] &= ~(1u << pos);
            return (i * INTRS_IN_ELEM) + pos;
        }
    }

    return INTR_INVALID;
}
