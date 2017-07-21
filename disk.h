#pragma once

#include "error.h"

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////
// Types declarations
/////////////////////////////////////////////////////////////////////////

typedef uint16_t disk_id;

typedef uint32_t disk_addr; // Disks use linear addressing
typedef uint32_t disk_size; // General size type for disk addressing
typedef uint8_t disk_block; // Compatible with mem_block

/////////////////////////////////////////////////////////////////////////
// Constants + helper macros
/////////////////////////////////////////////////////////////////////////

#define DISK_MAX_DISKS 256 // Chosen arbitrarily

typedef enum _disk_action {
	DA_NONE, // No action to perform
	DA_ADDR, // Get/set the address of the disk memory-mapped buffer
	DA_SEEK, // Get/set the offset of the memory map in the file
} disk_action;

typedef enum _disk_state {
	DS_OK,
	DS_WAIT,
	DS_ERROR,
} disk_state;

/////////////////////////////////////////////////////////////////////////
// Function declarations
/////////////////////////////////////////////////////////////////////////

extern error_t disk_install(const char *filename, disk_id *num);

extern error_t disk_remove(disk_id num);
