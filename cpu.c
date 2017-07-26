#include "cpu.h"

#include "intr.h"

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static bool do_reset;
static bool do_halt;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

bool cpu_step()
{
	// The following line is stopgap, will be handled in firmware
    if (interrupt_which() == INTR_HALT) { cpu_queue_halt(); }

	if (do_halt) {
		do_halt = false;

		return false;
	}

	if (do_reset) {
		do_reset = false;

        interrupt_clear_all();
        interrupt_enable();
        interrupt_raise(INTR_RESET);
	}

	return true;
}

void cpu_queue_reset()
{
	do_reset = true;
}

void cpu_queue_halt()
{
	do_halt = true;
}
