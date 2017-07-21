#include "error.h"

#include "mem.h"
#include "port.h"
#include "register.h"
#include "textio.h"

#include <stdio.h>
#include <stdint.h>

void recv_word(uint32_t data)
{
	printf("Device got: %u\n", data);
}

uint32_t send_word()
{
	printf("Device was read\n");
	return 555;
}

int main()
{
	install_textio_handler();

	port_write(0x0, 'h');
	port_write(0x0, 'e');
	port_write(0x0, 'l');
	port_write(0x0, 'l');
	port_write(0x0, '\n');

}

