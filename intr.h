#pragma once

#include "error.h"

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
// Type declarations
////////////////////////////////////////////////////////////////////////////////

typedef uint16_t intr_id;

////////////////////////////////////////////////////////////////////////////////
// Constants + helper macros
////////////////////////////////////////////////////////////////////////////////

enum _intr_name {
    INTR_RESET, // Soft reboot the system
    INTR_HALT, // Stop execution, quit the program
    INTR_GENF, // General fault, causes reset if can't be dealt with
    INTR_INS, // Execution encountered an invalid instruction
    INTR_KBD, // A key press was received

    INTR_INVALID = 512 // Will never be a valid interrupt number
};

#define INTR_NUM_INTRS 512 // Arbitrary limit

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes state required for using interrupts.
 *
 * Returns:
 * ERR_NOERR: Initialization completed successfully.
 * ERR_EXTERN: Some problem occurred in creating the resources.
 */
extern error_t begin_interrupts();

/**
 * Cleans up resources used by this module.
 */
extern void end_interrupts();

/**
 * Raise or clear a specific interrupt.
 *
 * IN which: The interrupt to raise/clear.
 *
 * Returns:
 * ERR_NOERR: The operation completed successfully.
 * ERR_INVAL: The interrupt specified was not a valid interrupt number.
 * ERR_EXTERN: An error occurred acquiring the mutex.
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
