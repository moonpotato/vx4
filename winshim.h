#pragma once

#include <stdint.h>

/**
 * Return the index of the first bit set in v (0 being LSB)
 * or -1 if none found.
 */
extern int ffs_shim(uint32_t v);
