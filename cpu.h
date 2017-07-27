#pragma once

#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Advance the CPU by executing a single instruction.
 *
 * Returns: If the CPU should continue executing.
 */
extern bool cpu_step();

/**
 * Set the CPU for immediate (non-interrupt-based) soft reset next step.
 */
extern void cpu_queue_reset();

/**
 * Set the CPU for immediate (non-interrupt-based) halt next step.
 */
extern void cpu_queue_halt();
