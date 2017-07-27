#include "cpu.h"

#include "error.h"
#include "mem.h"
#include "register.h"
#include "graphics.h"
#include "intr.h"
#include "stack.h"

#include <stdbool.h>
#include <stdint.h>

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

enum _arg_type {
    IA_NONE,
    IA_REG,
    IA_MEM,
    IA_CONS,
};

enum _ins_type {
	INS_NOP,
	INS_HALT,
	INS_JMP,
};

typedef struct _instruction {
    unsigned args : 3;
    unsigned more : 5;
    unsigned arg1 : 2;
    unsigned arg2 : 2;
    unsigned arg3 : 2;
    unsigned arg4 : 2;
    unsigned type : 16;
} instruction;

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

	if (flags.intr) {
		intr_id next_intr = interrupt_which();
		if (next_intr != INTR_INVALID) {
			// Fetch our interrupt vector (IV)
            mem_addr next_ip;
			mem_read_word(next_intr * 4, &next_ip);

			// Neither 0 nor 1 are sensible IVs (they are both inside the IVT)
            // So we use them as a signal to reset (0) or halt (1) instead
            if (reg_ip == 0) {
				flags.reset = true;
				return true;
            }
            else if (reg_ip == 1) {
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

    instruction curr;
    mem_read_word(reg_ip, (uint32_t *)&curr);
    reg_ip += 4;

    uint8_t additional[curr.more];
    mem_read_mem(reg_ip, additional, curr.more);
    reg_ip += curr.more;

    switch (curr.type) {
		default:
            interrupt_raise(INTR_INS);
            break;

		case INS_NOP:
			break;

		case INS_HALT:
			flags.halt = true;
			break;

		case INS_JMP:
			if (curr.more == 4) {
                if (curr.arg1 == IA_CONS) {
                    reg_ip = *(mem_addr *)&additional;
                }
			}
			else {
				interrupt_raise(INTR_INS);
			}

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
