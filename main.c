#include "error.h"

#include "mem.h"
#include "port.h"
#include "register.h"
#include "textio.h"
#include "sysp.h"
#include "fwload.h"
#include "disk.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
	// Load core firmware images
	// These are all considered critical, so we fail if any one fails
	DIE_ON(firmware_load(0x0, "fw.bin"));

	// Install core I/O ports
	// The only way these report failure is if there are no available ports
	// So don't bother checking errors at this stage-
	install_system_handler();
	install_textio_handler();

	// Install the main disk
	disk_id main_disk;
	DIE_ON(disk_install("potato.bin", &main_disk));

	mem_dump();

	return EXIT_SUCCESS;
}

