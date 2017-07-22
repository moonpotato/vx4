#pragma once

#include "error.h"
#include "disk.h" // For DISK_MMAP_START constant

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Constants + helper macros
////////////////////////////////////////////////////////////////////////////////

#define GFX_MEM_MAX (8u * 1024 * 1024) // Reserve 8MiB for pixel data

// Put the graphics memory right before the disk buffers
#define GFX_MMAP_START (DISK_MMAP_START - GFX_MEM_MAX)

typedef enum _gfx_action {
	GA_NONE, // No action to perform
	GA_ADDR, // Get the base address of the graphics mmap
	GA_BUFSZ, // Get the size (in bytes) of the graphics mmap
	GA_RES, // Get the current graphics resolution as (width << 16) | height
} gfx_action;

typedef enum _gfx_state {
	GS_OK,
	GS_WAIT,
	GS_ERROR,
} gfx_state;

////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////

extern error_t graphics_begin(int width, int height);

extern void graphics_render();

extern void graphics_end();
