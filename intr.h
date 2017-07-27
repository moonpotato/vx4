#pragma once

#include "error.h"

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Types declarations
////////////////////////////////////////////////////////////////////////////////

typedef uint16_t intr_id;

////////////////////////////////////////////////////////////////////////////////
// Constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define INTR_NUM_INTRS 512 // Chosen arbitrarily
#define INTR_INVALID INTR_NUM_INTRS // Will never be a valid interrupt number

enum _intr_name {
    INTR_RESET, // Soft reboot the system
    INTR_HALT, // Stop execution, quit the program
    INTR_GENF, // General fault, causes reset if can't be dealt with
};

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
