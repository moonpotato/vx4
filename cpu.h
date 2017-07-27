#pragma once

#include "error.h"
#include "mem.h"

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Starts the CPU simulation thread.
 *
 * Returns:
 * ERR_NOERR: The CPU was started.
 * ERR_EXTERN: An error occurred creating the thread.
 *
 */
extern error_t cpu_begin();

/**
 * Waits for the end of the CPU simulation thread.
 */
extern void cpu_wait_end();

/**
 * Returns whether the CPU is preparing to stop.
 */
extern bool cpu_halting();

/**
 * Set the CPU for immediate (non-interrupt-based) soft reset next step.
 */
extern void cpu_queue_reset();

/**
 * Set the CPU for immediate (non-interrupt-based) halt next step.
 */
extern void cpu_queue_halt();

/**
 * Redirects the CPU's execution to a new address for the next cycle.
 */
extern void cpu_queue_jump(mem_addr new_ip);

/**
 * Enables/disables interrupts on the CPU.
 */
extern void cpu_interrupt_set(bool enabled);
