#include "sysp.h"

#include "port.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Configures the command that will be executed by command_execute.
 *
 * A command consists of an action word and, optionally, an additional
 * data word. Reading a zero as either the action word or after the
 * command is locked (both the action and data words have been read)
 * causes a reset. If a command is issued without a data word, two zeros
 * must be written to properly reset the port.
 *
 * IN num: The port that caused the function to be called. Ignored.
 * IN command_part: The word to be added to the command.
 */
static void command_issue(port_id num, uint32_t command_part);

/**
 * Executes the command configured by command_issue.
 *
 * A command's execution is a stateful and potentially multi-stage
 * process, so repeated calls are likely intended to produce different
 * results. If the command needs to be reset, command_issue must
 * receive a reset request, then have the command re-issued.
 *
 * IN num: The port that caused the function to be called. Ignored.
 *
 * Returns: The value produced by this step of the command.
 */
static uint32_t command_execute(port_id num);

/**
 * Resets all command procedures. Called in the operation of
 * command_issue.
 */
static void command_clear();

static port_entry system_port = {
	"System command",
	command_issue,
	command_execute
};

// We need a place to store the port number we are assigned
static port_id assigned_port;

typedef struct _sys_operation {
	sys_action act;
	uint32_t data;
} sys_operation;

// Current operation as set by command_issue and read by command_execute
static sys_operation curr_op;

// Used by multiple state machine-type procedures
typedef enum _cmd_state {
	CMD_START,
	CMD_MID,
	CMD_DONE,
} cmd_state;

// Implementations of specific commands

/**
 * Fetches the name of a specific port, in parts.
 *
 * IN port: The port to describe. Only read on the first call after a reset.
 * IN reset: If true, don't fetch data. Instead, reset the state.
 *
 * Returns: 0 if reset == true, otherwise the next byte in the ident
 * string of the port specified on the first non-resetting call.
 */
static uint32_t read_port_ident(port_id port, bool reset);

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

void command_issue(port_id num, uint32_t command_part)
{
	(void)num;

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
			// A null byte doesn't interrupt here
			// We might actually _want_ a zero as our data
			// But if we've issued a one-word command, this means
			// We need to write _two_ zeros to correctly reset
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

uint32_t command_execute(port_id num)
{
	(void)num;

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

uint32_t read_port_ident(port_id port, bool reset)
{
	static cmd_state state = CMD_START;

	static const char *ident = NULL;
	static size_t pos = 0;

	if (reset) {
		ident = NULL;
		pos = 0;

		state = CMD_START;
		return 0;
	}

	switch (state) {
		default:
		case CMD_START:
			ident = port_get_ident(port);
			state = CMD_MID;
			// fallthrough

		case CMD_MID:
			if (ident == NULL) {
				return 0;
			}
			else {
				char out = ident[pos++];

				if (out == 0) {
					state = CMD_DONE;
				}

				return out;
			}

		case CMD_DONE:
			return 0;
	}
}
