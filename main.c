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
#include <stddef.h>

static size_t n_disks;
static disk_id *loaded_disks;

static void load_disks(int argc, char *argv[]);
static void unload_disks();

int main(int argc, char *argv[])
{
	// Load core firmware images
	// These are all considered critical, so we fail if any one fails
	DIE_ON(firmware_load(0x0, "fw.bin"));

	// Install core I/O ports
	DIE_ON(install_system_handler());
	DIE_ON(install_textio_handler());

	// Each argument passed on the command line becomes a loaded disk.
	load_disks(argc - 1, &argv[1]);

	// Test writing to the disk mmap
	// The main control logic will be called here
	mem_write_word(0xF0000000, 'feeb');

	// Clean up now, in reverse order
	unload_disks();

	remove_textio_handler();
	remove_system_handler();

	return EXIT_SUCCESS;
}

void load_disks(int argc, char *argv[])
{
	n_disks = argc;
	loaded_disks = calloc(n_disks, sizeof (disk_id));

	for (size_t i = 0; i < n_disks; ++i) {
        DIE_ON(disk_install(argv[i], &loaded_disks[i]));
	}
}

void unload_disks()
{
	for (size_t i = 0; i < n_disks; ++i) {
        disk_remove(loaded_disks[i]);
	}

	free(loaded_disks);
	loaded_disks = NULL;
}
