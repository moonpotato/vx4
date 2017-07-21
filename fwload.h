#pragma once

#include "error.h"
#include "mem.h"

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

/**
 * Loads firmware from a file at an address.
 *
 * IN loc: The address to begin the load.
 * IN file: The path to the firmware file to load.
 *
 * Returns:
 */
extern error_t firmware_load(maddr_t loc, const char *filename);
