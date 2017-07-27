#pragma once

#include "error.h"

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Types declarations
////////////////////////////////////////////////////////////////////////////////

typedef enum _intr_id {
    INTR_RESET, // Soft reboot the system
    INTR_HALT, // Stop execution, quit the program
    INTR_GENF, // General fault, causes reset if can't be dealt with

    INTR_NUM_INTRS = 512, // Arbitrary limit
    INTR_INVALID = 512, // Will never be a valid interrupt number
} intr_id;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Raise or clear a specific interrupt.
 *
 * IN which: The interrupt to raise/clear.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_INVAL: The interrupt specified was not a valid interrupt number.
 */
extern error_t interrupt_raise(intr_id which);
extern error_t interrupt_clear(intr_id which);

/**
 * Clear all set interrupts at once, ignoring them.
 */
extern void interrupt_clear_all();

/**
 * Get the lowest-numbered interrupt that is currently raised, and
 * clear it.
 *
 * Returns: The lowest interrupt number raised, or INTR_INVALID if none are.
 */
extern intr_id interrupt_which();
