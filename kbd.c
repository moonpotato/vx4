#include "kbd.h"

#include "port.h"

#include <stdint.h>
#include <stddef.h>

////////////////////////////////////////////////////////////////////////////////
// Internal constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define KBD_BUFFER_SIZE 2048 // Chosen arbitrarily

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a scancode from the keyboard buffer.
 *
 * Returns: The code read, or 0 on error (including empty buffer).
 */
static uint32_t keyboard_read_queue(port_id num);

static kbd_scancode scancode_buffer[KBD_BUFFER_SIZE];
static size_t buffer_start;
static size_t buffer_end;

static port_entry kbd_port = {
	"Window keyboard v1",
	NULL, // Keyboard is an input-only device. Commands may be accepted in the future.
	keyboard_read_queue // Port reads come from the buffer.
};

// We need a place to store the port number we are assigned
static port_id assigned_port;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t install_keyboard_handler()
{
	return port_install(&kbd_port, &assigned_port);
}

void keyboard_queue_press(kbd_scancode code)
{
	scancode_buffer[buffer_end] = code;

	buffer_end = (buffer_end + 1) % KBD_BUFFER_SIZE;
	if (buffer_end == buffer_start) {
        buffer_start = (buffer_start + 1) % KBD_BUFFER_SIZE;
	}
}

error_t remove_keyboard_handler()
{
	return port_remove(assigned_port);
}

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

uint32_t keyboard_read_queue(port_id num)
{
	(void)num;

	if (buffer_start == buffer_end) {
		// Buffer is empty
		return 0;
	}

	uint32_t ret = scancode_buffer[buffer_start];
	buffer_start = (buffer_start + 1) % KBD_BUFFER_SIZE;
	return ret;
}
