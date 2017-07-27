#pragma once

#include "error.h"

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Type declarations
////////////////////////////////////////////////////////////////////////////////

typedef uint16_t disk_id;

////////////////////////////////////////////////////////////////////////////////
// Constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define DISK_MAX_DISKS 256 // Chosen arbitrarily

// Memory from here to the end of the address space may become unavailable
#define DISK_MMAP_START (MEM_BLK_SIZE * (MEM_NUM_BLKS - DISK_MAX_DISKS))

typedef enum _disk_action {
	DA_NONE, // No action to perform
	DA_NUM, // Get the associated disk number
	DA_SEEK, // Get/set the offset of the memory map in the file
	DA_SYNC, // Cause the disk buffer to be written to the backing file
	DA_ADDR, // Get the base address of the disk buffer
	DA_BUFSZ, // Get the size (in bytes) of the disk buffer
} disk_action;

typedef enum _disk_state {
	DS_OK,
	DS_WAIT,
	DS_ERROR,
} disk_state;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

/**
 * Binds a file to a disk slot, maps a buffer into virtual memory at a set
 * location and copies the first block of the file into that memory.
 *
 * IN filename: The name of the file to load and use as backing.
 * OUT num: The disk number that was used.
 *
 * Returns:
 * ERR_NOERR: All operations completed successfully.
 * ERR_PCOND: All disk slots are already in use.
 * ERR_FILE: The file couldn't be opened.
 * ERR_EXTERN: An error occurred in calculating the size of the file, or
 * the file is too small.
 * ERR_NOMEM: The internal buffer could not be allocated.
 * ERR_PORT: An error occurred acquiring two ports to use for the disk,
 * likely because there are no remaining ports.
 */
extern error_t disk_install(const char *filename, disk_id *num);

/**
 * Unbinds the given disk from its file and disables it. The buffer is
 * synced one last time before deletion.
 *
 * IN num: The disk number to unbind.
 *
 * Returns:
 * ERR_NOERR: All operations completed successfully.
 * ERR_INVAL: The disk provided was out of range (can never exist).
 * ERR_PCOND: The disk is already unbound.
 * ERR_FILE: The disk was successfully unbound, but writing it back to
 * file may have failed.
 */
extern error_t disk_remove(disk_id num);
