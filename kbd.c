#include "kbd.h"

#include "port.h"
#include "intr.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <SDL2/SDL_mutex.h>

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

/**
 * Sets the do_interrupt value.
 */
static void keyboard_set_interrupt(port_id num, uint32_t data);

// Should every key input cause a hardware interrupt?
static bool do_interrupt;

static kbd_scancode scancode_buffer[KBD_BUFFER_SIZE];
static size_t buffer_start;
static size_t buffer_end;

static port_entry kbd_port = {
	"Window keyboard v2",
	keyboard_set_interrupt,
	keyboard_read_queue // Port reads come from the buffer.
};

// We need a place to store the port number we are assigned
static port_id assigned_port;

static SDL_mutex *kbd_mutex;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t install_keyboard_handler()
{
	kbd_mutex = SDL_CreateMutex();
	if (!kbd_mutex) {
		return ERR_EXTERN;
	}

	return port_install(&kbd_port, &assigned_port);
}

void keyboard_queue_press(kbd_scancode code)
{
	if (SDL_LockMutex(kbd_mutex) != 0) {
		return;
	}

	scancode_buffer[buffer_end] = code;

	buffer_end = (buffer_end + 1) % KBD_BUFFER_SIZE;
	if (buffer_end == buffer_start) {
        buffer_start = (buffer_start + 1) % KBD_BUFFER_SIZE;
	}

	if (do_interrupt) {
        interrupt_raise(INTR_KBD);
	}

	SDL_UnlockMutex(kbd_mutex);
}

error_t remove_keyboard_handler()
{
    SDL_DestroyMutex(kbd_mutex);
    kbd_mutex = NULL;

	return port_remove(assigned_port);
}

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

void keyboard_set_interrupt(port_id num, uint32_t data)
{
    (void)num;

    do_interrupt = (data) ? true : false;
}

uint32_t keyboard_read_queue(port_id num)
{
	(void)num;

	if (SDL_LockMutex(kbd_mutex) != 0) {
		return 0;
	}

	uint32_t ret;

	if (buffer_start == buffer_end) {
		ret = 0;
	}
	else {
		ret = scancode_buffer[buffer_start];
		buffer_start = (buffer_start + 1) % KBD_BUFFER_SIZE;
	}

	SDL_UnlockMutex(kbd_mutex);
	return ret;
}
