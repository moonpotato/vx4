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

static port_entry *ports[PORT_NUM_PORTS];

static error_t bind_port(port_t num, port_entry *cfg);
static error_t unbind_port(port_t num);

static port_t next_unused();
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
		*data = 0;
	}

	return ERR_NOERR;
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
	if (num < next_alloc) {
		next_alloc = num;
	}
}

