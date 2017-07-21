#include "sysp.h"

#include "port.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

static void command_issue(uint32_t command_part);
static uint32_t command_execute();

static void command_clear();

static port_entry system_port = {
	"System command",
	command_issue,
	command_execute
};

// We need a place to store the port number we are assigned
static port_t assigned_port;

typedef struct _sys_operation {
	sys_action act;
	uint32_t data;
} sys_operation;

static sys_operation curr_op;

static uint32_t read_port_ident(port_t port, bool reset);

typedef enum _cmd_state {
	CMD_START,
	CMD_MID,
	CMD_DONE,
} cmd_state;

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

error_t install_system_handler()
{
	return port_insert(&system_port, &assigned_port);
}

error_t remove_system_handler()
{
	return port_remove(assigned_port);
}

/////////////////////////////////////////////////////////////////////////
// Module internal functions
/////////////////////////////////////////////////////////////////////////

void command_issue(uint32_t command_part)
{
	static cmd_state state = CMD_START;

	switch (state) {
		case CMD_START:
			if (command_part == SYS_CLEAR) {
				command_clear();
				state = CMD_START;
			}
			else {
				curr_op.act = (int)command_part;
				state = CMD_MID;
			}
			break;

		case CMD_MID:
			curr_op.data = command_part;
			state = CMD_DONE;
			break;

		case CMD_DONE:
			if (command_part == SYS_CLEAR) {
				command_clear();
				state = CMD_START;
			}
			break;
	}
}

uint32_t command_execute()
{
	switch (curr_op.act) {
		case SYS_PORTINFO:
			return read_port_ident(curr_op.data, false);

		case SYS_CLEAR:
		default:
			return 0;
	}
}

void command_clear()
{
	curr_op.act = SYS_CLEAR;
	curr_op.data = 0;

	read_port_ident(0, true);
}

uint32_t read_port_ident(port_t port, bool reset)
{
	static const char *ident = NULL;
	static size_t pos = 0;
	static bool done = false;

	if (reset) {
		ident = NULL;
		pos = 0;
		done = false;
		return 0;
	}

	if (done) {
		return 0;
	}

	if (pos == 0) {
		ident = port_get_ident(port);
	}

	if (ident == NULL) {
		done = true;
		return 0;
	}

	char out = ident[pos++];

	if (out == '\0') {
		done = true;
	}

	return out;
}