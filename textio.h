#pragma once

#include "error.h"

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Registers the text I/O handler on the next available port.
 *
 * Returns: Any errors occurring during a call to port_insert.
 */
extern error_t install_textio_handler();

/**
 * Unregisters the text I/O handler from its assigned port.
 *
 * Returns: Any errors occurring during a call to port_remove.
 */
extern error_t remove_textio_handler();
