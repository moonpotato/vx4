#include "error.h"

#include "mem.h"
#include "port.h"
#include "register.h"
#include "textio.h"
#include "sysp.h"

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
	install_system_handler();
	install_textio_handler();

	for (int i = 0; i < 32; ++i) {
		port_write(0x0, SYS_CLEAR);

		port_write(0x0, SYS_PORTINFO);
		port_write(0x0, i);

		uint32_t data;

		do {
			port_read(0x0, &data);
			printf("%c", data);
		} while (data);

		putchar('\n');
	}
}

