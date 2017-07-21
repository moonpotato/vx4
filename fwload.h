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
 * ERR_NOERR: The file was copied into memory successfully.
 * ERR_FILE: The file doesn't exist or otherwise can't be accessed.
 * ERR_NOMEM: A buffer to hold the file could not be allocated.
 */
extern error_t firmware_load(maddr_t loc, const char *filename);
