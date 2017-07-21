#pragma once

#include "error.h"

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////
// Types declarations
/////////////////////////////////////////////////////////////////////////

typedef uint32_t port_t;
typedef uint32_t psize_t; // A size type for ranging over the port array

typedef void (*port_out_pf)(uint32_t);
typedef uint32_t (*port_in_pf)();

typedef struct _port_entry {
	const char *ident; // A string identifying the owner of the port
	port_out_pf write; // Called whenever the port is written to
	port_in_pf read; // Called whenever the port is read from
} port_entry;

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define PORT_NUM_PORTS 4096 // Chosen arbitrarily

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Registers a handler (read/write actions) on the next available port.
 *
 * IN cfg: A structure defining the handler for the port.
 * OUT num: The port assigned for the handler.
 *
 * NOTE: The caller retains ownership of the struct passed in as cfg, and
 * it MUST remain allocated until a corresponding call to port_remove.
 *
 * Returns:
 * ERR_NOERR: The handler was successfully added to a port.
 * ERR_PCOND: No available port exists to bind (all are in use).
 */
extern error_t port_insert(port_entry *cfg, port_t *num);

/**
 * Removes a handler set from a port, and marks it for reuse.
 *
 * IN num: The port to clear.
 *
 * Returns:
 * ERR_NOERR: The handler was successfully removed.
 * ERR_INVAL: The port specified was out of range (can never exist).
 * ERR_PCOND: The port specified is currently unbound.
 */
extern error_t port_remove(port_t num);

/**
 * Writes a word to a given port, causing it to be recieved by
 * a listening device.
 *
 * IN num: The port number to write. Must be in range [0, PORT_NUM_PORTS).
 * IN data: The word to be written.
 *
 * Returns:
 * ERR_NOERR: The write completed successfully.
 * ERR_INVAL: The port specified was out of range (can never exist).
 * ERR_PCOND: The port specified is currently unbound.
 */
extern error_t port_write(port_t num, uint32_t data);

/**
 * Reads a word from a given port, as provided by a device.
 *
 * IN num: The port number to read. Must be in range [0, PORT_NUM_PORTS).
 * OUT data: The word that is read.
 *
 * Returns:
 * ERR_NOERR: The read completed successfully.
 * ERR_INVAL: The port specified was out of range (can never exist).
 * ERR_PCOND: The port specified is currently unbound.
 */
extern error_t port_read(port_t num, uint32_t *data);

/**
 * Returns the ident name of a particular port.
 *
 * IN num: The port number to access.
 *
 * Returns: The ident string for the given port, or NULL on error.
 */
extern const char *port_get_ident(port_t num);

