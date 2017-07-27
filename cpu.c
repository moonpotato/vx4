#include "cpu.h"

#include "error.h"
#include "mem.h"
#include "register.h"
#include "graphics.h"
#include "intr.h"
#include "stack.h"
#include "instruction.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL2/SDL_thread.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define RESET_ON(expr) \
	do { \
		if ((expr) != ERR_NOERR) { \
			flags.reset = true; \
			return true; \
		} \
	} while (0)

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

static mem_addr reg_ip; // Instruction pointer

typedef struct _cpu_flags {
	bool reset : 1;
	bool halt : 1;
	bool intr : 1; // Are interrupts enabled?
	int reserved : 29; // Needed to fill out structure size
} cpu_flags;

static cpu_flags flags;

static SDL_Thread *cpu_thread;
static bool do_stopping;

/**
 * Advance the CPU by executing a single instruction.
 *
 * Returns: If the CPU should continue executing.
 */
static bool cpu_step();

/**
 * Run the CPU indefinitely
 */
static int cpu_loop(void *data);

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t cpu_begin()
{
    cpu_thread = SDL_CreateThread(cpu_loop, "cpu", NULL);

    if (!cpu_thread) {
		return ERR_EXTERN;
    }

    return ERR_NOERR;
}

void cpu_wait_end()
{
    SDL_WaitThread(cpu_thread, NULL);
}

bool cpu_halting()
{
	return do_stopping;
}

void cpu_queue_reset()
{
	flags.reset = true;
}

void cpu_queue_halt()
{
	flags.halt = true;
}

void cpu_queue_jump(mem_addr new_ip)
{
    reg_ip = new_ip;
}

void cpu_interrupt_set(bool enabled)
{
	flags.intr = enabled;
}

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
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

	if (flags.intr) {
		intr_id next_intr = interrupt_which();
		if (next_intr != INTR_INVALID) {
			// Fetch our interrupt vector (IV)
            mem_addr next_ip;
			mem_read_word(next_intr * 4, &next_ip);

			// Neither 0 nor 1 are sensible IVs (they are both inside the IVT)
            // So we use them as a signal to reset (0) or halt (1) instead
            if (next_ip == 0) {
				flags.reset = true;
				return true;
            }
            else if (next_ip == 1) {
				flags.halt = true;
				return true;
            }

			// Push all our registers
			// If this fails, cause a reset
			RESET_ON(stack_enter_frame());
			// If the stack is aligned correctly here, the following won't fail
            stack_push(reg_ip);
            stack_push_multi((uint32_t *)&flags, sizeof (cpu_flags) / 4);
            stack_skip(REG_NUM_REGS);
            reg_write_all_mem(reg_sp);

            // Finally, do the jump
			reg_ip = next_ip;
		}
	}

    instruction_id curr;
    mem_read_dbyte(reg_ip, &curr);
    reg_ip += 2;

    if (!IS_VALID_INSTRUCTION(curr)) {
        interrupt_raise(INTR_INS);
        return true;
    }

    mem_size extra = instructions[curr].extra;
    error_t stat;

    if (extra > 0) {
		uint8_t data[extra];
		mem_read_mem(reg_ip, data, extra);
		reg_ip += extra;

		stat = (instructions[curr].func)(data);
    }
    else {
		stat = (instructions[curr].func)(NULL);
    }

    if (stat != ERR_NOERR) {
        interrupt_raise(INTR_INS);
    }

	return true;
}

int cpu_loop(void *data)
{
	(void)data;

	while (cpu_step());

	do_stopping = true;
	return 0;
}
