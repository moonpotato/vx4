#include "error.h"

#include "textio.h"
#include "sysp.h"
#include "fwload.h"
#include "disk.h"
#include "graphics.h"
#include "kbd.h"
#include "intr.h"

// Needed for any program that runs with SDL2
#include <SDL2/SDL_main.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static size_t n_disks;
static disk_id *loaded_disks;

static void load_disks(int argc, char *argv[]);
static void unload_disks();

// Stub, until CPU system is written
bool cpu_step()
{
    return (interrupt_which() != INTR_HALT);
}

int main(int argc, char *argv[])
{
	// Load core firmware images
	// These are all considered critical, so we fail if any one fails
	DIE_ON(firmware_load(0x0, "fw.bin"));

	// Install core I/O ports
	DIE_ON(install_system_handler());
	DIE_ON(install_textio_handler());

	// Each argument passed on the command line becomes a loaded disk
	load_disks(argc - 1, &argv[1]);

	// At the moment, use a fixed-size render window
	DIE_ON(graphics_begin(640, 480));

	DIE_ON(install_keyboard_handler());

	bool cont = true;
	while (cont) {
		graphics_step();
		graphics_render();

		cpu_step();
	}

	// Clean up now, in reverse order
	remove_keyboard_handler();

	graphics_end();

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
