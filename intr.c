#include "intr.h"

#include "error.h"

#include <stddef.h>
#include <stdbool.h>
#include <strings.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define IS_VALID_INTR(intr) ((intr) < INTR_NUM_INTRS)

#define INTRS_IN_ELEM (8 * sizeof (unsigned))
#define INTR_BUFFER_SIZE (INTR_NUM_INTRS / INTRS_IN_ELEM)

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static bool disabled;

/**
 * Each interrupt being raised or not is represented as a single bit.
 */
static unsigned intr_buffer[INTR_BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

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

void interrupt_disable()
{
	disabled = true;
}

void interrupt_enable()
{
	disabled = false;
}

intr_id interrupt_which()
{
    if (disabled) {
		return INTR_INVALID;
    }

    for (size_t i = 0; i < INTR_BUFFER_SIZE; ++i) {
        int pos = ffs(intr_buffer[i]) - 1;

        if (pos != -1) {
            intr_buffer[i] &= ~(1u << pos);
            return (i * INTRS_IN_ELEM) + pos;
        }
    }

    return INTR_INVALID;
}
