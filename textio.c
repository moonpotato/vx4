#include "textio.h"

#include "port.h"

#include <stdio.h>
#include <stdint.h>

/////////////////////////////////////////////////////////////////////////
// Module internal declarations
/////////////////////////////////////////////////////////////////////////

static void console_write(uint32_t c);
static uint32_t console_read();

static port_entry text_port = {
	"Generic serial I/O",
	console_write,
	console_read
};

static port_t assigned_port;

/////////////////////////////////////////////////////////////////////////
// Interface functions
/////////////////////////////////////////////////////////////////////////

error_t install_textio_handler()
{
	return port_insert(&text_port, &assigned_port);
}

error_t remove_textio_handler()
{
	return port_remove(assigned_port);
}

/////////////////////////////////////////////////////////////////////////
// Module internal functions
/////////////////////////////////////////////////////////////////////////

static void console_write(uint32_t c)
{
	putchar((unsigned char)c);
}

static uint32_t console_read()
{
	int c = getchar();

	if (c == EOF) {
		return 0;
	}
	else {
		return (uint32_t)c;
	}
}
