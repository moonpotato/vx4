#include "cpu.h"

#include "mem.h"
#include "graphics.h"
#include "intr.h"
#include "stack.h"

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static mem_addr reg_ip; // Instruction pointer

typedef struct _cpu_flags {
	bool reset : 1;
	bool halt : 1;
	bool intr : 1; // Are interrupts enabled?
} cpu_flags;

static cpu_flags flags;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

bool cpu_step()
{
	if (flags.halt) {
		return false;
	}

	if (flags.reset) {
		flags.reset = false;
        mem_read_word(0x0, &reg_ip); // The reset vector is in place of the 0th IV
        // Sensible values for sp and bp, remembering they grow down
        reg_sp = reg_bp = GFX_MMAP_START;
        // Because we have a sensible stack, we can start with interrupts
        flags.intr = true;
	}

	return true;
}

void cpu_queue_reset()
{
	flags.reset = true;
}

void cpu_queue_halt()
{
	flags.halt = true;
}
