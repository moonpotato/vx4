#include "error.h"

#include "mem.h"
#include "port.h"
#include "register.h"

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
	port_entry p_ent = {
		"BLAFAEFBIASEFAEBEFD",
		recv_word,
		send_word
	};

	port_t p;
	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);
	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);
	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);
	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);
	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);

	port_remove(0x2);

	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);
	port_insert(&p_ent, &p);
	printf("Was allocated: %u\n", p);

	port_write(0x0, 1234);
	port_write(0x2, 1111);
	port_write(0x4, 4321);
	DIE_ON(port_write(0x7, 1234));
}

