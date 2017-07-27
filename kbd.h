#pragma once

#include "error.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Type declarations
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t kbd_scancode;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Registers the keyboard handler on the next available port.
 *
 * Returns: Any errors occurring during a call to port_insert, or
 * ERR_EXTERN: There was an error creating the keyboard mutex.
 */
extern error_t install_keyboard_handler();

/**
 * Adds a scancode to the end of the keyboard buffer.
 *
 * IN code: The scancode to add.
 */
extern void keyboard_queue_press(kbd_scancode code);

/**
 * Unregisters the keyboard handler from its assigned port.
 *
 * Returns: Any errors occurring during a call to port_remove.
 */
extern error_t remove_keyboard_handler();
