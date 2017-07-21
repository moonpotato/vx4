#include "port.h"

#include "error.h"

#include <stdlib.h>
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define IS_VALID_PORT(port) ((port) < PORT_NUM_PORTS)

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Stores the status of each port. Each entry is a pointer to the
 * structure describing the given port.
 */
static port_entry *ports[PORT_NUM_PORTS];

/**
 * Registers a handler on a specific port.
 *
 * IN num: The port to bind.
 * IN cfg: A structure defining the handler for the port.
 *
 * NOTE: The caller retains ownership of the struct passed in as cfg, and
 * it MUST remain allocated until a corresponding call to unbind_port.
 *
 * Returns:
 * ERR_NOERR: The handler was successfully added to the port.
 * ERR_INVAL: The port specified was out of range (can never exist).
 * ERR_PCOND: The port specified was already in use.
 */
static error_t bind_port(port_t num, port_entry *cfg);

/**
 * Unregisters a handler on a specific port.
 *
 * IN num: The port to clear.
 *
 * Returns:
 * ERR_NOERR: The handler was successfully removed.
 * ERR_INVAL: The port specified was out of range (can never exist).
 * ERR_PCOND: The port specified is currently unbound.
 */
static error_t unbind_port(port_t num);

/**
 * Returns the lowest-numbered port unused (ready to be allocated).
 */
static port_t next_unused();

/**
 * Marks a port as available for reuse.
 */
static void mark_unused(port_t num);

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

error_t port_insert(port_entry *cfg, port_t *num)
{
	*num = next_unused();

	return bind_port(*num, cfg);
}

error_t port_remove(port_t num)
{
	error_t stat = unbind_port(num);

	// If the unbinding failed, the port may not be valid
	// so don't attempt to reuse it.
	if (stat == ERR_NOERR) {
		mark_unused(num);
	}

	return stat;
}

error_t port_write(port_t num, uint32_t data)
{
	if (!IS_VALID_PORT(num)) {
		return ERR_INVAL;
	}

	const port_entry *curr = ports[num];

	if (curr == NULL) {
		return ERR_PCOND;
	}

	// Default write handler just swallows the data
	// So we don't error on NULL here
	if (curr->write != NULL) {
		curr->write(data);
	}

	return ERR_NOERR;
}

error_t port_read(port_t num, uint32_t *data)
{
	if (!IS_VALID_PORT(num)) {
		return ERR_INVAL;
	}

	const port_entry *curr = ports[num];

	if (curr == NULL) {
		return ERR_PCOND;
	}

	if (curr->read != NULL) {
		*data = curr->read();
	}
	else {
		// Default read handler is an endless stream of zeros
		*data = 0;
	}

	return ERR_NOERR;
}

const char *port_get_ident(port_t num)
{
	if (!IS_VALID_PORT(num)) {
		return NULL;
	}

	const port_entry *curr = ports[num];

	if (curr == NULL) {
		return NULL;
	}

	return curr->ident;
}

/////////////////////////////////////////////////////////////////////////
// Module internal functions
/////////////////////////////////////////////////////////////////////////

error_t bind_port(port_t num, port_entry *cfg)
{
	if (!IS_VALID_PORT(num)) {
		return ERR_INVAL;
	}

	if (ports[num] != NULL) {
		return ERR_PCOND;
	}

	ports[num] = cfg;
	return ERR_NOERR;
}

error_t unbind_port(port_t num)
{
	if (!IS_VALID_PORT(num)) {
		return ERR_INVAL;
	}

	if (ports[num] == NULL) {
		return ERR_PCOND;
	}

	ports[num] = NULL;
	return ERR_NOERR;
}

// Used to hold state for the following two functions
static port_t next_alloc;

port_t next_unused()
{
	// First check the next_alloc variable
	// If it's good, we're good
	// Otherwise we have to go hunting
	if (ports[next_alloc] != NULL) {
		for (port_t i = 0; IS_VALID_PORT(i); ++i) {
			if (ports[i] == NULL) {
				next_alloc = i;
				break;
			}
		}
	}

	port_t to_ret = next_alloc;

	if (!IS_VALID_PORT(++next_alloc)) {
		next_alloc = 0;
	}

	return to_ret;
}

void mark_unused(port_t num)
{
	// We want to prefer low-numbered ports
	if (num < next_alloc) {
		next_alloc = num;
	}
}

