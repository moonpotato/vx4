#pragma once

#include "error.h"

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

typedef enum _sys_action {
	SYS_CLEAR, // Used to reset the port before issuing a command
	SYS_RESET, // Reset the whole system
	SYS_HALT, // Halt the system, quitting the program
	SYS_PORTINFO, // Make the ident of a port available to be read
} sys_action;

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Registers the system handler on the next available port.
 *
 * Returns: Any errors occuring during a call to port_insert.
 */
extern error_t install_system_handler();

/**
 * Unregisters the system handler from its assigned port.
 *
 * Returns: Any errors occuring during a call to port_remove.
 */
extern error_t remove_system_handler();
