#include "textio.h"

#include "port.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Module internal declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Writes a character to the console.
 *
 * IN c: The unsigned char to write, promoted to 32 bits.
 */
static void console_write(port_id num, uint32_t c);

/**
 * Reads a character from the console
 *
 * Returns: The unsigned char read, promoted to 32 bits,
 * or 0 on error.
 */
static uint32_t console_read(port_id num);

static port_entry text_port = {
    "Generic serial I/O",
    console_write, // Port writes go to console
    console_read // Port reads come from console
};

// We need a place to store the port number we are assigned
static port_id assigned_port;

////////////////////////////////////////////////////////////////////////////////
// Interface functions
////////////////////////////////////////////////////////////////////////////////

error_t install_textio_handler()
{
    // We want to see any results immediately
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    return port_install(&text_port, &assigned_port);
}

error_t remove_textio_handler()
{
    return port_remove(assigned_port);
}

////////////////////////////////////////////////////////////////////////////////
// Module internal functions
////////////////////////////////////////////////////////////////////////////////

void console_write(port_id num, uint32_t c)
{
    (void)num;

    putchar((unsigned char)c);
}

uint32_t console_read(port_id num)
{
    (void)num;

    int c = getchar();

    if (c == EOF) {
        return 0;
    }
    else {
        return (uint32_t)c;
    }
}
